#define VMA_IMPLEMENTATION
#include "pch.h"
#include "Renderer.hpp"
#include "Vertex.hpp"
#include "Log.h"

#include "util/DeviceHelper.hpp"
#include "util/FileOperations.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <glm/glm.hpp>

#define check(result) { if(result != vk::Result::eSuccess) { fatal("Error in: %s", __FILE__); } }

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
    m_texture_pool(&m_pool_allocator, 10, sizeof(Texture)),
    m_sampler_pool(&m_pool_allocator, 10, sizeof(Sampler)),
    m_descriptor_set_pool(&m_pool_allocator, 10, sizeof(DescriptorSet))
{
    init_instance();
    init_surface();
    init_device();
    init_swapchain();
    init_render_pass();
    init_descriptor_pools();
    init_descriptor_sets();
    init_graphics_pipeline();
    init_command_pools();
    init_command_buffers();
    init_depth_resources();
    init_framebuffers();
    init_sync_objects();
    init_imgui();

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
}

Renderer::~Renderer()
{
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
    m_logical_device.destroyDescriptorSetLayout(m_descriptor_set_layout, nullptr);
    m_logical_device.destroyDescriptorSetLayout(m_camera_data_layout, nullptr);
    m_logical_device.destroyDescriptorSetLayout(m_texture_set_layout, nullptr);
    m_logical_device.destroyCommandPool(m_main_command_pool);
    m_logical_device.destroyCommandPool(m_extra_command_pool);
    for(auto& command_pool : m_command_pools)
    {
        m_logical_device.destroyCommandPool(command_pool, nullptr);
    }
    vmaDestroyAllocator(m_allocator);
    cleanup_swapchain();
    m_logical_device.destroyRenderPass(m_render_pass, nullptr);
    m_logical_device.destroyPipelineLayout(m_pipeline_layout, nullptr);
    m_logical_device.destroyPipeline(m_graphics_pipeline, nullptr);
    m_logical_device.destroy();
    m_instance.destroySurfaceKHR(m_surface, nullptr);
#ifdef DEBUG
    m_instance.destroyDebugUtilsMessengerEXT(m_debug_messenger, nullptr, m_dispatch_loader);
#endif
    m_instance.destroy();
}

void Renderer::render(Scene* scene)
{
    
}

void Renderer::begin_frame()
{

}

void Renderer::end_frame()
{

}

BufferHandle Renderer::create_buffer(const BufferCreationInfo& buffer_creation)
{
    BufferHandle handle = (BufferHandle)m_buffer_pool.acquire();
    auto* buffer = static_cast<Buffer*>(m_buffer_pool.access(handle));

    buffer->size = buffer_creation.size;

    vk::BufferCreateInfo buffer_info{};
    buffer_info.sType = vk::StructureType::eBufferCreateInfo;
    buffer_info.size = buffer_creation.size;
    buffer_info.usage = buffer_creation.usage;
    buffer_info.sharingMode = vk::SharingMode::eExclusive;

    VmaAllocationCreateInfo memory_info{};
    memory_info.flags = VMA_ALLOCATION_CREATE_STRATEGY_BEST_FIT_BIT;
    memory_info.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

    if(buffer_creation.persistent)
    {
        memory_info.flags |= VMA_ALLOCATION_CREATE_MAPPED_BIT;
    }

    VmaAllocationInfo allocation_info{};
    vmaCreateBuffer(m_allocator, reinterpret_cast<const VkBufferCreateInfo*>(&buffer_info), &memory_info,
                    &buffer->vk_buffer, &buffer->vma_allocation, &allocation_info);

    if(buffer_creation.data)
    {
        void* data;
        vmaMapMemory(m_allocator, buffer->vma_allocation, &data);
        memcpy(data, buffer_creation.data, (size_t)buffer_creation.size);
        vmaUnmapMemory(m_allocator, buffer->vma_allocation);
    }

    if(buffer_creation.persistent)
    {
        buffer->mapped_data = static_cast<u8*>(allocation_info.pMappedData);
    }

    return handle;
}

