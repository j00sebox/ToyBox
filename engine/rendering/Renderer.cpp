#define VMA_IMPLEMENTATION
#include "pch.h"
#include "Renderer.hpp"
#include "Vertex.hpp"
#include "Log.hpp"
#include "util/DeviceHelper.hpp"
#include "util/FileOperations.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>

#define check(result) { if(result != vk::Result::eSuccess) { fatal("Error in: %s", __FILE__); } }

struct RecordDrawTask : enki::ITaskSet
{
    void init(CommandBuffer* _command_buffer, Renderer* _renderer, const Scene* _scene, const Pipeline* _pipeline, u32 _start, u32 _end)
    {
        command_buffer = _command_buffer;
        renderer = _renderer;
        scene = _scene;
        pipeline = _pipeline;
        start = _start;
        end = _end;
    }

    void ExecuteRange(enki::TaskSetPartition range, uint32_t threadnum) override
    {
        for(u32 i = start; i < end; ++i)
        {
            command_buffer->push_constants(pipeline, vk::ShaderStageFlagBits::eVertex, 0, sizeof(glm::mat4), &scene->m_render_list[i].transform);
            command_buffer->push_constants(pipeline, vk::ShaderStageFlagBits::eFragment, 64, sizeof(glm::uvec4), scene->m_render_list[i].material.textures);

            command_buffer->bind_vertex_buffer(renderer->get_buffer(scene->m_render_list[i].mesh.vertex_buffer));
            command_buffer->bind_index_buffer(renderer->get_buffer(scene->m_render_list[i].mesh.index_buffer));
            command_buffer->draw_indexed(scene->m_render_list[i].mesh.index_count);
        }
        command_buffer->end();
    }

    CommandBuffer* command_buffer;

private:
    Renderer* renderer;
    const Scene* scene;
    const Pipeline* pipeline;
    u32 start;
    u32 end;
};

struct CameraData
{
    glm::mat4 view;
    glm::mat4 proj;
    glm::vec3 camera_position;
};

Renderer::Renderer(GLFWwindow* window, enki::TaskScheduler* scheduler) :
    m_window(window),
    m_scheduler(scheduler),
    m_buffer_pool(&m_pool_allocator, 100, sizeof(Buffer)),
    m_texture_pool(&m_pool_allocator, 20, sizeof(Texture)),
    m_sampler_pool(&m_pool_allocator, 10, sizeof(Sampler)),
    m_descriptor_set_layout_pool(&m_pool_allocator, 10, sizeof(DescriptorSetLayout)),
    m_descriptor_set_pool(&m_pool_allocator, 10, sizeof(DescriptorSet)),
    m_pipeline_pool(&m_pool_allocator, 10, sizeof(Pipeline))
{
    init_instance();
    init_surface();
    init_device();
    init_swapchain();
    init_renderpasses();
    init_descriptor_pools();
    init_descriptor_sets();
    init_graphics_pipeline();
    init_command_pools();
    init_command_buffers();
    init_viewport();
    init_depth_resources();
    init_framebuffers();
    init_sync_objects();

    m_null_texture = create_texture({
        .format = vk::Format::eR8G8B8A8Srgb,
        .image_src = "../assets/textures/null_texture.png"
    });

    m_default_sampler = create_sampler({
        .min_filter = vk::Filter::eLinear,
        .mag_filter = vk::Filter::eLinear,
        .u_mode = vk::SamplerAddressMode::eRepeat,
        .v_mode = vk::SamplerAddressMode::eRepeat,
        .w_mode = vk::SamplerAddressMode::eRepeat
    });

    init_imgui();
}

Renderer::~Renderer()
{
    ImGui_ImplVulkan_Shutdown();
    m_logical_device.destroyDescriptorPool(m_imgui_pool, nullptr);
    for(i32 i = 0; i < k_max_frames_in_flight; ++i)
    {
        // sync objects
        m_logical_device.destroySemaphore(m_image_available_semaphores[i], nullptr);
        m_logical_device.destroySemaphore(m_render_finished_semaphores[i], nullptr);
        m_logical_device.destroyFence(m_in_flight_fences[i], nullptr);

        // uniform buffers
        destroy_buffer(m_camera_buffers[i]);
    }
    destroy_texture(m_null_texture);
    destroy_sampler(m_default_sampler);
    m_logical_device.destroyDescriptorPool(m_descriptor_pool, nullptr);
    m_logical_device.destroyDescriptorPool(m_normal_descriptor_pool, nullptr);
    destroy_descriptor_set_layout(m_camera_data_layout);
    destroy_descriptor_set_layout(m_texture_set_layout);
    m_logical_device.destroyCommandPool(m_main_command_pool);
    m_logical_device.destroyCommandPool(m_viewport_command_pool);
    m_logical_device.destroyCommandPool(m_extra_command_pool);
    for(auto& command_pool : m_command_pools)
    {
        m_logical_device.destroyCommandPool(command_pool, nullptr);
    }
    vmaDestroyAllocator(m_allocator);
    cleanup_swapchain();
    m_logical_device.destroyRenderPass(m_renderpass, nullptr);
    m_logical_device.destroyRenderPass(m_viewport_renderpass, nullptr);
    m_logical_device.destroyRenderPass(m_imgui_renderpass, nullptr);
    destroy_pipeline(m_graphics_pipeline);
    destroy_pipeline(m_viewport_pipeline);
    m_logical_device.destroy();
    m_instance.destroySurfaceKHR(m_surface, nullptr);
#ifdef DEBUG
    m_instance.destroyDebugUtilsMessengerEXT(m_debug_messenger, nullptr, m_dispatch_loader);
#endif
    m_instance.destroy();
}

void Renderer::render(Scene* scene)
{
    if(m_swapchain_recreated)
    {
        scene->camera->resize(m_swapchain_extent.width, m_swapchain_extent.height);
        m_swapchain_recreated = false;
    }
    CameraData camera_data{};
    camera_data.view = scene->camera->camera_look_at();
    camera_data.proj = scene->camera->get_perspective();
    camera_data.camera_position = scene->camera->get_pos();

    // GLM was originally designed for OpenGL where the Y coordinate of the clip space is inverted,
    // so we can remedy this by flipping the sign of the Y scaling factor in the projection matrix
    camera_data.proj[1][1] *= -1;

    auto* camera_buffer = static_cast<Buffer*>(m_buffer_pool.access(m_camera_buffers[m_current_frame]));
    memcpy(camera_buffer->mapped_data, &camera_data, pad_uniform_buffer(sizeof(camera_data)));

    begin_frame();

    auto* material_set = static_cast<DescriptorSet*>(m_descriptor_set_pool.access(m_texture_set));
    auto* camera_set = static_cast<DescriptorSet*>(m_descriptor_set_pool.access(m_camera_sets[m_current_frame]));

    RecordDrawTask record_draw_tasks[m_scheduler->GetNumTaskThreads()];
    u32 models_per_thread, num_recordings, surplus;

    if (m_scheduler->GetNumTaskThreads() > scene->m_render_list.size())
    {
        models_per_thread = 1;
        num_recordings = scene->m_render_list.size();
        surplus = 0;
    }
    else
    {
        models_per_thread = scene->m_render_list.size() / m_scheduler->GetNumTaskThreads();
        num_recordings = m_scheduler->GetNumTaskThreads();
        surplus = scene->m_render_list.size() % m_scheduler->GetNumTaskThreads();
    }

    vk::CommandBufferInheritanceInfo inheritance_info{};
    inheritance_info.renderPass = m_viewport_renderpass;
    inheritance_info.framebuffer = m_viewport_framebuffers[m_image_index];
    inheritance_info.subpass = 0;

    if(scene->skybox)
    {
        auto* skybox_set = static_cast<DescriptorSet*>(m_descriptor_set_pool.access(scene->skybox->descriptor_set));
        auto* skybox_pipeline = static_cast<Pipeline*>(m_pipeline_pool.access(scene->skybox->pipeline));

        m_skybox_commands[m_current_frame].begin(inheritance_info);
        m_skybox_commands[m_current_frame].bind_pipeline(skybox_pipeline);
        m_skybox_commands[m_current_frame].set_viewport(m_swapchain_extent.width, m_swapchain_extent.height);
        m_skybox_commands[m_current_frame].set_scissor(m_swapchain_extent);
        m_skybox_commands[m_current_frame].bind_descriptor_set(skybox_pipeline, 0, 1, camera_set);
        m_skybox_commands[m_current_frame].bind_descriptor_set(skybox_pipeline, 1, 1, skybox_set);
        m_skybox_commands[m_current_frame].bind_vertex_buffer(get_buffer(scene->skybox->vertex_buffer));
        m_skybox_commands[m_current_frame].bind_index_buffer(get_buffer(scene->skybox->index_buffer));
        m_skybox_commands[m_current_frame].draw_indexed(scene->skybox->index_count);
        m_skybox_commands[m_current_frame].end();
    }

    auto* viewport_pipeline = static_cast<Pipeline*>(m_pipeline_pool.access(m_viewport_pipeline));

    u32 start = 0;
    for(u32 i = 0; i < num_recordings; ++i)
    {
        m_command_buffers[m_current_cb_index].begin(inheritance_info);
        m_command_buffers[m_current_cb_index].bind_pipeline(viewport_pipeline);

        // since we specified that the viewport and scissor were dynamic we need to do them now
        m_command_buffers[m_current_cb_index].set_viewport(m_swapchain_extent.width, m_swapchain_extent.height);
        m_command_buffers[m_current_cb_index].set_scissor(m_swapchain_extent);

        m_command_buffers[m_current_cb_index].bind_descriptor_set(viewport_pipeline, 0, 1, camera_set);
        m_command_buffers[m_current_cb_index].bind_descriptor_set(viewport_pipeline, 1, 1, material_set);

        record_draw_tasks[i].init(&m_command_buffers[m_current_cb_index], this, scene, viewport_pipeline, start, start + models_per_thread);
        m_scheduler->AddTaskSetToPipe(&record_draw_tasks[i]);

        start += models_per_thread;
        m_current_cb_index += k_max_frames_in_flight;
    }

    RecordDrawTask extra_draws;
    if(surplus > 0)
    {
        m_extra_draw_commands[m_current_frame].begin(inheritance_info);
        m_extra_draw_commands[m_current_frame].bind_pipeline(viewport_pipeline);
        m_extra_draw_commands[m_current_frame].set_viewport(m_swapchain_extent.width, m_swapchain_extent.height);
        m_extra_draw_commands[m_current_frame].set_scissor(m_swapchain_extent);
        m_extra_draw_commands[m_current_frame].bind_descriptor_set(viewport_pipeline, 0, 1, camera_set);
        m_extra_draw_commands[m_current_frame].bind_descriptor_set(viewport_pipeline, 1, 1, material_set);

        extra_draws.init(&m_extra_draw_commands[m_current_frame], this, scene, viewport_pipeline, start, start + surplus);
        m_scheduler->AddTaskSetToPipe(&extra_draws);
    }

    m_viewport_command_buffers[m_current_frame].execute_command(&m_skybox_commands[m_current_frame]);
    for(u32 i = 0; i < num_recordings; ++i)
    {
        m_scheduler->WaitforTask(&record_draw_tasks[i]);

        if(record_draw_tasks[i].command_buffer)
        {
            m_viewport_command_buffers[m_current_frame].execute_command(record_draw_tasks[i].command_buffer);
        }
    }
    if(surplus > 0)
    {
        m_scheduler->WaitforTask(&extra_draws);
        m_viewport_command_buffers[m_current_frame].execute_command(extra_draws.command_buffer);
    }
    m_viewport_command_buffers[m_current_frame].end_renderpass();
    m_viewport_command_buffers[m_current_frame].end();

    m_imgui_commands[m_current_frame].begin();
    m_imgui_commands[m_current_frame].begin_renderpass(m_imgui_renderpass, m_imgui_framebuffers[m_image_index], m_swapchain_extent, vk::SubpassContents::eInline);
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), m_imgui_commands[m_current_frame].get());
    m_imgui_commands[m_current_frame].end_renderpass();
    m_imgui_commands[m_current_frame].end();

    end_frame();

    m_current_frame = (m_current_frame + 1) % k_max_frames_in_flight;
    m_current_cb_index = m_current_frame;
}

