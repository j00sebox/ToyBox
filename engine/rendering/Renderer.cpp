#define VMA_IMPLEMENTATION
#include "pch.h"
#include "Renderer.hpp"
#include "Log.h"

#include "util/DeviceHelper.hpp"

Renderer::Renderer(GLFWwindow* window) :
    m_window(window)
{
    init_instance();
    init_surface();
    init_device();
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

void Renderer::init_instance()
{
#ifdef DEBUG
    check(check_validation_layer_support());
#endif

    vk::ApplicationInfo app_info{};
    app_info.sType = vk::StructureType::eApplicationInfo;
    app_info.pApplicationName = "ToyBox";
    app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.pEngineName = "ToyBox";
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

    check(vk::createInstance(&instance_create_info, nullptr, &m_instance) == vk::Result::eSuccess);

    m_dispatch_loader = vk::DispatchLoaderDynamic(m_instance, vkGetInstanceProcAddr);

#ifdef DEBUG
    m_debug_messenger = m_instance.createDebugUtilsMessengerEXT(debug_create_info, nullptr, m_dispatch_loader);
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
    std::set<u32> unique_queue_families = {indices.graphics_family.value(), indices.present_family.value(), indices.transfer_family.value()};
    f32 queue_priority = 1.f;
    for (u32 queue_family: unique_queue_families)
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
    
    check(m_physical_device.createDevice(&device_create_info, nullptr, &m_logical_device) == vk::Result::eSuccess);

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

bool Renderer::check_validation_layer_support() const
{
    uint32_t layer_count;
    check(vk::enumerateInstanceLayerProperties(&layer_count, nullptr) == vk::Result::eSuccess);

    std::vector<vk::LayerProperties> available_layers(layer_count);
    check(vk::enumerateInstanceLayerProperties(&layer_count, available_layers.data()) == vk::Result::eSuccess);

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

std::vector<const char*> Renderer::get_required_extensions() const
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