TextureHandle Renderer::create_texture(const TextureCreationInfo& texture_creation)
{
    if(m_texture_map.contains(texture_creation.image_src))
    {
        return m_texture_map[texture_creation.image_src];
    }

    TextureHandle handle = (TextureHandle)m_texture_pool.acquire();
    auto* texture = static_cast<Texture*>(m_texture_pool.access(handle));

    stbi_set_flip_vertically_on_load(1);

    int width, height, channels;
    stbi_uc* pixels = stbi_load(texture_creation.image_src, &width, &height, &channels, STBI_rgb_alpha);

    vk::DeviceSize image_size = width * height * 4;

    texture->width = width;
    texture->height = height;
    texture->name = texture_creation.image_src;

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
    image_create_info.format = texture_creation.format;
    image_create_info.tiling = vk::ImageTiling::eOptimal;
    image_create_info.initialLayout = vk::ImageLayout::eUndefined;
    image_create_info.usage = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled;
    image_create_info.sharingMode = vk::SharingMode::eExclusive;
    image_create_info.samples = vk::SampleCountFlagBits::e1;

    VmaAllocationCreateInfo memory_info{};
    memory_info.usage = VMA_MEMORY_USAGE_GPU_ONLY;

    vmaCreateImage(m_allocator, reinterpret_cast<const VkImageCreateInfo*>(&image_create_info), &memory_info, &texture->vk_image, &texture->vma_allocation, nullptr);

    transition_image_layout(texture->vk_image, texture_creation.format, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);

    copy_buffer_to_image(staging_buffer->vk_buffer, texture->vk_image, static_cast<u32>(width), static_cast<u32>(height));

    transition_image_layout(texture->vk_image, texture_creation.format, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);

    texture->vk_image_view = create_image_view(texture->vk_image, texture_creation.format, vk::ImageAspectFlagBits::eColor);

    destroy_buffer(staging_handle);

    m_texture_map[texture_creation.image_src] = handle;

    return handle;
}