void Renderer::begin_frame()
{
    check(m_logical_device.waitForFences(1, &m_in_flight_fences[m_current_frame], true, UINT64_MAX));

    vk::Result result = m_logical_device.acquireNextImageKHR(m_swapchain, UINT64_MAX, m_image_available_semaphores[m_current_frame], nullptr, &m_image_index);

    // if swapchain is not good we immediately recreate and try again in the next frame
    if(result == vk::Result::eErrorOutOfDateKHR)
    {
        recreate_swapchain();
        return;
    }

    // need to reset fences to unsignaled, but only reset if we are submitting work
    check(m_logical_device.resetFences(1, &m_in_flight_fences[m_current_frame]));

    m_main_command_buffers[m_current_frame].begin();
    m_main_command_buffers[m_current_frame].begin_renderpass(m_renderpass, m_swapchain_framebuffers[m_image_index], m_swapchain_extent, vk::SubpassContents::eInline);
    m_main_command_buffers[m_current_frame].set_viewport(m_swapchain_extent.width, m_swapchain_extent.height);
    m_main_command_buffers[m_current_frame].set_scissor(m_swapchain_extent);
    auto* graphics_pipeline = static_cast<Pipeline*>(m_pipeline_pool.access(m_graphics_pipeline));
    m_main_command_buffers[m_current_frame].bind_pipeline(graphics_pipeline);
    m_main_command_buffers[m_current_frame].end_renderpass();
    m_main_command_buffers[m_current_frame].end();

    m_viewport_command_buffers[m_current_frame].begin();

    // all secondary buffers will be using this renderpass
    m_viewport_command_buffers[m_current_frame].begin_renderpass(m_viewport_renderpass, m_viewport_framebuffers[m_image_index], m_swapchain_extent, vk::SubpassContents::eSecondaryCommandBuffers);
}

void Renderer::end_frame()
{
    vk::SubmitInfo submit_info{};
    submit_info.sType = vk::StructureType::eSubmitInfo;

    // we are specifying what semaphores we want to use and what stage we want to wait on
    vk::Semaphore wait_semaphores[] = {m_image_available_semaphores[m_current_frame]};
    vk::PipelineStageFlags wait_stages[] = {vk::PipelineStageFlagBits::eColorAttachmentOutput};
    vk::CommandBuffer command_buffers_to_submit[] = {m_main_command_buffers[m_current_frame].get(), m_viewport_command_buffers[m_current_frame].get(), m_imgui_commands[m_current_frame].get()};
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = wait_semaphores;
    submit_info.pWaitDstStageMask = wait_stages;
    submit_info.commandBufferCount = 3;
    submit_info.pCommandBuffers = command_buffers_to_submit;

    vk::Semaphore signal_semaphores[] = {m_render_finished_semaphores[m_current_frame]};
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = signal_semaphores;

    check(m_graphics_queue.submit(1, &submit_info, m_in_flight_fences[m_current_frame]));

    // last step is to submit the result back to the swapchain
    vk::SwapchainKHR swapchains[] = {m_swapchain};
    vk::PresentInfoKHR present_info{};
    present_info.sType = vk::StructureType::ePresentInfoKHR;
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = signal_semaphores;
    present_info.swapchainCount = 1;
    present_info.pSwapchains = swapchains;
    present_info.pImageIndices = &m_image_index;

    vk::Result result = m_present_queue.presentKHR(&present_info);

    if(result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR)
    {
        recreate_swapchain();
    }
}

BufferHandle Renderer::create_buffer(const BufferConfig& buffer_config)
{
    BufferHandle handle = (BufferHandle)m_buffer_pool.acquire();
    auto* buffer = static_cast<Buffer*>(m_buffer_pool.access(handle));

    buffer->size = buffer_config.size;

    vk::BufferCreateInfo buffer_info{};
    buffer_info.sType = vk::StructureType::eBufferCreateInfo;
    buffer_info.size = buffer_config.size;
    buffer_info.usage = buffer_config.usage;
    buffer_info.sharingMode = vk::SharingMode::eExclusive;

    VmaAllocationCreateInfo memory_info{};
    memory_info.flags = VMA_ALLOCATION_CREATE_STRATEGY_BEST_FIT_BIT;
    memory_info.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

    if(buffer_config.persistent)
    {
        memory_info.flags |= VMA_ALLOCATION_CREATE_MAPPED_BIT;
    }

    VmaAllocationInfo allocation_info{};
    vmaCreateBuffer(m_allocator, reinterpret_cast<const VkBufferCreateInfo*>(&buffer_info), &memory_info,
                    &buffer->vk_buffer, &buffer->vma_allocation, &allocation_info);

    if(buffer_config.data)
    {
        void* data;
        vmaMapMemory(m_allocator, buffer->vma_allocation, &data);
        memcpy(data, buffer_config.data, (size_t)buffer_config.size);
        vmaUnmapMemory(m_allocator, buffer->vma_allocation);
    }

    if(buffer_config.persistent)
    {
        buffer->mapped_data = static_cast<u8*>(allocation_info.pMappedData);
    }

    return handle;
}

TextureHandle Renderer::create_texture(const TextureConfig& texture_config)
{
    if(m_texture_map.contains(texture_config.image_src))
    {
        return m_texture_map[texture_config.image_src];
    }

    TextureHandle handle = (TextureHandle)m_texture_pool.acquire();
    auto* texture = static_cast<Texture*>(m_texture_pool.access(handle));

    stbi_set_flip_vertically_on_load(1);

    int width, height, channels;
    stbi_uc* pixels = stbi_load(texture_config.image_src, &width, &height, &channels, STBI_rgb_alpha);

    vk::DeviceSize image_size = width * height * 4;

    texture->width = width;
    texture->height = height;
    texture->name = texture_config.image_src;

    if(!pixels)
    {
        throw std::runtime_error("failed to load texture image!");
    }

    BufferHandle staging_handle = create_buffer({
        .usage = vk::BufferUsageFlagBits::eTransferSrc,
        .size = (u32)image_size,
        .data = pixels
    });
    auto* staging_buffer = static_cast<Buffer*>(m_buffer_pool.access(staging_handle));

    stbi_image_free(pixels);

    vk::ImageCreateInfo image_create_info{};
    image_create_info.sType = vk::StructureType::eImageCreateInfo;
    image_create_info.imageType = vk::ImageType::e2D;
    image_create_info.extent.width = static_cast<u32>(width);
    image_create_info.extent.height = static_cast<u32>(height);
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.format = texture_config.format;
    image_create_info.tiling = vk::ImageTiling::eOptimal;
    image_create_info.initialLayout = vk::ImageLayout::eUndefined;
    image_create_info.usage = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled;
    image_create_info.sharingMode = vk::SharingMode::eExclusive;
    image_create_info.samples = vk::SampleCountFlagBits::e1;
    image_create_info.flags = texture_config.flags;

    VmaAllocationCreateInfo memory_info{};
    memory_info.usage = VMA_MEMORY_USAGE_GPU_ONLY;

    vmaCreateImage(m_allocator, reinterpret_cast<const VkImageCreateInfo*>(&image_create_info), &memory_info, &texture->vk_image, &texture->vma_allocation, nullptr);

    transition_image_layout(texture->vk_image, texture_config.format, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);

    copy_buffer_to_image(staging_buffer->vk_buffer, texture->vk_image, static_cast<u32>(width), static_cast<u32>(height));

    transition_image_layout(texture->vk_image, texture_config.format, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);

    texture->vk_image_view = create_image_view(texture->vk_image, vk::ImageViewType::e2D, texture_config.format, vk::ImageAspectFlagBits::eColor);

    destroy_buffer(staging_handle);

    m_texture_map[texture_config.image_src] = handle;

    return handle;
}

TextureHandle Renderer::create_cubemap(std::string images[6])
{
    TextureHandle handle = (TextureHandle)m_texture_pool.acquire();
    auto* cubemap = static_cast<Texture*>(m_texture_pool.access(handle));

    stbi_set_flip_vertically_on_load(true);
    i32 width, height, channels;
    stbi_uc* first_image = stbi_load(images[0].c_str(), &width, &height, &channels, STBI_rgb_alpha);

    vk::DeviceSize image_size = width * height * 4;
    BufferHandle staging_handle = create_buffer({
            .usage = vk::BufferUsageFlagBits::eTransferSrc,
            .size = (u32)image_size * 6,
    });
    auto* staging_buffer = static_cast<Buffer*>(m_buffer_pool.access(staging_handle));

    if(!first_image)
    {
        fatal("Could not load texture image!");
    }

    void* data;
    vmaMapMemory(m_allocator, staging_buffer->vma_allocation, &data);
    memcpy(data, first_image, image_size);
    stbi_image_free(first_image);

    for(u32 i = 1; i < 6; ++i)
    {
        stbi_set_flip_vertically_on_load(false);
        stbi_uc* pixels = stbi_load(images[i].c_str(), &width, &height, &channels, STBI_rgb_alpha);
        if(!pixels[i])
        {
            fatal("failed to load texture image!");
        }
        memcpy((u8*)data + (image_size * i), pixels, image_size);
        stbi_image_free(pixels);
    }
    vmaUnmapMemory(m_allocator, staging_buffer->vma_allocation);

    vk::Format format = vk::Format::eR8G8B8A8Srgb;

    vk::ImageCreateInfo image_create_info{};
    image_create_info.sType = vk::StructureType::eImageCreateInfo;
    image_create_info.imageType = vk::ImageType::e2D;
    image_create_info.extent.width = static_cast<u32>(width);
    image_create_info.extent.height = static_cast<u32>(height);
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 6;
    image_create_info.format = format;
    image_create_info.tiling = vk::ImageTiling::eOptimal;
    image_create_info.initialLayout = vk::ImageLayout::eUndefined;
    image_create_info.usage = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled;
    image_create_info.sharingMode = vk::SharingMode::eExclusive;
    image_create_info.samples = vk::SampleCountFlagBits::e1;
    image_create_info.flags = vk::ImageCreateFlagBits::eCubeCompatible;

    VmaAllocationCreateInfo memory_info{};
    memory_info.usage = VMA_MEMORY_USAGE_GPU_ONLY;

    vmaCreateImage(m_allocator, reinterpret_cast<const VkImageCreateInfo*>(&image_create_info), &memory_info, &cubemap->vk_image, &cubemap->vma_allocation, nullptr);

    transition_image_layout(cubemap->vk_image, format, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, 6);

    copy_buffer_to_image(staging_buffer->vk_buffer, cubemap->vk_image, static_cast<u32>(width), static_cast<u32>(height), 6);

    transition_image_layout(cubemap->vk_image, format, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal, 6);

    cubemap->vk_image_view = create_image_view(cubemap->vk_image, vk::ImageViewType::eCube, format, vk::ImageAspectFlagBits::eColor, 6);

    destroy_buffer(staging_handle);

    return handle;
}

