#define VMA_IMPLEMENTATION
#include "pch.h"
#include "Renderer.hpp"
#include "Log.h"

#include "util/DeviceHelper.hpp"

#define check(result) { if(result != vk::Result::eSuccess) { fatal("Error in: %s", __FILE__); } }

Renderer::Renderer(GLFWwindow* window) :
    m_window(window)
{
    info("1");
    init_instance();
    init_surface();
    info("2");
    init_device();
    info("3");
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

    if(vk::createInstance(&instance_create_info, nullptr, &m_instance) != vk::Result::eSuccess) fatal("Could not create instance!");

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
    info("A");
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

    info("B");

    std::vector<vk::DeviceQueueCreateInfo> queue_create_infos;
    m_queue_indices = {indices.graphics_family.value(), indices.present_family.value(), indices.transfer_family.value()};
    f32 queue_priority = 1.f;
    for (u32 queue_family: m_queue_indices)
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

    info("C");

    // only using one queue per family
    m_logical_device.getQueue(indices.graphics_family.value(), 0, &m_graphics_queue);
    m_logical_device.getQueue(indices.present_family.value(), 0, &m_present_queue);
    m_logical_device.getQueue(indices.transfer_family.value(), 0, &m_transfer_queue);
    m_physical_device.getProperties(&m_device_properties);

    info("D");

    VmaAllocatorCreateInfo vma_info{};
    vma_info.vulkanApiVersion = m_device_properties.apiVersion;
    vma_info.physicalDevice = m_physical_device;
    vma_info.device = m_logical_device;
    vma_info.instance = m_instance;
    vmaCreateAllocator(&vma_info, &m_allocator);

    info("E");
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

}

void Renderer::init_descriptor_sets()
{

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