SamplerHandle Renderer::create_sampler(const SamplerCreationInfo& sampler_creation)
{
    SamplerHandle sampler_handle = (SamplerHandle)m_sampler_pool.acquire();
    auto* sampler = static_cast<Sampler*>(m_sampler_pool.access(sampler_handle));

    vk::SamplerCreateInfo sampler_info{};
    sampler_info.sType = vk::StructureType::eSamplerCreateInfo;
    sampler_info.magFilter = sampler_creation.mag_filter;
    sampler_info.minFilter = sampler_creation.min_filter;

    sampler_info.addressModeU = sampler_creation.u_mode;
    sampler_info.addressModeV = sampler_creation.v_mode;
    sampler_info.addressModeW = sampler_creation.w_mode;

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

DescriptorSetHandle Renderer::create_descriptor_set(const DescriptorSetCreationInfo& descriptor_set_creation)
{
    DescriptorSetHandle descriptor_set_handle = (DescriptorSetHandle)m_descriptor_set_pool.acquire();
    auto* descriptor_set = static_cast<DescriptorSet*>(m_descriptor_set_pool.access(descriptor_set_handle));

    vk::DescriptorSetAllocateInfo allocInfo = {};
    allocInfo.sType = vk::StructureType::eDescriptorSetAllocateInfo;
    allocInfo.descriptorPool = m_descriptor_pool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &descriptor_set_creation.layout;

    check(m_logical_device.allocateDescriptorSets(&allocInfo, &descriptor_set->vk_descriptor_set));

    vk::WriteDescriptorSet descriptor_writes[descriptor_set_creation.num_resources];

    for(int i = 0; i < descriptor_set_creation.num_resources; ++i)
    {
        switch(descriptor_set_creation.types[i])
        {
            case vk::DescriptorType::eUniformBuffer:
            {
                auto* buffer = static_cast<Buffer*>(m_buffer_pool.access(descriptor_set_creation.resource_handles[i]));

                vk::DescriptorBufferInfo descriptor_info{};
                descriptor_info.buffer = buffer->vk_buffer;
                descriptor_info.offset = 0;
                descriptor_info.range = buffer->size;

                descriptor_writes[i].sType = vk::StructureType::eWriteDescriptorSet;
                descriptor_writes[i].dstSet = descriptor_set->vk_descriptor_set;
                descriptor_writes[i].dstBinding = descriptor_set_creation.bindings[i]; // index binding
                descriptor_writes[i].dstArrayElement = 0;

                descriptor_writes[i].descriptorType = vk::DescriptorType::eUniformBuffer;
                descriptor_writes[i].descriptorCount = 1; // how many array elements to update

                descriptor_writes[i].pBufferInfo = &descriptor_info;

                // used for descriptors that reference image data
                descriptor_writes[i].pImageInfo = nullptr;
                descriptor_writes[i].pTexelBufferView = nullptr;

                m_logical_device.updateDescriptorSets(1, &descriptor_writes[i], 0, nullptr);

                break;
            }
            case vk::DescriptorType::eCombinedImageSampler:
            {
                auto* texture = static_cast<Texture*>(m_texture_pool.access(descriptor_set_creation.resource_handles[i]));
                auto* sampler = static_cast<Sampler*>(m_sampler_pool.access(descriptor_set_creation.sampler_handles[i]));

                vk::DescriptorImageInfo descriptor_info{};
                descriptor_info.imageView = texture->vk_image_view;
                descriptor_info.sampler = sampler->vk_sampler;
                descriptor_info.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

                descriptor_writes[i].sType = vk::StructureType::eWriteDescriptorSet;
                descriptor_writes[i].dstSet = descriptor_set->vk_descriptor_set; // descriptor set to update
                descriptor_writes[i].dstBinding = descriptor_set_creation.bindings[i];
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
    alloc_info.memoryTypeIndex = DeviceHelper::find_memory_type(memory_requirements.memoryTypeBits, properties, m_physical_device);

    check(m_logical_device.allocateMemory(&alloc_info, nullptr, &image_memory));
    m_logical_device.bindImageMemory(image, image_memory, 0);
}

vk::ImageView Renderer::create_image_view(const vk::Image& image, vk::Format format, vk::ImageAspectFlags image_aspect)
{
    vk::ImageViewCreateInfo imageview_create_info{};
    imageview_create_info.sType = vk::StructureType::eImageViewCreateInfo;
    imageview_create_info.image = image;
    imageview_create_info.viewType = vk::ImageViewType::e2D;
    imageview_create_info.format = format;
    imageview_create_info.components.r = vk::ComponentSwizzle::eIdentity;
    imageview_create_info.components.g = vk::ComponentSwizzle::eIdentity;
    imageview_create_info.components.b = vk::ComponentSwizzle::eIdentity;
    imageview_create_info.components.a = vk::ComponentSwizzle::eIdentity;
    imageview_create_info.subresourceRange.aspectMask = image_aspect;
    imageview_create_info.subresourceRange.baseMipLevel = 0;
    imageview_create_info.subresourceRange.levelCount = 1;
    imageview_create_info.subresourceRange.baseArrayLayer = 0;
    imageview_create_info.subresourceRange.layerCount = 1;

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
    m_texture_map.erase(texture->name);
}

void Renderer::destroy_sampler(SamplerHandle sampler_handle)
{
    auto* sampler = static_cast<Sampler*>(m_sampler_pool.access(sampler_handle));
    m_logical_device.destroySampler(sampler->vk_sampler, nullptr);
    m_sampler_pool.free(sampler_handle);
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

    m_physical_device = DeviceHelper::pick_physical_device(m_instance, m_required_device_extensions);
    DeviceHelper::QueueFamilies indices = DeviceHelper::find_queue_families(m_physical_device, m_surface);

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
    DeviceHelper::SwapChainSupportDetails swap_chain_support = DeviceHelper::query_swap_chain_support(m_physical_device, m_surface);

    i32 width, height;
    glfwGetFramebufferSize(m_window, &width, &height);
    vk::Extent2D extent = { static_cast<u32>(width), static_cast<u32>(height) };

    vk::SurfaceFormatKHR surface_format = DeviceHelper::choose_swap_surface_format(swap_chain_support.formats);
    vk::PresentModeKHR present_mode = DeviceHelper::choose_swap_present_mode(swap_chain_support.present_modes);
    vk::Extent2D actual_extent = DeviceHelper::choose_swap_extent(swap_chain_support.capabilities, extent);

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

    DeviceHelper::QueueFamilies indices = DeviceHelper::find_queue_families(m_physical_device, m_surface);
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
        m_swapchain_image_views[i] = create_image_view(m_swapchain_images[i], m_swapchain_image_format, vk::ImageAspectFlagBits::eColor);
    }
}

void Renderer::init_render_pass()
{
    vk::AttachmentDescription colour_attachment{};
    colour_attachment.format = m_swapchain_image_format;
    colour_attachment.samples = vk::SampleCountFlagBits::e1;
    colour_attachment.loadOp = vk::AttachmentLoadOp::eClear;
    colour_attachment.storeOp = vk::AttachmentStoreOp::eStore;
    colour_attachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
    colour_attachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
    colour_attachment.initialLayout = vk::ImageLayout::eUndefined;
    colour_attachment.finalLayout = vk::ImageLayout::ePresentSrcKHR;

    vk::AttachmentDescription depth_attachment{};
    depth_attachment.format = vk::Format::eD32Sfloat;
    depth_attachment.samples = vk::SampleCountFlagBits::e1;
    depth_attachment.loadOp = vk::AttachmentLoadOp::eClear;
    depth_attachment.storeOp = vk::AttachmentStoreOp::eDontCare;
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
    dependency.srcAccessMask = vk::AccessFlagBits::eNone;
    dependency.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests;
    dependency.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentWrite;

    std::array<vk::AttachmentDescription, 2> attachments = {colour_attachment, depth_attachment};
    vk::RenderPassCreateInfo render_pass_info{};
    render_pass_info.sType = vk::StructureType::eRenderPassCreateInfo;
    render_pass_info.attachmentCount = static_cast<unsigned>(attachments.size());
    render_pass_info.pAttachments = attachments.data();
    render_pass_info.subpassCount = 1;
    render_pass_info.pSubpasses = &subpass;
    render_pass_info.dependencyCount = 1;
    render_pass_info.pDependencies = &dependency;

    check(m_logical_device.createRenderPass(&render_pass_info, nullptr, &m_render_pass));
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
}

void Renderer::init_descriptor_sets()
{
    vk::DescriptorSetLayoutBinding camera_data_layout_binding{};
    camera_data_layout_binding.binding = 0; // binding in the shader
    camera_data_layout_binding.descriptorType = vk::DescriptorType::eUniformBuffer;
    camera_data_layout_binding.descriptorCount = 1;
    camera_data_layout_binding.stageFlags = vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment;
    camera_data_layout_binding.pImmutableSamplers = nullptr; // only relevant for image sampling descriptors

    vk::DescriptorSetLayoutBinding lighting_data_layout_binding{};
    lighting_data_layout_binding.binding = 1;
    lighting_data_layout_binding.descriptorType = vk::DescriptorType::eUniformBuffer;
    lighting_data_layout_binding.descriptorCount = 1;
    lighting_data_layout_binding.stageFlags = vk::ShaderStageFlagBits::eFragment;
    lighting_data_layout_binding.pImmutableSamplers = nullptr;

    vk::DescriptorSetLayoutBinding bindings[] = { camera_data_layout_binding, lighting_data_layout_binding };

    vk::DescriptorSetLayoutCreateInfo uniform_layout_info{};
    uniform_layout_info.sType = vk::StructureType::eDescriptorSetLayoutCreateInfo;
    uniform_layout_info.bindingCount = 2;
    uniform_layout_info.pBindings = bindings;

    check(m_logical_device.createDescriptorSetLayout(&uniform_layout_info, nullptr, &m_camera_data_layout))

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
            .layout = m_camera_data_layout,
            .num_resources = 1,
        });
    }

    // bindless texture set layout
    vk::DescriptorSetLayoutBinding image_sampler_binding{};
    image_sampler_binding.descriptorType = vk::DescriptorType::eCombinedImageSampler;
    image_sampler_binding.descriptorCount = k_max_bindless_resources;
    image_sampler_binding.binding = k_bindless_texture_binding; // binding for all bindless textures (paradox)
    image_sampler_binding.stageFlags = vk::ShaderStageFlagBits::eFragment;

    vk::DescriptorSetLayoutBinding storage_image_binding{};
    storage_image_binding.descriptorType = vk::DescriptorType::eStorageImage;
    storage_image_binding.descriptorCount = k_max_bindless_resources;
    storage_image_binding.binding = k_bindless_image_binding;

    vk::DescriptorSetLayoutBinding bindless_bindings[] = { image_sampler_binding, storage_image_binding };

    vk::DescriptorSetLayoutCreateInfo bindless_layout_create_info{};
    bindless_layout_create_info.sType = vk::StructureType::eDescriptorSetLayoutCreateInfo;
    bindless_layout_create_info.bindingCount = 2;
    bindless_layout_create_info.pBindings = bindless_bindings;
    bindless_layout_create_info.flags = vk::DescriptorSetLayoutCreateFlagBits::eUpdateAfterBindPoolEXT;

    vk::DescriptorBindingFlags bindless_flags = vk::DescriptorBindingFlagBits::ePartiallyBound | vk::DescriptorBindingFlagBits::eUpdateAfterBind;

    vk::DescriptorBindingFlags binding_flags[] = { bindless_flags, bindless_flags };
    vk::DescriptorSetLayoutBindingFlagsCreateInfoEXT extended_info{};
    extended_info.sType = vk::StructureType::eDescriptorSetLayoutBindingFlagsCreateInfoEXT;
    extended_info.bindingCount = 2;
    extended_info.pBindingFlags = binding_flags;

    bindless_layout_create_info.pNext = &extended_info;

    check(m_logical_device.createDescriptorSetLayout(&bindless_layout_create_info, nullptr, &m_texture_set_layout));

    m_texture_set = (DescriptorSetHandle)m_descriptor_set_pool.acquire();
    auto* descriptor_set = static_cast<DescriptorSet*>(m_descriptor_set_pool.access(m_texture_set));

    vk::DescriptorSetAllocateInfo allocInfo = {};
    allocInfo.sType = vk::StructureType::eDescriptorSetAllocateInfo;
    allocInfo.descriptorPool = m_descriptor_pool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &m_texture_set_layout;

    check(m_logical_device.allocateDescriptorSets(&allocInfo, &descriptor_set->vk_descriptor_set));
}