SamplerHandle Renderer::create_sampler(const SamplerConfig& sampler_config)
{
    SamplerHandle sampler_handle = (SamplerHandle)m_sampler_pool.acquire();
    auto* sampler = static_cast<Sampler*>(m_sampler_pool.access(sampler_handle));

    vk::SamplerCreateInfo sampler_info{};
    sampler_info.sType = vk::StructureType::eSamplerCreateInfo;
    sampler_info.magFilter = sampler_config.mag_filter;
    sampler_info.minFilter = sampler_config.min_filter;

    sampler_info.addressModeU = sampler_config.u_mode;
    sampler_info.addressModeV = sampler_config.v_mode;
    sampler_info.addressModeW = sampler_config.w_mode;

    vk::PhysicalDeviceProperties properties{};
    m_physical_device.getProperties(&properties);

    sampler_info.anisotropyEnable = true;
    sampler_info.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
    sampler_info.borderColor = vk::BorderColor::eIntOpaqueBlack;
    sampler_info.unnormalizedCoordinates = false;
    sampler_info.compareEnable = false;
    sampler_info.compareOp = vk::CompareOp::eAlways;
    sampler_info.mipmapMode = vk::SamplerMipmapMode::eLinear;
    sampler_info.mipLodBias = 0.f;
    sampler_info.minLod = 0.f;
    sampler_info.maxLod = 0.f;

    check(m_logical_device.createSampler(&sampler_info, nullptr, &sampler->vk_sampler));

    return sampler_handle;
}

DescriptorSetLayoutHandle Renderer::create_descriptor_set_layout(const DescriptorSetLayoutConfig& descriptor_set_layout_config)
{
    DescriptorSetLayoutHandle handle = (DescriptorSetLayoutHandle)m_descriptor_set_layout_pool.acquire();
    auto* descriptor_set_layout = static_cast<DescriptorSetLayout*>(m_descriptor_set_layout_pool.access(handle));

    vk::DescriptorSetLayoutBinding layout_bindings[descriptor_set_layout_config.num_bindings];
    for(u32 i = 0; i < descriptor_set_layout_config.num_bindings; ++i)
    {
        layout_bindings[i].binding = descriptor_set_layout_config.bindings[i].binding_index;
        layout_bindings[i].descriptorType = descriptor_set_layout_config.bindings[i].type;
        layout_bindings[i].descriptorCount = descriptor_set_layout_config.bindings[i].descriptor_count;
        layout_bindings[i].stageFlags = descriptor_set_layout_config.bindings[i].stage_flags;
        layout_bindings[i].pImmutableSamplers = nullptr;
    }

    vk::DescriptorSetLayoutCreateInfo layout_create_info{};
    layout_create_info.sType = vk::StructureType::eDescriptorSetLayoutCreateInfo;
    layout_create_info.bindingCount = descriptor_set_layout_config.num_bindings;
    layout_create_info.pBindings = layout_bindings;
    layout_create_info.flags = descriptor_set_layout_config.flags;
    layout_create_info.pNext = descriptor_set_layout_config.extension;

    check(m_logical_device.createDescriptorSetLayout(&layout_create_info, nullptr, &descriptor_set_layout->vk_descriptor_set_layout));

    return handle;
}

DescriptorSetHandle Renderer::create_descriptor_set(const DescriptorSetConfig& descriptor_set_config)
{
    DescriptorSetHandle descriptor_set_handle = (DescriptorSetHandle)m_descriptor_set_pool.acquire();
    auto* descriptor_set = static_cast<DescriptorSet*>(m_descriptor_set_pool.access(descriptor_set_handle));

    vk::DescriptorSetAllocateInfo allocInfo = {};
    allocInfo.sType = vk::StructureType::eDescriptorSetAllocateInfo;
    allocInfo.descriptorPool = m_descriptor_pool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &descriptor_set_config.layout;

    check(m_logical_device.allocateDescriptorSets(&allocInfo, &descriptor_set->vk_descriptor_set));

    vk::WriteDescriptorSet descriptor_writes[descriptor_set_config.num_resources];

    for(int i = 0; i < descriptor_set_config.num_resources; ++i)
    {
        switch(descriptor_set_config.types[i])
        {
            case vk::DescriptorType::eUniformBuffer:
            {
                auto* buffer = static_cast<Buffer*>(m_buffer_pool.access(descriptor_set_config.resource_handles[i]));

                vk::DescriptorBufferInfo descriptor_info{};
                descriptor_info.buffer = buffer->vk_buffer;
                descriptor_info.offset = 0;
                descriptor_info.range = buffer->size;

                descriptor_writes[i].sType = vk::StructureType::eWriteDescriptorSet;
                descriptor_writes[i].dstSet = descriptor_set->vk_descriptor_set;
                descriptor_writes[i].dstBinding = descriptor_set_config.bindings[i];
                descriptor_writes[i].dstArrayElement = 0;
                descriptor_writes[i].descriptorType = vk::DescriptorType::eUniformBuffer;
                descriptor_writes[i].descriptorCount = 1;
                descriptor_writes[i].pBufferInfo = &descriptor_info;

                // used for descriptors that reference image data
                descriptor_writes[i].pImageInfo = nullptr;
                descriptor_writes[i].pTexelBufferView = nullptr;

                m_logical_device.updateDescriptorSets(1, &descriptor_writes[i], 0, nullptr);

                break;
            }
            case vk::DescriptorType::eCombinedImageSampler:
            {
                auto* texture = static_cast<Texture*>(m_texture_pool.access(descriptor_set_config.resource_handles[i]));
                auto* sampler = (descriptor_set_config.sampler_handles[i].index == k_invalid_sampler_handle.index) ? static_cast<Sampler*>(m_sampler_pool.access(m_default_sampler)) : static_cast<Sampler*>(m_sampler_pool.access(descriptor_set_config.sampler_handles[i]));

                vk::DescriptorImageInfo descriptor_info{};
                descriptor_info.imageView = texture->vk_image_view;
                descriptor_info.sampler = sampler->vk_sampler;
                descriptor_info.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

                descriptor_writes[i].sType = vk::StructureType::eWriteDescriptorSet;
                descriptor_writes[i].dstSet = descriptor_set->vk_descriptor_set; // descriptor set to update
                descriptor_writes[i].dstBinding = descriptor_set_config.bindings[i];
                descriptor_writes[i].dstArrayElement = 0;
                descriptor_writes[i].descriptorType = vk::DescriptorType::eCombinedImageSampler;
                descriptor_writes[i].descriptorCount = 1; // how many array elements to update
                descriptor_writes[i].pImageInfo = &descriptor_info;

                m_logical_device.updateDescriptorSets(1, &descriptor_writes[i], 0, nullptr);

                break;
            }
        }
    }

    // m_logical_device.updateDescriptorSets(descriptor_set_creation.num_resources, descriptor_writes, 0, nullptr);

    return descriptor_set_handle;
}

