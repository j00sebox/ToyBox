#define VMA_IMPLEMENTATION
#include "pch.h"
#include "Renderer.hpp"
#include "Log.h"

#include "util/DeviceHelper.hpp"

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

Renderer::Renderer(GLFWwindow* window) :
    m_window(window),
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
}

Renderer::~Renderer()
{
    for(i32 i = 0; i < k_max_frames_in_flight; ++i)
    {
        // uniform buffers
        destroy_buffer(m_camera_buffers[i]);
    }
    destroy_sampler(m_default_sampler);
    m_logical_device.destroyDescriptorPool(m_descriptor_pool, nullptr);
    m_logical_device.destroyDescriptorSetLayout(m_descriptor_set_layout, nullptr);
    m_logical_device.destroyDescriptorSetLayout(m_camera_data_layout, nullptr);
    m_logical_device.destroyDescriptorSetLayout(m_texture_set_layout, nullptr);
    vmaDestroyAllocator(m_allocator);
    cleanup_swapchain();
    m_logical_device.destroyRenderPass(m_render_pass, nullptr);
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

    // TODO: implement these functions
//    transition_image_layout(texture->vk_image, texture_creation.format, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);
//
//    copy_buffer_to_image(staging_buffer->vk_buffer, texture->vk_image, static_cast<u32>(width), static_cast<u32>(height));
//
//    transition_image_layout(texture->vk_image, texture_creation.format, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);

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
    // FIXME: magic numbers
    vk::DescriptorSetLayoutBinding image_sampler_binding{};
    image_sampler_binding.descriptorType = vk::DescriptorType::eCombinedImageSampler;
    image_sampler_binding.descriptorCount = k_max_bindless_resources;
    image_sampler_binding.binding = 10; // binding for all bindless textures (paradox)
    image_sampler_binding.stageFlags = vk::ShaderStageFlagBits::eFragment;

    vk::DescriptorSetLayoutBinding storage_image_binding{};
    storage_image_binding.descriptorType = vk::DescriptorType::eStorageImage;
    storage_image_binding.descriptorCount = k_max_bindless_resources;
    storage_image_binding.binding = 11;

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

    m_default_sampler = create_sampler({
        .min_filter = vk::Filter::eLinear,
        .mag_filter = vk::Filter::eLinear,
        .u_mode = vk::SamplerAddressMode::eRepeat,
        .v_mode = vk::SamplerAddressMode::eRepeat,
        .w_mode = vk::SamplerAddressMode::eRepeat
    });
}

void Renderer::init_graphics_pipeline()
{

}

void Renderer::init_command_pools()
{

}

void Renderer::init_command_buffers()
{

}

void Renderer::init_depth_resources()
{

}

void Renderer::init_framebuffers()
{

}

void Renderer::init_sync_objects()
{

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

size_t Renderer::pad_uniform_buffer(size_t original_size) const
{
    size_t alignment = m_device_properties.limits.minUniformBufferOffsetAlignment;
    size_t aligned_size = (alignment + original_size - 1) & ~(alignment - 1);
    return aligned_size;
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