void Renderer::init_graphics_pipeline()
{
    bool cache_exists = std::filesystem::exists("pipeline_cache.bin");
    bool cache_header_valid = false;

    vk::PipelineCache pipeline_cache = nullptr;

    vk::PipelineCacheCreateInfo pipeline_cache_info{};
    pipeline_cache_info.sType = vk::StructureType::ePipelineCacheCreateInfo;

    if(cache_exists)
    {
        std::vector<u8> pipeline_cache_data = fileop::read_binary_file("pipeline_cache.bin");

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

    auto vert_shader_code = fileop::read_binary_file("../assets/shaders/vert.spv");
    auto frag_shader_code = fileop::read_binary_file("../assets/shaders/frag.spv");

    vk::ShaderModule vert_shader_module = create_shader_module(vert_shader_code);
    vk::ShaderModule frag_shader_module = create_shader_module(frag_shader_code);

    vk::PipelineShaderStageCreateInfo vert_shader_stage_info{};
    vert_shader_stage_info.sType = vk::StructureType::ePipelineShaderStageCreateInfo;
    vert_shader_stage_info.stage = vk::ShaderStageFlagBits::eVertex;
    vert_shader_stage_info.module = vert_shader_module;
    vert_shader_stage_info.pName = "main";

    vk::PipelineShaderStageCreateInfo frag_shader_stage_info{};
    frag_shader_stage_info.sType = vk::StructureType::ePipelineShaderStageCreateInfo;
    frag_shader_stage_info.stage = vk::ShaderStageFlagBits::eFragment;
    frag_shader_stage_info.module = frag_shader_module;
    frag_shader_stage_info.pName = "main";

    vk::PipelineShaderStageCreateInfo shader_stages[] = {vert_shader_stage_info, frag_shader_stage_info};

    std::vector<vk::DynamicState> dynamic_states =
    {
            vk::DynamicState::eViewport,
            vk::DynamicState::eScissor
    };

    // the configuration of these values will be ignored, so they can be changed at runtime
    vk::PipelineDynamicStateCreateInfo dynamic_state{};
    dynamic_state.sType = vk::StructureType::ePipelineDynamicStateCreateInfo;
    dynamic_state.dynamicStateCount = static_cast<unsigned>(dynamic_states.size());
    dynamic_state.pDynamicStates = dynamic_states.data();

    auto binding_description = Vertex::get_binding_description();
    auto attribute_descriptions = Vertex::get_attribute_description();

    vk::PipelineVertexInputStateCreateInfo vertex_input_info{};
    vertex_input_info.sType = vk::StructureType::ePipelineVertexInputStateCreateInfo;
    vertex_input_info.vertexBindingDescriptionCount = 1;
    vertex_input_info.pVertexBindingDescriptions = &binding_description;
    vertex_input_info.vertexAttributeDescriptionCount = static_cast<unsigned>(attribute_descriptions.size());
    vertex_input_info.pVertexAttributeDescriptions = attribute_descriptions.data();

    vk::PipelineInputAssemblyStateCreateInfo input_assembly{};
    input_assembly.sType = vk::StructureType::ePipelineInputAssemblyStateCreateInfo;
    input_assembly.topology = vk::PrimitiveTopology::eTriangleList;
    input_assembly.primitiveRestartEnable = false;

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

    vk::PipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = vk::StructureType::ePipelineRasterizationStateCreateInfo;
    rasterizer.depthClampEnable = false;
    rasterizer.rasterizerDiscardEnable = false;
    rasterizer.polygonMode = vk::PolygonMode::eFill;
    rasterizer.lineWidth = 1.f;
    rasterizer.cullMode = vk::CullModeFlagBits::eBack;
    rasterizer.frontFace = vk::FrontFace::eCounterClockwise;
    rasterizer.depthBiasEnable = false;
    rasterizer.depthBiasConstantFactor = 0.f;
    rasterizer.depthBiasClamp = 0.f;
    rasterizer.depthBiasSlopeFactor = 0.f;

    vk::PipelineMultisampleStateCreateInfo multi_sampling{};
    multi_sampling.sType = vk::StructureType::ePipelineMultisampleStateCreateInfo;
    multi_sampling.sampleShadingEnable = false;
    multi_sampling.rasterizationSamples = vk::SampleCountFlagBits::e1;

//        multi_sampling.minSampleShading = 1.f;
//        multi_sampling.pSampleMask = nullptr;
//        multi_sampling.alphaToCoverageEnable = false;
//        multi_sampling.alphaToOneEnable = false;

    // if using depth or stencil buffer then they need to be configured
    // vk::PipelineDepthStencilStateCreateInfo

    // colour blending
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

    vk::PipelineColorBlendStateCreateInfo colour_blending{};
    colour_blending.sType = vk::StructureType::ePipelineColorBlendStateCreateInfo;
    colour_blending.logicOpEnable = false;
    colour_blending.logicOp = vk::LogicOp::eCopy; // optional
    colour_blending.attachmentCount = 1;
    colour_blending.pAttachments = &colour_blend_attachment;

    vk::PipelineDepthStencilStateCreateInfo depth_stencil{};
    depth_stencil.sType = vk::StructureType::ePipelineDepthStencilStateCreateInfo;
    depth_stencil.depthTestEnable = true;
    depth_stencil.depthWriteEnable = true;
    depth_stencil.depthCompareOp = vk::CompareOp::eLess;
    depth_stencil.depthBoundsTestEnable = false;
    depth_stencil.minDepthBounds = 0.f;
    depth_stencil.maxDepthBounds = 1.f;
    depth_stencil.stencilTestEnable = false;

    vk::PipelineLayoutCreateInfo pipeline_layout_info{};
    pipeline_layout_info.sType = vk::StructureType::ePipelineLayoutCreateInfo;

    // need to specify the descriptor set layout here
    pipeline_layout_info.setLayoutCount = 2;
    vk::DescriptorSetLayout layouts[] = {m_camera_data_layout, m_texture_set_layout};
    pipeline_layout_info.pSetLayouts = layouts;

    // need to tell the pipeline that there will be a push constant coming in
    vk::PushConstantRange model_push_constant_info{};
    model_push_constant_info.offset = 0;
    model_push_constant_info.size = sizeof(glm::mat4);
    model_push_constant_info.stageFlags = vk::ShaderStageFlagBits::eVertex;

    vk::PushConstantRange texture_push_constant_info{};
    texture_push_constant_info.offset = 64;
    texture_push_constant_info.size = sizeof(glm::uvec4);
    texture_push_constant_info.stageFlags = vk::ShaderStageFlagBits::eFragment;

    vk::PushConstantRange push_constant_ranges[] = { model_push_constant_info, texture_push_constant_info };
    pipeline_layout_info.pushConstantRangeCount = 2;
    pipeline_layout_info.pPushConstantRanges = push_constant_ranges;

    check(m_logical_device.createPipelineLayout(&pipeline_layout_info, nullptr, &m_pipeline_layout));

    vk::GraphicsPipelineCreateInfo pipeline_create_info{};
    pipeline_create_info.sType = vk::StructureType::eGraphicsPipelineCreateInfo;
    pipeline_create_info.stageCount = 2;
    pipeline_create_info.pStages = shader_stages;
    pipeline_create_info.pVertexInputState = &vertex_input_info;
    pipeline_create_info.pInputAssemblyState = &input_assembly;
    pipeline_create_info.pViewportState = &viewport_state;
    pipeline_create_info.pRasterizationState = &rasterizer;
    pipeline_create_info.pMultisampleState = &multi_sampling;
    pipeline_create_info.pDepthStencilState = nullptr; // optional
    pipeline_create_info.pColorBlendState = &colour_blending;
    pipeline_create_info.pDynamicState = &dynamic_state;
    pipeline_create_info.layout = m_pipeline_layout;
    pipeline_create_info.renderPass = m_render_pass;
    pipeline_create_info.subpass = 0;
    pipeline_create_info.pDepthStencilState = &depth_stencil;
    pipeline_create_info.basePipelineHandle = nullptr;
    pipeline_create_info.basePipelineIndex = -1;

    check(m_logical_device.createGraphicsPipelines(pipeline_cache, 1, &pipeline_create_info, nullptr, &m_graphics_pipeline));

    if(!cache_exists || !cache_header_valid)
    {
        size_t cache_data_size = 0;

        check(m_logical_device.getPipelineCacheData(pipeline_cache, &cache_data_size, nullptr));

        void* cache_data = malloc(cache_data_size);

        check(m_logical_device.getPipelineCacheData(pipeline_cache, &cache_data_size, cache_data));

        fileop::write_binary_file(cache_data, cache_data_size, "pipeline_cache.bin");
        free(cache_data);
    }

    m_logical_device.destroyPipelineCache(pipeline_cache, nullptr);
    m_logical_device.destroyShaderModule(vert_shader_module, nullptr);
    m_logical_device.destroyShaderModule(frag_shader_module, nullptr);
}

void Renderer::init_command_pools()
{
    DeviceHelper::QueueFamilies queue_family_indices = DeviceHelper::find_queue_families(m_physical_device, m_surface);

    vk::CommandPoolCreateInfo pool_create_info{};
    pool_create_info.sType = vk::StructureType::eCommandPoolCreateInfo;
    pool_create_info.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
    pool_create_info.queueFamilyIndex = queue_family_indices.graphics_family.value();

    check(m_logical_device.createCommandPool(&pool_create_info, nullptr, &m_main_command_pool));
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
            check(m_logical_device.allocateCommandBuffers(&secondary_alloc_info, &m_command_buffers[i + command_buffer_index].vk_command_buffer));
        }

        command_buffer_index += k_max_frames_in_flight;
    }

    vk::CommandBufferAllocateInfo primary_alloc_info{};
    primary_alloc_info.sType = vk::StructureType::eCommandBufferAllocateInfo;
    primary_alloc_info.commandPool = m_main_command_pool;
    primary_alloc_info.level = vk::CommandBufferLevel::ePrimary;
    primary_alloc_info.commandBufferCount = 1;

    vk::CommandBufferAllocateInfo extra_alloc_info{};
    extra_alloc_info.sType = vk::StructureType::eCommandBufferAllocateInfo;
    extra_alloc_info.commandPool = m_extra_command_pool;
    extra_alloc_info.level = vk::CommandBufferLevel::eSecondary;
    extra_alloc_info.commandBufferCount = 1;

    vk::CommandBufferAllocateInfo imgui_alloc_info{};
    imgui_alloc_info.sType = vk::StructureType::eCommandBufferAllocateInfo;
    imgui_alloc_info.commandPool = m_main_command_pool;
    imgui_alloc_info.level = vk::CommandBufferLevel::eSecondary;
    imgui_alloc_info.commandBufferCount = 1;

    for(u32 i = 0; i < k_max_frames_in_flight; ++i)
    {
        check(m_logical_device.allocateCommandBuffers(&primary_alloc_info, &m_primary_command_buffers[i].vk_command_buffer));
        check(m_logical_device.allocateCommandBuffers(&extra_alloc_info, &m_extra_draw_commands[i].vk_command_buffer));
        check(m_logical_device.allocateCommandBuffers(&imgui_alloc_info, &m_imgui_commands[i].vk_command_buffer));
    }
}