PipelineHandle Renderer::create_pipeline(const PipelineConfig& pipeline_config)
{
    PipelineHandle handle = (PipelineHandle)m_pipeline_pool.acquire();
    auto* pipeline = static_cast<Pipeline*>(m_pipeline_pool.access(handle));

    pipeline->vk_bindpoint = vk::PipelineBindPoint::eGraphics;

    bool cache_exists = (pipeline_config.pipeline_cache_location && std::filesystem::exists(pipeline_config.pipeline_cache_location));
    bool cache_header_valid = false;
    vk::PipelineCache pipeline_cache = nullptr;

    vk::PipelineCacheCreateInfo pipeline_cache_info{};
    pipeline_cache_info.sType = vk::StructureType::ePipelineCacheCreateInfo;

    if(cache_exists)
    {
        std::vector<u8> pipeline_cache_data = fileop::read_binary_file(pipeline_config.pipeline_cache_location);

        // if there is a new driver version there is a chance that it won't be able to make use of the old cache file
        // need to check some cache header details and compare them to our physical device
        // if they match, create the cache like normal, otherwise need to overwrite it
        auto* cache_header = (vk::PipelineCacheHeaderVersionOne*)pipeline_cache_data.data();
        cache_header_valid = (cache_header->deviceID == m_device_properties.deviceID &&
                              cache_header->vendorID == m_device_properties.vendorID &&
                              memcmp(cache_header->pipelineCacheUUID, m_device_properties.pipelineCacheUUID, VK_UUID_SIZE) == 0);

        if(cache_header_valid)
        {
            pipeline_cache_info.pInitialData = pipeline_cache_data.data();
            pipeline_cache_info.initialDataSize = pipeline_cache_data.size();
        }
    }

    check(m_logical_device.createPipelineCache(&pipeline_cache_info, nullptr, &pipeline_cache));

    vk::PipelineShaderStageCreateInfo shader_create_infos[pipeline_config.shader_count];
    vk::ShaderModule shader_modules[pipeline_config.shader_count];
    for(u32 i = 0; i < pipeline_config.shader_count; ++i)
    {
        auto shader_code = fileop::read_binary_file(pipeline_config.shader_stages[i].shader_file);
        shader_modules[i] = create_shader_module(shader_code);

        shader_create_infos[i].sType = vk::StructureType::ePipelineShaderStageCreateInfo;
        shader_create_infos[i].stage = pipeline_config.shader_stages[i].stage_flags;
        shader_create_infos[i].module = shader_modules[i];
        shader_create_infos[i].pName = pipeline_config.shader_stages[i].entry_point;
    }

    vk::VertexInputAttributeDescription attribute_descriptions[pipeline_config.vertex_attribute_count];
    for(u32 i = 0; i < pipeline_config.vertex_attribute_count; ++i)
    {
        attribute_descriptions[i].binding = pipeline_config.vertex_binding_description.binding;
        attribute_descriptions[i].location = pipeline_config.vertex_attributes[i].location;
        attribute_descriptions[i].format = pipeline_config.vertex_attributes[i].format;
        attribute_descriptions[i].offset = pipeline_config.vertex_attributes[i].offset;
    }

    vk::PipelineVertexInputStateCreateInfo vertex_input_info{};
    vertex_input_info.sType = vk::StructureType::ePipelineVertexInputStateCreateInfo;
    vertex_input_info.vertexBindingDescriptionCount = 1;
    vertex_input_info.pVertexBindingDescriptions = &pipeline_config.vertex_binding_description;
    vertex_input_info.vertexAttributeDescriptionCount = pipeline_config.vertex_attribute_count;
    vertex_input_info.pVertexAttributeDescriptions = attribute_descriptions;

    std::array<vk::DynamicState, 2> dynamic_states =
    {
            vk::DynamicState::eViewport,
            vk::DynamicState::eScissor
    };

    // the configuration of these values will be ignored, so they can be changed at runtime
    vk::PipelineDynamicStateCreateInfo dynamic_state{};
    dynamic_state.sType = vk::StructureType::ePipelineDynamicStateCreateInfo;
    dynamic_state.dynamicStateCount = static_cast<u32>(dynamic_states.size());
    dynamic_state.pDynamicStates = dynamic_states.data();

    vk::Viewport viewport{};
    viewport.x = 0.f;
    viewport.y = 0.f;
    viewport.width = (f32)m_swapchain_extent.width;
    viewport.height = (f32)m_swapchain_extent.height;
    viewport.minDepth = 0.f;
    viewport.maxDepth = 1.f;

    vk::Rect2D scissor{};
    scissor.offset.x = 0;
    scissor.offset.y = 0;
    scissor.extent = m_swapchain_extent;

    // since we made viewport and scissor dynamic we don't need to bind them here
    vk::PipelineViewportStateCreateInfo viewport_state{};
    viewport_state.sType = vk::StructureType::ePipelineViewportStateCreateInfo;
    viewport_state.viewportCount = 1;
    viewport_state.scissorCount = 1;

    vk::PipelineMultisampleStateCreateInfo multi_sampling{};
    multi_sampling.sType = vk::StructureType::ePipelineMultisampleStateCreateInfo;
    multi_sampling.sampleShadingEnable = false;
    multi_sampling.rasterizationSamples = vk::SampleCountFlagBits::e1;

    vk::PipelineColorBlendStateCreateInfo colour_blending{};
    colour_blending.sType = vk::StructureType::ePipelineColorBlendStateCreateInfo;
    colour_blending.logicOpEnable = false;
    colour_blending.logicOp = vk::LogicOp::eCopy;
    colour_blending.attachmentCount = pipeline_config.colour_attachment_count;
    colour_blending.pAttachments = pipeline_config.colour_attachments;

    vk::PipelineLayoutCreateInfo pipeline_layout_create_info{};
    pipeline_layout_create_info.sType = vk::StructureType::ePipelineLayoutCreateInfo;
    pipeline_layout_create_info.setLayoutCount = pipeline_config.descriptor_set_layout_count;

    vk::DescriptorSetLayout descriptor_set_layouts[pipeline_config.descriptor_set_layout_count];
    for(u32 i = 0; i < pipeline_config.descriptor_set_layout_count; ++i)
    {
        auto* descriptor_set_layout = static_cast<DescriptorSetLayout*>(m_descriptor_set_layout_pool.access(pipeline_config.descriptor_set_layouts[i]));
        descriptor_set_layouts[i] = descriptor_set_layout->vk_descriptor_set_layout;
    }
    pipeline_layout_create_info.pSetLayouts = descriptor_set_layouts;

    if(pipeline_config.push_constant_count > 0)
    {
        pipeline_layout_create_info.pushConstantRangeCount = pipeline_config.push_constant_count;
        pipeline_layout_create_info.pPushConstantRanges = pipeline_config.push_constants;
    }

    check(m_logical_device.createPipelineLayout(&pipeline_layout_create_info, nullptr, &pipeline->vk_pipeline_layout));

    vk::GraphicsPipelineCreateInfo pipeline_create_info{};
    pipeline_create_info.sType = vk::StructureType::eGraphicsPipelineCreateInfo;
    pipeline_create_info.stageCount = pipeline_config.shader_count;
    pipeline_create_info.pStages = shader_create_infos;
    pipeline_create_info.pVertexInputState = &vertex_input_info;
    pipeline_create_info.pInputAssemblyState = &pipeline_config.pipeline_input_assembly;
    pipeline_create_info.pViewportState = &viewport_state;
    pipeline_create_info.pRasterizationState = &pipeline_config.rasterization;
    pipeline_create_info.pMultisampleState = &multi_sampling;
    pipeline_create_info.pColorBlendState = &colour_blending;
    pipeline_create_info.pDynamicState = &dynamic_state;
    pipeline_create_info.layout = pipeline->vk_pipeline_layout;
    pipeline_create_info.renderPass = m_viewport_renderpass;
    pipeline_create_info.subpass = 0;
    pipeline_create_info.pDepthStencilState = &pipeline_config.depth_stencil_attachment;
    pipeline_create_info.basePipelineHandle = nullptr;
    pipeline_create_info.basePipelineIndex = -1;

    check(m_logical_device.createGraphicsPipelines(pipeline_cache, 1, &pipeline_create_info, nullptr, &pipeline->vk_pipeline));

    if(pipeline_config.pipeline_cache_location && (!cache_exists || !cache_header_valid))
    {
        size_t cache_data_size = 0;

        check(m_logical_device.getPipelineCacheData(pipeline_cache, &cache_data_size, nullptr));

        void* cache_data = malloc(cache_data_size);

        check(m_logical_device.getPipelineCacheData(pipeline_cache, &cache_data_size, cache_data));

        fileop::write_binary_file(cache_data, cache_data_size, pipeline_config.pipeline_cache_location);
        free(cache_data);
    }

    m_logical_device.destroyPipelineCache(pipeline_cache, nullptr);

    for(u32 i = 0; i < pipeline_config.shader_count; ++i)
    {
        m_logical_device.destroyShaderModule(shader_modules[i]);
    }

    return handle;
}

void Renderer::create_image(u32 width, u32 height, vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties, vk::Image& image, vk::DeviceMemory& image_memory)
{
    vk::ImageCreateInfo image_info{};
    image_info.sType = vk::StructureType::eImageCreateInfo;
    image_info.imageType = vk::ImageType::e2D;
    image_info.extent.width = static_cast<u32>(width);
    image_info.extent.height = static_cast<u32>(height);
    image_info.extent.depth = 1;
    image_info.mipLevels = 1;
    image_info.arrayLayers = 1;
    image_info.format = format;
    image_info.tiling = tiling;
    image_info.initialLayout = vk::ImageLayout::eUndefined;
    image_info.usage = usage;
    image_info.sharingMode = vk::SharingMode::eExclusive;
    image_info.samples = vk::SampleCountFlagBits::e1;

    check(m_logical_device.createImage(&image_info, nullptr, &image));

    vk::MemoryRequirements memory_requirements;
    m_logical_device.getImageMemoryRequirements(image, &memory_requirements);

    vk::MemoryAllocateInfo alloc_info{};
    alloc_info.sType = vk::StructureType::eMemoryAllocateInfo;
    alloc_info.allocationSize = memory_requirements.size;
    alloc_info.memoryTypeIndex = devh::find_memory_type(memory_requirements.memoryTypeBits, properties, m_physical_device);

    check(m_logical_device.allocateMemory(&alloc_info, nullptr, &image_memory));
    m_logical_device.bindImageMemory(image, image_memory, 0);
}

vk::ImageView Renderer::create_image_view(const vk::Image& image, vk::ImageViewType image_view_type, vk::Format format, vk::ImageAspectFlags image_aspect, u32 layer_count)
{
    vk::ImageViewCreateInfo imageview_create_info{};
    imageview_create_info.sType = vk::StructureType::eImageViewCreateInfo;
    imageview_create_info.image = image;
    imageview_create_info.viewType = image_view_type;
    imageview_create_info.format = format;
    imageview_create_info.components.r = vk::ComponentSwizzle::eIdentity;
    imageview_create_info.components.g = vk::ComponentSwizzle::eIdentity;
    imageview_create_info.components.b = vk::ComponentSwizzle::eIdentity;
    imageview_create_info.components.a = vk::ComponentSwizzle::eIdentity;
    imageview_create_info.subresourceRange.aspectMask = image_aspect;
    imageview_create_info.subresourceRange.baseMipLevel = 0;
    imageview_create_info.subresourceRange.levelCount = 1;
    imageview_create_info.subresourceRange.baseArrayLayer = 0;
    imageview_create_info.subresourceRange.layerCount = layer_count;

    vk::ImageView image_view;
    check(m_logical_device.createImageView(&imageview_create_info, nullptr, &image_view));

    return image_view;
}

vk::ShaderModule Renderer::create_shader_module(const std::vector<char>& code)
{
    vk::ShaderModuleCreateInfo create_info{};
    create_info.sType = vk::StructureType::eShaderModuleCreateInfo;
    create_info.codeSize = code.size();
    create_info.pCode = (const u32*)(code.data());

    vk::ShaderModule shader_module;
    check(m_logical_device.createShaderModule(&create_info, nullptr, &shader_module));

    return shader_module;
}

void Renderer::destroy_buffer(BufferHandle buffer_handle)
{
    auto* buffer = static_cast<Buffer*>(m_buffer_pool.access(buffer_handle));
    vmaDestroyBuffer(m_allocator, buffer->vk_buffer, buffer->vma_allocation);
    m_buffer_pool.free(buffer_handle);
}

void Renderer::destroy_texture(TextureHandle texture_handle)
{
    if(!m_texture_pool.valid_handle(texture_handle))
    {
        return;
    }

    auto* texture = static_cast<Texture*>(m_texture_pool.access(texture_handle));
    m_logical_device.destroyImageView(texture->vk_image_view, nullptr);
    vmaDestroyImage(m_allocator, texture->vk_image, texture->vma_allocation);
    m_texture_pool.free(texture_handle);
    if(texture->name)
    {
        m_texture_map.erase(texture->name);
    }
}

void Renderer::destroy_sampler(SamplerHandle sampler_handle)
{
    auto* sampler = static_cast<Sampler*>(m_sampler_pool.access(sampler_handle));
    m_logical_device.destroySampler(sampler->vk_sampler, nullptr);
    m_sampler_pool.free(sampler_handle);
}

void Renderer::destroy_descriptor_set_layout(DescriptorSetLayoutHandle set_layout_handle)
{
    auto* descriptor_set_layout = static_cast<DescriptorSetLayout*>(m_descriptor_set_layout_pool.access(set_layout_handle));
    m_logical_device.destroyDescriptorSetLayout(descriptor_set_layout->vk_descriptor_set_layout, nullptr);
    m_descriptor_set_layout_pool.free(set_layout_handle);
}

void Renderer::destroy_pipeline(PipelineHandle pipeline_handle)
{
    auto* pipeline = static_cast<Pipeline*>(m_pipeline_pool.access(pipeline_handle));
    m_logical_device.destroyPipelineLayout(pipeline->vk_pipeline_layout);
    m_logical_device.destroyPipeline(pipeline->vk_pipeline);
    m_pipeline_pool.free(pipeline_handle);
}

void Renderer::update_texture_set(TextureHandle* texture_handles, u32 num_textures)
{
    auto* texture_set = static_cast<DescriptorSet*>(m_descriptor_set_pool.access(m_texture_set));

    vk::DescriptorImageInfo descriptor_image_infos[num_textures];
    vk::WriteDescriptorSet descriptor_writes[num_textures];
    for(u32 i = 0; i < num_textures; ++i)
    {
        auto* texture = static_cast<Texture*>(m_texture_pool.access(texture_handles[i]));
        auto* sampler = static_cast<Sampler*>(m_sampler_pool.access(m_default_sampler));

        descriptor_image_infos[i].imageView = texture->vk_image_view;
        descriptor_image_infos[i].sampler = sampler->vk_sampler;
        descriptor_image_infos[i].imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

        descriptor_writes[i].sType = vk::StructureType::eWriteDescriptorSet;
        descriptor_writes[i].dstSet = texture_set->vk_descriptor_set;
        descriptor_writes[i].dstBinding = k_bindless_texture_binding;
        descriptor_writes[i].dstArrayElement = texture_handles[i].index;
        descriptor_writes[i].descriptorType = vk::DescriptorType::eCombinedImageSampler;
        descriptor_writes[i].descriptorCount = 1;
        descriptor_writes[i].pImageInfo = &descriptor_image_infos[i];
    }
    m_logical_device.updateDescriptorSets(num_textures, descriptor_writes, 0, nullptr);
}

void Renderer::init_instance()
{
#ifdef DEBUG
    if(!check_validation_layer_support()) fatal("Validation layers are not supported!")
#endif

    vk::ApplicationInfo app_info{};
    app_info.sType = vk::StructureType::eApplicationInfo;
    app_info.pApplicationName = "ToyBox";
    app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.pEngineName = "No Engine";
    app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.apiVersion = VK_API_VERSION_1_3;

    vk::InstanceCreateInfo instance_create_info{};
    instance_create_info.sType =  vk::StructureType::eInstanceCreateInfo;
    instance_create_info.pApplicationInfo = &app_info;

    auto extensions = get_required_extensions();
    instance_create_info.enabledExtensionCount = static_cast<u32>(extensions.size());
    instance_create_info.ppEnabledExtensionNames = extensions.data();

#ifdef DEBUG
    vk::DebugUtilsMessengerCreateInfoEXT debug_create_info{};
    debug_create_info.sType = vk::StructureType::eDebugUtilsMessengerCreateInfoEXT;

    debug_create_info.messageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eError |
                                        vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
                                        vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose;

    debug_create_info.messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
                                    vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
                                    vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance;

    debug_create_info.pfnUserCallback = debug_callback;

    instance_create_info.pNext = &debug_create_info;
    instance_create_info.enabledLayerCount = static_cast<u32>(m_validation_layers.size());
    instance_create_info.ppEnabledLayerNames = m_validation_layers.data();
#else
    instance_create_info.enabledLayerCount = 0;
#endif

    check(vk::createInstance(&instance_create_info, nullptr, &m_instance));

    m_dispatch_loader = vk::DispatchLoaderDynamic(m_instance, vkGetInstanceProcAddr);

#ifdef DEBUG
   check(m_instance.createDebugUtilsMessengerEXT(&debug_create_info, nullptr, &m_debug_messenger, m_dispatch_loader));
#endif
}

void Renderer::init_surface()
{
    VkSurfaceKHR c_style_surface;
    if(glfwCreateWindowSurface(m_instance, m_window, nullptr, &c_style_surface) != VK_SUCCESS)
    {
        fatal("Could not create window surface!");
    }
    m_surface = c_style_surface;
}

void Renderer::init_device()
{
    vk::PhysicalDeviceFeatures physical_device_features{};
    physical_device_features.samplerAnisotropy = true;

    // setup for bindless resources
    vk::PhysicalDeviceVulkan12Features physical_device_features12{};
    physical_device_features12.sType = vk::StructureType::ePhysicalDeviceVulkan12Features;
    physical_device_features12.descriptorBindingSampledImageUpdateAfterBind = true;
    physical_device_features12.descriptorBindingStorageImageUpdateAfterBind = true;
    physical_device_features12.descriptorBindingStorageBufferUpdateAfterBind = true;
    physical_device_features12.descriptorIndexing = true;
    physical_device_features12.descriptorBindingPartiallyBound = true;
    physical_device_features12.runtimeDescriptorArray = true;
    physical_device_features12.shaderSampledImageArrayNonUniformIndexing = true;

    vk::DeviceCreateInfo device_create_info{};
    device_create_info.sType = vk::StructureType::eDeviceCreateInfo;
    device_create_info.pEnabledFeatures = &physical_device_features;
    device_create_info.pNext = &physical_device_features12;

#ifdef DEBUG
    device_create_info.enabledLayerCount = static_cast<u32>(m_validation_layers.size());
    device_create_info.ppEnabledLayerNames = m_validation_layers.data();
#else
    device_create_info.enabledLayerCount = 0;
#endif

    m_physical_device = devh::pick_physical_device(m_instance, m_required_device_extensions);
    devh::QueueFamilies indices = devh::find_queue_families(m_physical_device, m_surface);

    std::vector<vk::DeviceQueueCreateInfo> queue_create_infos;
    std::set<u32> unique_indices = {indices.graphics_family.value(), indices.present_family.value(), indices.transfer_family.value()};
    f32 queue_priority = 1.f;
    for (u32 queue_family: unique_indices)
    {
        vk::DeviceQueueCreateInfo queue_create_info{};
        queue_create_info.sType = vk::StructureType::eDeviceQueueCreateInfo;
        queue_create_info.queueFamilyIndex = queue_family;
        queue_create_info.queueCount = 1;
        queue_create_info.pQueuePriorities = &queue_priority;
        queue_create_infos.push_back(queue_create_info);
    }

    device_create_info.queueCreateInfoCount = static_cast<u32>(queue_create_infos.size());
    device_create_info.pQueueCreateInfos = queue_create_infos.data();

    device_create_info.enabledExtensionCount = static_cast<u32>(m_required_device_extensions.size());
    device_create_info.ppEnabledExtensionNames = m_required_device_extensions.data();
    
    check(m_physical_device.createDevice(&device_create_info, nullptr, &m_logical_device));

    // only using one queue per family
    m_logical_device.getQueue(indices.graphics_family.value(), 0, &m_graphics_queue);
    m_logical_device.getQueue(indices.present_family.value(), 0, &m_present_queue);
    m_logical_device.getQueue(indices.transfer_family.value(), 0, &m_transfer_queue);
    m_physical_device.getProperties(&m_device_properties);

    VmaAllocatorCreateInfo vma_info{};
    vma_info.vulkanApiVersion = m_device_properties.apiVersion;
    vma_info.physicalDevice = m_physical_device;
    vma_info.device = m_logical_device;
    vma_info.instance = m_instance;
    vmaCreateAllocator(&vma_info, &m_allocator);
}

void Renderer::init_swapchain()
{
    devh::SwapChainSupportDetails swap_chain_support = devh::query_swap_chain_support(m_physical_device, m_surface);

    i32 width, height;
    glfwGetFramebufferSize(m_window, &width, &height);
    vk::Extent2D extent = { static_cast<u32>(width), static_cast<u32>(height) };

    vk::SurfaceFormatKHR surface_format = devh::choose_swap_surface_format(swap_chain_support.formats);
    vk::PresentModeKHR present_mode = devh::choose_swap_present_mode(swap_chain_support.present_modes);
    vk::Extent2D actual_extent = devh::choose_swap_extent(swap_chain_support.capabilities, extent);

    // 1 extra than the min to avoid GPU hangs
    u32 image_count = swap_chain_support.capabilities.minImageCount + 1;

    if((swap_chain_support.capabilities.maxImageCount > 0) && (image_count > swap_chain_support.capabilities.maxImageCount))
    {
        image_count = swap_chain_support.capabilities.maxImageCount;
    }

    vk::SwapchainCreateInfoKHR create_info{};
    create_info.sType = vk::StructureType::eSwapchainCreateInfoKHR;
    create_info.surface = m_surface;
    create_info.minImageCount = image_count;
    create_info.imageFormat = surface_format.format;
    create_info.imageColorSpace = surface_format.colorSpace;
    create_info.imageExtent = actual_extent;
    create_info.imageArrayLayers = 1; // amount of layers each image consists of
    create_info.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;

    devh::QueueFamilies indices = devh::find_queue_families(m_physical_device, m_surface);
    u32 queue_family_indices[] = { indices.graphics_family.value(), indices.present_family.value() };

    if(indices.graphics_family != indices.present_family)
    {
        create_info.imageSharingMode = vk::SharingMode::eConcurrent;
        create_info.queueFamilyIndexCount = 2;
        create_info.pQueueFamilyIndices = queue_family_indices;
    }
    else
    {
        create_info.imageSharingMode = vk::SharingMode::eExclusive;
    }
    create_info.preTransform = swap_chain_support.capabilities.currentTransform;
    create_info.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
    create_info.presentMode = present_mode;
    create_info.clipped = true;
    create_info.oldSwapchain = nullptr;

    check(m_logical_device.createSwapchainKHR(&create_info, nullptr, &m_swapchain));

    check(m_logical_device.getSwapchainImagesKHR(m_swapchain, &image_count, nullptr));
    m_swapchain_images.resize(image_count);

    check(m_logical_device.getSwapchainImagesKHR(m_swapchain, &image_count, m_swapchain_images.data()));

    m_swapchain_image_format = surface_format.format;
    m_swapchain_extent = actual_extent;

    m_swapchain_image_views.resize(m_swapchain_images.size());

    for(size_t i = 0; i < m_swapchain_images.size(); ++i)
    {
        m_swapchain_image_views[i] = create_image_view(m_swapchain_images[i], vk::ImageViewType::e2D, m_swapchain_image_format, vk::ImageAspectFlagBits::eColor);
    }
}