void Renderer::init_depth_resources()
{
    create_image(m_swapchain_extent.width, m_swapchain_extent.height,
                 vk::Format::eD32Sfloat, vk::ImageTiling::eOptimal,
                 vk::ImageUsageFlagBits::eDepthStencilAttachment,
                 vk::MemoryPropertyFlagBits::eDeviceLocal,
                 m_depth_image, m_depth_image_memory);

    m_depth_image_view = create_image_view(m_depth_image, vk::Format::eD32Sfloat, vk::ImageAspectFlagBits::eDepth);
}

void Renderer::init_framebuffers()
{
    m_swapchain_framebuffers.resize(m_swapchain_image_views.size());

    for(u32 i = 0; i < m_swapchain_image_views.size(); ++i)
    {
        std::array<vk::ImageView, 2> attachments = { m_swapchain_image_views[i], m_depth_image_view };

        vk::FramebufferCreateInfo framebuffer_create_info{};
        framebuffer_create_info.sType = vk::StructureType::eFramebufferCreateInfo;
        framebuffer_create_info.renderPass = m_render_pass;
        framebuffer_create_info.attachmentCount = static_cast<unsigned>(attachments.size());
        framebuffer_create_info.pAttachments = attachments.data();
        framebuffer_create_info.width = m_swapchain_extent.width;
        framebuffer_create_info.height = m_swapchain_extent.height;
        framebuffer_create_info.layers = 1;

        check(m_logical_device.createFramebuffer(&framebuffer_create_info, nullptr, &m_swapchain_framebuffers[i]));
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

}

vk::CommandBuffer Renderer::begin_single_time_commands()
{
    vk::CommandBufferAllocateInfo alloc_info{};
    alloc_info.sType = vk::StructureType::eCommandBufferAllocateInfo;
    alloc_info.level = vk::CommandBufferLevel::ePrimary;
    alloc_info.commandPool = m_command_pools[0];
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

    m_logical_device.freeCommandBuffers(m_command_pools[0], 1, &command_buffer);
}

void Renderer::cleanup_swapchain()
{
    for(auto framebuffer : m_swapchain_framebuffers)
    {
        m_logical_device.destroyFramebuffer(framebuffer, nullptr);
    }

    for(auto image_view : m_swapchain_image_views)
    {
        m_logical_device.destroyImageView(image_view, nullptr);
    }

    m_logical_device.destroyImage(m_depth_image, nullptr);
    m_logical_device.freeMemory(m_depth_image_memory);
    m_logical_device.destroyImageView(m_depth_image_view, nullptr);

    m_logical_device.destroySwapchainKHR(m_swapchain, nullptr);
}

void Renderer::recreate_swapchain()
{

}

size_t Renderer::pad_uniform_buffer(size_t original_size) const
{
    size_t alignment = m_device_properties.limits.minUniformBufferOffsetAlignment;
    size_t aligned_size = (alignment + original_size - 1) & ~(alignment - 1);
    return aligned_size;
}

void Renderer::copy_buffer_to_image(vk::Buffer buffer, vk::Image image, u32 width, u32 height)
{
    vk::CommandBuffer command_buffer = begin_single_time_commands();

    vk::BufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
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

void Renderer::transition_image_layout(vk::Image image, vk::Format format, vk::ImageLayout old_layout, vk::ImageLayout new_layout)
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
    barrier.subresourceRange.layerCount = 1;

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