void Renderer::init_renderpasses()
{
    {
        vk::AttachmentDescription colour_attachment{};
        colour_attachment.format = m_swapchain_image_format;
        colour_attachment.samples = vk::SampleCountFlagBits::e1;
        colour_attachment.loadOp = vk::AttachmentLoadOp::eClear;
        colour_attachment.storeOp = vk::AttachmentStoreOp::eStore;
        colour_attachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
        colour_attachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
        colour_attachment.initialLayout = vk::ImageLayout::eUndefined;
        colour_attachment.finalLayout = vk::ImageLayout::eColorAttachmentOptimal;

        vk::AttachmentDescription depth_attachment{};
        depth_attachment.format = vk::Format::eD32Sfloat;
        depth_attachment.samples = vk::SampleCountFlagBits::e1;
        depth_attachment.loadOp = vk::AttachmentLoadOp::eClear;
        depth_attachment.storeOp = vk::AttachmentStoreOp::eStore;
        depth_attachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
        depth_attachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
        depth_attachment.initialLayout = vk::ImageLayout::eUndefined;
        depth_attachment.finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

        vk::AttachmentReference colour_attachment_ref{};
        colour_attachment_ref.attachment = 0;
        colour_attachment_ref.layout = vk::ImageLayout::eColorAttachmentOptimal;

        vk::AttachmentReference depth_attachment_ref{};
        depth_attachment_ref.attachment = 1;
        depth_attachment_ref.layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

        vk::SubpassDescription subpass{};
        subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colour_attachment_ref;
        subpass.pDepthStencilAttachment = &depth_attachment_ref;

        vk::SubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests;
        dependency.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests;
        dependency.srcAccessMask = vk::AccessFlagBits::eNone;
        dependency.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentWrite;

        std::array<vk::AttachmentDescription, 2> attachments = {colour_attachment, depth_attachment};
        vk::RenderPassCreateInfo render_pass_info{};
        render_pass_info.sType = vk::StructureType::eRenderPassCreateInfo;
        render_pass_info.attachmentCount = static_cast<u32>(attachments.size());
        render_pass_info.pAttachments = attachments.data();
        render_pass_info.subpassCount = 1;
        render_pass_info.pSubpasses = &subpass;
        render_pass_info.dependencyCount = 1;
        render_pass_info.pDependencies = &dependency;

        check(m_logical_device.createRenderPass(&render_pass_info, nullptr, &m_renderpass));
    }

    {
        vk::AttachmentDescription colour_attachment{};
        colour_attachment.format = m_swapchain_image_format;
        colour_attachment.samples = vk::SampleCountFlagBits::e1;
        colour_attachment.loadOp = vk::AttachmentLoadOp::eClear;
        colour_attachment.storeOp = vk::AttachmentStoreOp::eStore;
        colour_attachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
        colour_attachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
        colour_attachment.initialLayout = vk::ImageLayout::eUndefined;
        colour_attachment.finalLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

        vk::AttachmentDescription depth_attachment{};
        depth_attachment.format = vk::Format::eD32Sfloat;
        depth_attachment.samples = vk::SampleCountFlagBits::e1;
        depth_attachment.loadOp = vk::AttachmentLoadOp::eClear;
        depth_attachment.storeOp = vk::AttachmentStoreOp::eStore;
        depth_attachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
        depth_attachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
        depth_attachment.initialLayout = vk::ImageLayout::eUndefined;
        depth_attachment.finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

        vk::AttachmentReference colour_attachment_ref{};
        colour_attachment_ref.attachment = 0;
        colour_attachment_ref.layout = vk::ImageLayout::eColorAttachmentOptimal;

        vk::AttachmentReference depth_attachment_ref{};
        depth_attachment_ref.attachment = 1;
        depth_attachment_ref.layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

        vk::SubpassDescription subpass{};
        subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colour_attachment_ref;
        subpass.pDepthStencilAttachment = &depth_attachment_ref;

        std::array<vk::SubpassDependency, 2> dependencies;

        dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
        dependencies[0].dstSubpass = 0;
        dependencies[0].srcStageMask = vk::PipelineStageFlagBits::eFragmentShader;
        dependencies[0].dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
        dependencies[0].srcAccessMask = vk::AccessFlagBits::eShaderRead;
        dependencies[0].dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
        dependencies[0].dependencyFlags = vk::DependencyFlagBits::eByRegion;

        dependencies[1].srcSubpass = 0;
        dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
        dependencies[1].srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
        dependencies[1].dstStageMask = vk::PipelineStageFlagBits::eFragmentShader;
        dependencies[1].srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
        dependencies[1].dstAccessMask = vk::AccessFlagBits::eShaderRead;
        dependencies[1].dependencyFlags = vk::DependencyFlagBits::eByRegion;

        std::array<vk::AttachmentDescription, 2> attachments = {colour_attachment, depth_attachment};
        vk::RenderPassCreateInfo render_pass_info{};
        render_pass_info.sType = vk::StructureType::eRenderPassCreateInfo;
        render_pass_info.attachmentCount = static_cast<u32>(attachments.size());
        render_pass_info.pAttachments = attachments.data();
        render_pass_info.subpassCount = 1;
        render_pass_info.pSubpasses = &subpass;
        render_pass_info.dependencyCount = dependencies.size();
        render_pass_info.pDependencies = dependencies.data();

        check(m_logical_device.createRenderPass(&render_pass_info, nullptr, &m_viewport_renderpass));
    }

    {
        vk::AttachmentDescription colour_attachment{};
        colour_attachment.format = m_swapchain_image_format;
        colour_attachment.samples = vk::SampleCountFlagBits::e1;
        colour_attachment.loadOp = vk::AttachmentLoadOp::eClear;
        colour_attachment.storeOp = vk::AttachmentStoreOp::eStore;
        colour_attachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
        colour_attachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
        colour_attachment.initialLayout = vk::ImageLayout::eColorAttachmentOptimal;
        colour_attachment.finalLayout = vk::ImageLayout::ePresentSrcKHR;

        vk::AttachmentReference colour_attachment_ref{};
        colour_attachment_ref.attachment = 0;
        colour_attachment_ref.layout = vk::ImageLayout::eColorAttachmentOptimal;

        vk::SubpassDescription subpass{};
        subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colour_attachment_ref;

        vk::SubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
        dependency.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
        dependency.srcAccessMask = vk::AccessFlagBits::eNone;
        dependency.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;

        vk::RenderPassCreateInfo imgui_renderpass_create_info{};
        imgui_renderpass_create_info.sType = vk::StructureType::eRenderPassCreateInfo;
        imgui_renderpass_create_info.attachmentCount = 1;
        imgui_renderpass_create_info.pAttachments = &colour_attachment;
        imgui_renderpass_create_info.subpassCount = 1;
        imgui_renderpass_create_info.pSubpasses = &subpass;
        imgui_renderpass_create_info.dependencyCount = 1;
        imgui_renderpass_create_info.pDependencies = &dependency;

        check(m_logical_device.createRenderPass(&imgui_renderpass_create_info, nullptr, &m_imgui_renderpass));
    }
}

void Renderer::init_descriptor_pools()
{
    vk::DescriptorPoolSize pool_sizes[] =
    {
        { vk::DescriptorType::eUniformBuffer, k_max_bindless_resources },
        { vk::DescriptorType::eCombinedImageSampler, k_max_bindless_resources },
        { vk::DescriptorType::eStorageImage, k_max_bindless_resources }
    };

    vk::DescriptorPoolCreateInfo pool_create_info{};
    pool_create_info.sType = vk::StructureType::eDescriptorPoolCreateInfo;
    pool_create_info.flags = vk::DescriptorPoolCreateFlagBits::eUpdateAfterBindEXT; // for bindless resources
    pool_create_info.maxSets = k_max_descriptor_sets;
    pool_create_info.poolSizeCount = sizeof(pool_sizes) / sizeof(vk::DescriptorPoolSize);
    pool_create_info.pPoolSizes = pool_sizes;

    check(m_logical_device.createDescriptorPool(&pool_create_info, nullptr, &m_descriptor_pool));

    vk::DescriptorPoolSize normal_pool_sizes[] =
    {
        { vk::DescriptorType::eUniformBuffer, 10 },
        { vk::DescriptorType::eCombinedImageSampler, 10 },
        { vk::DescriptorType::eStorageImage, 10 }
    };

    vk::DescriptorPoolCreateInfo normal_pool_create_info{};
    normal_pool_create_info.sType = vk::StructureType::eDescriptorSetLayoutCreateInfo;
    normal_pool_create_info.maxSets = 10;
    normal_pool_create_info.poolSizeCount = sizeof(normal_pool_sizes) / sizeof(vk::DescriptorPoolSize);
    normal_pool_create_info.pPoolSizes = normal_pool_sizes;

    check(m_logical_device.createDescriptorPool(&pool_create_info, nullptr, &m_normal_descriptor_pool));
}

void Renderer::init_descriptor_sets()
{
    m_camera_data_layout = create_descriptor_set_layout({
        .bindings = {
            {
                .binding_index = 0,
                .type = vk::DescriptorType::eUniformBuffer,
                .stage_flags = vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment,
                .descriptor_count = 1
            }
        },
        .num_bindings = 1
    });
    auto* camera_descriptor_set_layout = static_cast<DescriptorSetLayout*>(m_descriptor_set_layout_pool.access(m_camera_data_layout));

    // create the buffers for the view/projection transforms
    u32 camera_buffer_size = pad_uniform_buffer(sizeof(CameraData));

    // TODO: combine into one big buffer
    for(int i = 0; i < k_max_frames_in_flight; ++i)
    {
        m_camera_buffers[i] = create_buffer({
            .usage = vk::BufferUsageFlagBits::eUniformBuffer,
            .size = camera_buffer_size,
            .persistent = true
        });
    }

    m_camera_sets.resize(k_max_frames_in_flight);

    // create the sets for the camera
    for(int i = 0; i < k_max_frames_in_flight; ++i)
    {
        m_camera_sets[i] = create_descriptor_set({
            .resource_handles = {m_camera_buffers[i].index},
            .bindings = {0},
            .types = {vk::DescriptorType::eUniformBuffer},
            .layout = camera_descriptor_set_layout->vk_descriptor_set_layout,
            .num_resources = 1,
        });
    }

    // bindless texture set layout
    vk::DescriptorBindingFlags bindless_flags = vk::DescriptorBindingFlagBits::ePartiallyBound | vk::DescriptorBindingFlagBits::eUpdateAfterBind;
    vk::DescriptorBindingFlags binding_flags[] = { bindless_flags, bindless_flags };
    vk::DescriptorSetLayoutBindingFlagsCreateInfoEXT extended_info{};
    extended_info.sType = vk::StructureType::eDescriptorSetLayoutBindingFlagsCreateInfoEXT;
    extended_info.bindingCount = 2;
    extended_info.pBindingFlags = binding_flags;

    m_texture_set_layout = create_descriptor_set_layout({
        .bindings = {
            {
                .binding_index = k_bindless_texture_binding,
                .type = vk::DescriptorType::eCombinedImageSampler,
                .stage_flags = vk::ShaderStageFlagBits::eFragment,
                .descriptor_count = k_max_bindless_resources
            },
            {
                .binding_index = k_bindless_image_binding,
                .type = vk::DescriptorType::eStorageImage,
                .descriptor_count = k_max_bindless_resources
            }
        },
        .num_bindings = 2,
        .flags = vk::DescriptorSetLayoutCreateFlagBits::eUpdateAfterBindPoolEXT,
        .extension = &extended_info
    });
    auto* texture_descriptor_set_layout = static_cast<DescriptorSetLayout*>(m_descriptor_set_layout_pool.access(m_texture_set_layout));

    m_texture_set = (DescriptorSetHandle)m_descriptor_set_pool.acquire();
    auto* descriptor_set = static_cast<DescriptorSet*>(m_descriptor_set_pool.access(m_texture_set));

    vk::DescriptorSetAllocateInfo allocInfo = {};
    allocInfo.sType = vk::StructureType::eDescriptorSetAllocateInfo;
    allocInfo.descriptorPool = m_descriptor_pool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &texture_descriptor_set_layout->vk_descriptor_set_layout;

    check(m_logical_device.allocateDescriptorSets(&allocInfo, &descriptor_set->vk_descriptor_set));
}

void Renderer::init_graphics_pipeline()
{
    PipelineConfig pipeline_config{
        .renderPass = m_renderpass,
        .pipeline_cache_location = "graphics_pipeline_cache.bin"
    };

    pipeline_config.set_rasterizer({
       .polygon_mode = vk::PolygonMode::eFill,
       .cull_mode = vk::CullModeFlagBits::eBack,
       .front_face = vk::FrontFace::eCounterClockwise
    });

    pipeline_config.set_binding_description({
        .binding = 0,
        .stride = sizeof(Vertex)
    });

    pipeline_config.set_input_assembly(vk::PrimitiveTopology::eTriangleList);

    pipeline_config.add_shader_stage({
        .shader_file = "../assets/shaders/vert.spv",
        .stage_flags = vk::ShaderStageFlagBits::eVertex
    });

    pipeline_config.add_shader_stage({
        .shader_file = "../assets/shaders/frag.spv",
        .stage_flags = vk::ShaderStageFlagBits::eFragment
    });

    auto attribute_descriptions = Vertex::get_attribute_description();

    for(auto attribute_description : attribute_descriptions)
    {
        pipeline_config.add_vertex_attribute({
            .location = attribute_description.location,
            .format = attribute_description.format,
            .offset = attribute_description.offset
        });
    }

    pipeline_config.add_descriptor_set_layout(m_camera_data_layout);
    pipeline_config.add_descriptor_set_layout(m_texture_set_layout);

    vk::PipelineColorBlendAttachmentState colour_blend_attachment{};
    colour_blend_attachment.colorWriteMask = vk::ColorComponentFlagBits::eR
                                             | vk::ColorComponentFlagBits::eG
                                             | vk::ColorComponentFlagBits::eB
                                             | vk::ColorComponentFlagBits::eA;
    colour_blend_attachment.blendEnable = false;
    colour_blend_attachment.srcColorBlendFactor = vk::BlendFactor::eOne;
    colour_blend_attachment.dstColorBlendFactor = vk::BlendFactor::eZero;
    colour_blend_attachment.colorBlendOp = vk::BlendOp::eAdd;
    colour_blend_attachment.srcAlphaBlendFactor = vk::BlendFactor::eOne;
    colour_blend_attachment.dstAlphaBlendFactor = vk::BlendFactor::eZero;
    colour_blend_attachment.alphaBlendOp = vk::BlendOp::eAdd;

    vk::PipelineDepthStencilStateCreateInfo depth_stencil{};
    depth_stencil.sType = vk::StructureType::ePipelineDepthStencilStateCreateInfo;
    depth_stencil.depthTestEnable = true;
    depth_stencil.depthWriteEnable = true;
    depth_stencil.depthCompareOp = vk::CompareOp::eLess;
    depth_stencil.depthBoundsTestEnable = false;
    depth_stencil.minDepthBounds = 0.f;
    depth_stencil.maxDepthBounds = 1.f;
    depth_stencil.stencilTestEnable = false;

    pipeline_config.add_colour_attachment(colour_blend_attachment);
    pipeline_config.add_depth_stencil_attachment(depth_stencil);

    vk::PushConstantRange model_push_constant_info{};
    model_push_constant_info.offset = 0;
    model_push_constant_info.size = sizeof(glm::mat4);
    model_push_constant_info.stageFlags = vk::ShaderStageFlagBits::eVertex;

    vk::PushConstantRange texture_push_constant_info{};
    texture_push_constant_info.offset = 64;
    texture_push_constant_info.size = sizeof(glm::uvec4);
    texture_push_constant_info.stageFlags = vk::ShaderStageFlagBits::eFragment;

    pipeline_config.add_push_constant(model_push_constant_info);
    pipeline_config.add_push_constant(texture_push_constant_info);

    m_graphics_pipeline = create_pipeline(pipeline_config);

    pipeline_config.renderPass = m_viewport_renderpass;
    pipeline_config.pipeline_cache_location = "viewport_pipeline_cache.bin";
    m_viewport_pipeline = create_pipeline(pipeline_config);
}

void Renderer::init_viewport()
{
    m_viewport_images.resize(m_swapchain_images.size());
    m_viewport_memory.resize(m_swapchain_images.size());
    m_viewport_image_views.resize(m_swapchain_images.size());

    for(u32 i = 0; i < m_viewport_images.size(); ++i)
    {
        create_image(m_swapchain_extent.width, m_swapchain_extent.height,
                     vk::Format::eB8G8R8A8Srgb, vk::ImageTiling::eOptimal,
                     vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled,
                     vk::MemoryPropertyFlagBits::eDeviceLocal,
                     m_viewport_images[i], m_viewport_memory[i]);

        vk::CommandBuffer cmd_buffer = begin_single_time_commands();
        insert_image_memory_barrier(cmd_buffer, m_viewport_images[i], vk::AccessFlagBits::eTransferRead, vk::AccessFlagBits::eMemoryRead,
                                    vk::ImageLayout::eUndefined, vk::ImageLayout::eShaderReadOnlyOptimal,
                                    vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eTransfer,
                                    vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1});
        end_single_time_commands(cmd_buffer);

        m_viewport_image_views[i] = create_image_view(m_viewport_images[i], vk::ImageViewType::e2D, vk::Format::eB8G8R8A8Srgb, vk::ImageAspectFlagBits::eColor);
    }
}

void Renderer::init_command_pools()
{
    devh::QueueFamilies queue_family_indices = devh::find_queue_families(m_physical_device, m_surface);

    vk::CommandPoolCreateInfo pool_create_info{};
    pool_create_info.sType = vk::StructureType::eCommandPoolCreateInfo;
    pool_create_info.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
    pool_create_info.queueFamilyIndex = queue_family_indices.graphics_family.value();

    check(m_logical_device.createCommandPool(&pool_create_info, nullptr, &m_main_command_pool));
    check(m_logical_device.createCommandPool(&pool_create_info, nullptr, &m_viewport_command_pool));
    check(m_logical_device.createCommandPool(&pool_create_info, nullptr, &m_extra_command_pool));

    m_command_pools.resize(m_scheduler->GetNumTaskThreads());

    for(auto& command_pool : m_command_pools)
    {
        check(m_logical_device.createCommandPool(&pool_create_info, nullptr, &command_pool));
    }

    pool_create_info.queueFamilyIndex = queue_family_indices.transfer_family.value();
}

void Renderer::init_command_buffers()
{
    m_command_buffers.resize(m_command_pools.size() * k_max_frames_in_flight);

    u32 command_buffer_index = 0;
    for(const auto& command_pool : m_command_pools)
    {
        // now need to allocate the secondary command buffers for this pool
        vk::CommandBufferAllocateInfo secondary_alloc_info{};
        secondary_alloc_info.sType = vk::StructureType::eCommandBufferAllocateInfo;
        secondary_alloc_info.commandPool = command_pool;
        secondary_alloc_info.level = vk::CommandBufferLevel::eSecondary;
        secondary_alloc_info.commandBufferCount = 1;

        for(u32 i = 0; i < k_max_frames_in_flight; ++i)
        {
            check(m_logical_device.allocateCommandBuffers(&secondary_alloc_info, &m_command_buffers[i + command_buffer_index].get()));
        }

        command_buffer_index += k_max_frames_in_flight;
    }

    vk::CommandBufferAllocateInfo main_alloc_info{};
    main_alloc_info.sType = vk::StructureType::eCommandBufferAllocateInfo;
    main_alloc_info.commandPool = m_main_command_pool;
    main_alloc_info.level = vk::CommandBufferLevel::ePrimary;
    main_alloc_info.commandBufferCount = 1;

    vk::CommandBufferAllocateInfo viewport_commands_alloc_info{};
    viewport_commands_alloc_info.sType = vk::StructureType::eCommandBufferAllocateInfo;
    viewport_commands_alloc_info.commandPool = m_viewport_command_pool;
    viewport_commands_alloc_info.level = vk::CommandBufferLevel::ePrimary;
    viewport_commands_alloc_info.commandBufferCount = 1;

    vk::CommandBufferAllocateInfo extra_alloc_info{};
    extra_alloc_info.sType = vk::StructureType::eCommandBufferAllocateInfo;
    extra_alloc_info.commandPool = m_extra_command_pool;
    extra_alloc_info.level = vk::CommandBufferLevel::eSecondary;
    extra_alloc_info.commandBufferCount = 1;

    vk::CommandBufferAllocateInfo imgui_alloc_info{};
    imgui_alloc_info.sType = vk::StructureType::eCommandBufferAllocateInfo;
    imgui_alloc_info.commandPool = m_main_command_pool;
    imgui_alloc_info.level = vk::CommandBufferLevel::ePrimary;
    imgui_alloc_info.commandBufferCount = 1;

    for(u32 i = 0; i < k_max_frames_in_flight; ++i)
    {
        check(m_logical_device.allocateCommandBuffers(&main_alloc_info, &m_main_command_buffers[i].get()));
        check(m_logical_device.allocateCommandBuffers(&viewport_commands_alloc_info, &m_viewport_command_buffers[i].get()));
        check(m_logical_device.allocateCommandBuffers(&extra_alloc_info, &m_extra_draw_commands[i].get()));
        check(m_logical_device.allocateCommandBuffers(&extra_alloc_info, &m_skybox_commands[i].get()));
        check(m_logical_device.allocateCommandBuffers(&imgui_alloc_info, &m_imgui_commands[i].get()));
    }
}

void Renderer::init_depth_resources()
{
    create_image(m_swapchain_extent.width, m_swapchain_extent.height,
                 vk::Format::eD32Sfloat, vk::ImageTiling::eOptimal,
                 vk::ImageUsageFlagBits::eDepthStencilAttachment,
                 vk::MemoryPropertyFlagBits::eDeviceLocal,
                 m_depth_image, m_depth_image_memory);

    m_depth_image_view = create_image_view(m_depth_image, vk::ImageViewType::e2D, vk::Format::eD32Sfloat, vk::ImageAspectFlagBits::eDepth);
}

void Renderer::init_framebuffers()
{
    m_swapchain_framebuffers.resize(m_swapchain_image_views.size());
    m_viewport_framebuffers.resize(m_swapchain_image_views.size());
    m_imgui_framebuffers.resize(m_swapchain_image_views.size());

    for(u32 i = 0; i < m_swapchain_image_views.size(); ++i)
    {
        std::array<vk::ImageView, 2> attachments = { m_swapchain_image_views[i], m_depth_image_view };

        vk::FramebufferCreateInfo framebuffer_create_info{};
        framebuffer_create_info.sType = vk::StructureType::eFramebufferCreateInfo;
        framebuffer_create_info.renderPass = m_renderpass;
        framebuffer_create_info.attachmentCount = static_cast<u32>(attachments.size());
        framebuffer_create_info.pAttachments = attachments.data();
        framebuffer_create_info.width = m_swapchain_extent.width;
        framebuffer_create_info.height = m_swapchain_extent.height;
        framebuffer_create_info.layers = 1;

        std::array<vk::ImageView, 2> viewport_attachments = { m_viewport_image_views[i], m_depth_image_view };

        vk::FramebufferCreateInfo viewport_framebuffer_create_info{};
        viewport_framebuffer_create_info.sType = vk::StructureType::eFramebufferCreateInfo;
        viewport_framebuffer_create_info.renderPass = m_viewport_renderpass;
        viewport_framebuffer_create_info.attachmentCount = static_cast<u32>(viewport_attachments.size());
        viewport_framebuffer_create_info.pAttachments = viewport_attachments.data();
        viewport_framebuffer_create_info.width = m_swapchain_extent.width;
        viewport_framebuffer_create_info.height = m_swapchain_extent.height;
        viewport_framebuffer_create_info.layers = 1;

        vk::FramebufferCreateInfo imgui_framebuffer_create_info{};
        imgui_framebuffer_create_info.sType = vk::StructureType::eFramebufferCreateInfo;
        imgui_framebuffer_create_info.renderPass = m_imgui_renderpass;
        imgui_framebuffer_create_info.attachmentCount = 1;
        imgui_framebuffer_create_info.pAttachments = &m_swapchain_image_views[i];
        imgui_framebuffer_create_info.width = m_swapchain_extent.width;
        imgui_framebuffer_create_info.height = m_swapchain_extent.height;
        imgui_framebuffer_create_info.layers = 1;

        check(m_logical_device.createFramebuffer(&framebuffer_create_info, nullptr, &m_swapchain_framebuffers[i]));
        check(m_logical_device.createFramebuffer(&viewport_framebuffer_create_info, nullptr, &m_viewport_framebuffers[i]));
        check(m_logical_device.createFramebuffer(&imgui_framebuffer_create_info, nullptr, &m_imgui_framebuffers[i]));
    }
}

void Renderer::init_sync_objects()
{
    vk::SemaphoreCreateInfo semaphore_create_info{};
    semaphore_create_info.sType = vk::StructureType::eSemaphoreCreateInfo;

    vk::FenceCreateInfo fence_create_info{};
    fence_create_info.sType = vk::StructureType::eFenceCreateInfo;
    fence_create_info.flags = vk::FenceCreateFlagBits::eSignaled;

    for(size_t i = 0; i < k_max_frames_in_flight; ++i)
    {
        check(m_logical_device.createSemaphore(&semaphore_create_info, nullptr, &m_image_available_semaphores[i]));
        check(m_logical_device.createSemaphore(&semaphore_create_info, nullptr, &m_render_finished_semaphores[i]));
        check(m_logical_device.createFence(&fence_create_info, nullptr, &m_in_flight_fences[i]));
    }
}

void Renderer::init_imgui()
{
    vk::DescriptorPoolSize pool_sizes[] =
    {
        { vk::DescriptorType::eSampler, 1000 },
        { vk::DescriptorType::eCombinedImageSampler, 1000 },
        { vk::DescriptorType::eSampledImage, 1000 },
        { vk::DescriptorType::eStorageImage, 1000 },
        { vk::DescriptorType::eUniformTexelBuffer, 1000 },
        { vk::DescriptorType::eStorageTexelBuffer, 1000 },
        { vk::DescriptorType::eUniformBuffer, 1000 },
        { vk::DescriptorType::eStorageBuffer, 1000 },
        { vk::DescriptorType::eUniformBufferDynamic, 1000 },
        { vk::DescriptorType::eStorageBufferDynamic, 1000 },
        { vk::DescriptorType::eInputAttachment, 1000 }
    };

    vk::DescriptorPoolCreateInfo pool_create_info{};
    pool_create_info.sType = vk::StructureType::eDescriptorPoolCreateInfo;
    pool_create_info.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet;
    pool_create_info.maxSets = 1000;
    pool_create_info.poolSizeCount = std::size(pool_sizes);
    pool_create_info.pPoolSizes = pool_sizes;

    check(m_logical_device.createDescriptorPool(&pool_create_info, nullptr, &m_imgui_pool));

    ImGui::CreateContext();

    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    ImGui_ImplGlfw_InitForVulkan(m_window, true);

    ImGui_ImplVulkan_InitInfo init_info{};
    init_info.Instance = m_instance;
    init_info.PhysicalDevice = m_physical_device;
    init_info.Device = m_logical_device;
    init_info.Queue = m_graphics_queue;
    init_info.Subpass = 0;
    init_info.DescriptorPool = m_imgui_pool;
    init_info.MinImageCount = k_max_frames_in_flight;
    init_info.ImageCount = k_max_frames_in_flight;
    init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

    ImGui_ImplVulkan_Init(&init_info, m_imgui_renderpass);

    vk::CommandBuffer upload_fonts = begin_single_time_commands();
    ImGui_ImplVulkan_CreateFontsTexture(upload_fonts);
    end_single_time_commands(upload_fonts);

    // clear font textures from cpu data
    ImGui_ImplVulkan_DestroyFontUploadObjects();

    m_viewport_descriptors.resize(m_viewport_image_views.size());
    auto* default_sampler = static_cast<Sampler*>(m_sampler_pool.access(m_default_sampler));
    for (u32 i = 0; i < m_viewport_image_views.size(); ++i)
    {
        m_viewport_descriptors[i] = ImGui_ImplVulkan_AddTexture(default_sampler->vk_sampler, m_viewport_image_views[i], VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    }
}

vk::CommandBuffer Renderer::begin_single_time_commands()
{
    vk::CommandBufferAllocateInfo alloc_info{};
    alloc_info.sType = vk::StructureType::eCommandBufferAllocateInfo;
    alloc_info.level = vk::CommandBufferLevel::ePrimary;
    alloc_info.commandPool = m_main_command_pool;
    alloc_info.commandBufferCount = 1;

    vk::CommandBuffer command_buffer;
    check(m_logical_device.allocateCommandBuffers(&alloc_info, &command_buffer));

    vk::CommandBufferBeginInfo begin_info{};
    begin_info.sType = vk::StructureType::eCommandBufferBeginInfo;
    begin_info.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;

    check(command_buffer.begin(&begin_info));

    return command_buffer;
}

void Renderer::end_single_time_commands(vk::CommandBuffer command_buffer)
{
    command_buffer.end();

    vk::SubmitInfo  submit_info{};
    submit_info.sType = vk::StructureType::eSubmitInfo;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &command_buffer;

    check(m_graphics_queue.submit(1, &submit_info, nullptr));
    m_graphics_queue.waitIdle();

    m_logical_device.freeCommandBuffers(m_main_command_pool, 1, &command_buffer);
}

void Renderer::cleanup_swapchain()
{
    for(auto framebuffer : m_swapchain_framebuffers)
    {
        m_logical_device.destroyFramebuffer(framebuffer, nullptr);
    }

    for(auto framebuffer : m_imgui_framebuffers)
    {
        m_logical_device.destroyFramebuffer(framebuffer, nullptr);
    }

    for(auto framebuffer : m_viewport_framebuffers)
    {
        m_logical_device.destroyFramebuffer(framebuffer, nullptr);
    }

    for(auto image_view : m_swapchain_image_views)
    {
        m_logical_device.destroyImageView(image_view, nullptr);
    }

    for(u32 i = 0; i < m_viewport_images.size(); ++i)
    {
        m_logical_device.destroyImage(m_viewport_images[i], nullptr);
        m_logical_device.freeMemory(m_viewport_memory[i]);
        m_logical_device.destroyImageView(m_viewport_image_views[i], nullptr);
    }

    m_logical_device.destroyImage(m_depth_image, nullptr);
    m_logical_device.freeMemory(m_depth_image_memory);
    m_logical_device.destroyImageView(m_depth_image_view, nullptr);

    m_logical_device.destroySwapchainKHR(m_swapchain, nullptr);
}

void Renderer::recreate_swapchain()
{
    int width = 0, height = 0;
    glfwGetFramebufferSize(m_window, &width, &height);
    while (width == 0 || height == 0) {
        glfwGetFramebufferSize(m_window, &width, &height);
        glfwWaitEvents();
    }

    wait_for_device_idle();
    cleanup_swapchain();
    init_swapchain();
    init_depth_resources();
    init_viewport();
    init_framebuffers();

    m_viewport_descriptors.resize(m_viewport_image_views.size());
    auto* default_sampler = static_cast<Sampler*>(m_sampler_pool.access(m_default_sampler));
    for (u32 i = 0; i < m_viewport_image_views.size(); ++i)
    {
        m_viewport_descriptors[i] = ImGui_ImplVulkan_AddTexture(default_sampler->vk_sampler, m_viewport_image_views[i], VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    }

    m_swapchain_recreated = true;
}

size_t Renderer::pad_uniform_buffer(size_t original_size) const
{
    size_t alignment = m_device_properties.limits.minUniformBufferOffsetAlignment;
    size_t aligned_size = (alignment + original_size - 1) & ~(alignment - 1);
    return aligned_size;
}

void Renderer::copy_buffer_to_image(vk::Buffer buffer, vk::Image image, u32 width, u32 height, u32 layer_count)
{
    vk::CommandBuffer command_buffer = begin_single_time_commands();

    vk::BufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = layer_count;
    region.imageOffset = vk::Offset3D{0,0,0};
    region.imageExtent = vk::Extent3D
    {
            width,
            height,
            1
    };

    command_buffer.copyBufferToImage(buffer, image, vk::ImageLayout::eTransferDstOptimal, 1, &region);

    end_single_time_commands(command_buffer);
}

void Renderer::transition_image_layout(vk::Image image, vk::Format format, vk::ImageLayout old_layout, vk::ImageLayout new_layout, u32 layer_count)
{
    vk::CommandBuffer command_buffer = begin_single_time_commands();

    vk::PipelineStageFlags source_stage, destination_stage;

    vk::ImageMemoryBarrier barrier{};
    barrier.sType = vk::StructureType::eImageMemoryBarrier;
    barrier.oldLayout = old_layout;
    barrier.newLayout = new_layout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = layer_count;

    if(old_layout == vk::ImageLayout::eUndefined && new_layout == vk::ImageLayout::eTransferDstOptimal)
    {
        barrier.srcAccessMask = vk::AccessFlagBits::eNone;
        barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

        source_stage = vk::PipelineStageFlagBits::eTopOfPipe;
        destination_stage = vk::PipelineStageFlagBits::eTransfer;
    }
    else if(old_layout == vk::ImageLayout::eTransferDstOptimal && new_layout == vk::ImageLayout::eShaderReadOnlyOptimal)
    {
        barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
        barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

        source_stage = vk::PipelineStageFlagBits::eTransfer;
        destination_stage = vk::PipelineStageFlagBits::eFragmentShader;
    }
    else
    {
        throw std::invalid_argument("unsupported layout transition!");
    }

    command_buffer.pipelineBarrier(
            source_stage, destination_stage,
            vk::DependencyFlags(),
            nullptr,
            nullptr,
            barrier
    );

    end_single_time_commands(command_buffer);
}

void Renderer::insert_image_memory_barrier(vk::CommandBuffer cmd_buffer, vk::Image image, vk::AccessFlags srcAccessMask,
                                           vk::AccessFlags dstAccessMask, vk::ImageLayout oldImageLayout,
                                           vk::ImageLayout newImageLayout, vk::PipelineStageFlags srcStageMask,
                                           vk::PipelineStageFlags dstStageMask,
                                           vk::ImageSubresourceRange subresourceRange)
{
    vk::ImageMemoryBarrier image_memory_barrier{};
    image_memory_barrier.sType = vk::StructureType::eImageMemoryBarrier;
    image_memory_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    image_memory_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    image_memory_barrier.srcAccessMask = srcAccessMask;
    image_memory_barrier.dstAccessMask = dstAccessMask;
    image_memory_barrier.oldLayout = oldImageLayout;
    image_memory_barrier.newLayout = newImageLayout;
    image_memory_barrier.image = image;
    image_memory_barrier.subresourceRange = subresourceRange;

    cmd_buffer.pipelineBarrier(srcStageMask, dstStageMask, vk::DependencyFlags(), 0, nullptr, 0, nullptr, 1, &image_memory_barrier);
}

bool Renderer::check_validation_layer_support() const
{
    uint32_t layer_count;
    if(vk::enumerateInstanceLayerProperties(&layer_count, nullptr) != vk::Result::eSuccess) fatal("Could not enumerate layer properties!")

    std::vector<vk::LayerProperties> available_layers(layer_count);
    if(vk::enumerateInstanceLayerProperties(&layer_count, available_layers.data()) != vk::Result::eSuccess) fatal("Could not enumerate layer properties!")

    for(const char* layer_name : m_validation_layers)
    {
        bool layer_found = false;

        for(const auto& layer_properties : available_layers)
        {
            if(strcmp(layer_name, layer_properties.layerName) == 0)
            {
                layer_found = true;
                break;
            }
        }

        if(!layer_found)
        {
            return false;
        }
    }

    return true;
}

std::vector<const char*> Renderer::get_required_extensions()
{
    uint32_t extension_count = 0;
    const char** glfw_extensions;
    glfw_extensions = glfwGetRequiredInstanceExtensions(&extension_count);

    std::vector<const char*> extensions(glfw_extensions, glfw_extensions + extension_count);

#ifdef DEBUG
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif

    return extensions;
}