#include "pch.h"
#include "DeviceHelper.hpp"
#include "Log.hpp"

namespace devh
{
    QueueFamilies find_queue_families(vk::PhysicalDevice device, vk::SurfaceKHR surface)
    {
        QueueFamilies indices;
        u32 queue_family_count = 0;
        device.getQueueFamilyProperties(&queue_family_count, nullptr);

        if (queue_family_count == 0) return indices;

        std::vector<vk::QueueFamilyProperties> queue_families(queue_family_count);
        device.getQueueFamilyProperties(&queue_family_count, queue_families.data());

        int i = 0;
        for (const auto &queue_family: queue_families)
        {
            if (queue_family.queueFlags & vk::QueueFlagBits::eGraphics)
            {
                indices.graphics_family = i;
            }
            else if(queue_family.queueFlags & vk::QueueFlagBits::eTransfer)
            {
                indices.transfer_family = i;
            }

            vk::Bool32 present_support = false;
            if(device.getSurfaceSupportKHR(i, surface, &present_support) != vk::Result::eSuccess)
            {
                throw std::runtime_error("Could not retrieve surface support details !");
            }

            if (present_support)
            {
                indices.present_family = i;
            }

            if (indices.is_complete()) break;

            ++i;
        }

        return indices;
    }

    bool check_device_extension_support(vk::PhysicalDevice device, const std::vector<const char*>& required_extensions)
    {
        u32 extension_count = 0;
        if(device.enumerateDeviceExtensionProperties(nullptr, &extension_count, nullptr) != vk::Result::eSuccess) {}

        std::vector<vk::ExtensionProperties> available_extensions(extension_count);
        if(device.enumerateDeviceExtensionProperties(nullptr, &extension_count, available_extensions.data()) != vk::Result::eSuccess) {}

        std::set<std::string> extensions(required_extensions.begin(), required_extensions.end());
        for(const auto& extension : available_extensions)
        {
            extensions.erase(extension.extensionName);
        }

        return extensions.empty();
    }

    bool is_device_suitable(vk::PhysicalDevice device, const std::vector<const char*>& required_extensions)
    {
        vk::PhysicalDeviceDescriptorIndexingFeatures indexing_features;
        indexing_features.sType = vk::StructureType::ePhysicalDeviceDescriptorIndexingFeaturesEXT;

        vk::PhysicalDeviceFeatures2 physical_device_features2;
        physical_device_features2.sType = vk::StructureType::ePhysicalDeviceFeatures2;
        physical_device_features2.pNext = &indexing_features;
        device.getFeatures2(&physical_device_features2);

        return indexing_features.descriptorBindingPartiallyBound && indexing_features.runtimeDescriptorArray && check_device_extension_support(device, required_extensions);
    }

    u32 rate_device_suitability(vk::PhysicalDevice device)
    {
        u32 score = 0;
        vk::PhysicalDeviceProperties device_properties;
        vk::PhysicalDeviceFeatures device_features;

        device.getProperties(&device_properties);
        device.getFeatures(&device_features);

        if (device_properties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu)
        {
            score += 1000;
        }

        // max possible size of textures
        score += device_properties.limits.maxImageDimension2D;

        // no geometry shader, no deal Howie
        if (!device_features.geometryShader)
        {
            return 0;
        }

        return score;
    }

    vk::PhysicalDevice pick_physical_device(const vk::Instance& instance, const std::vector<const char*>& required_extensions)
    {
        u32 device_count = 0;
        if(instance.enumeratePhysicalDevices(&device_count, nullptr) != vk::Result::eSuccess)
        {
            fatal("Could not enumerate physical devices!");
        }

        if (device_count == 0)
        {
            throw std::runtime_error("failed to find GPUs with Vulkan support!");
        }

        std::vector<vk::PhysicalDevice> devices(device_count);

        // keeps track of devices and their score to pick the best option
        std::multimap<u32, vk::PhysicalDevice> candidates;
        if(instance.enumeratePhysicalDevices(&device_count, devices.data()) != vk::Result::eSuccess) {}

        for (const auto& device: devices)
        {
            if (is_device_suitable(device, required_extensions))
            {
                u32 score = rate_device_suitability(device);
                candidates.insert(std::make_pair(score, device));
            }
        }

        if (candidates.rbegin()->first > 0)
        {
            return candidates.rbegin()->second;
        }
        else
        {
            fatal("Failed to find a suitable GPU!");
        }
    }

    SwapChainSupportDetails query_swap_chain_support(vk::PhysicalDevice device, vk::SurfaceKHR surface)
    {
        SwapChainSupportDetails details;

        vk::Result result = device.getSurfaceCapabilitiesKHR(surface, &details.capabilities);

        unsigned format_count = 0;
        result = device.getSurfaceFormatsKHR(surface, &format_count, nullptr);

        if (format_count != 0)
        {
            details.formats.resize(format_count);
            result = device.getSurfaceFormatsKHR(surface, &format_count, details.formats.data());
        }

        unsigned present_mode_count = 0;
        result = device.getSurfacePresentModesKHR(surface, &present_mode_count, nullptr);

        if (present_mode_count != 0)
        {
            details.present_modes.resize(present_mode_count);
            result = device.getSurfacePresentModesKHR(surface, &present_mode_count, details.present_modes.data());
        }

        return details;
    }

    vk::SurfaceFormatKHR choose_swap_surface_format(const std::vector<vk::SurfaceFormatKHR>& available_formats)
    {
        for(const auto& available_format : available_formats)
        {
            if(available_format.format == vk::Format::eB8G8R8A8Srgb && available_format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
            {
                return available_format;
            }
        }

        return available_formats[0];
    }

    vk::PresentModeKHR choose_swap_present_mode(const std::vector<vk::PresentModeKHR>& available_present_modes)
    {
        for(const auto& present_mode : available_present_modes)
        {
            // triple buffering
            if(present_mode == vk::PresentModeKHR::eMailbox)
            {
                return present_mode;
            }
        }

        // most similar to regular v-sync
        return vk::PresentModeKHR::eFifo;
    }

    // resolution of the swap chain images
    vk::Extent2D choose_swap_extent(const vk::SurfaceCapabilitiesKHR& capabilities, vk::Extent2D extent)
    {
        if(capabilities.currentExtent.width != std::numeric_limits<unsigned>::max())
        {
            return capabilities.currentExtent;
        }
        else
        {
            // bound extent within limits set by the implementation
            extent.width = std::clamp(extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
            extent.height = std::clamp(extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

            return extent;
        }
    }

    u32 find_memory_type(u32 type_filter, vk::MemoryPropertyFlags properties, vk::PhysicalDevice physical_device)
    {
        vk::PhysicalDeviceMemoryProperties memory_properties;
        physical_device.getMemoryProperties(&memory_properties);

        for(u32 i = 0; i < memory_properties.memoryTypeCount; ++i)
        {
            if((type_filter & (1 << i)) && (memory_properties.memoryTypes[i].propertyFlags & properties) == properties)
            {
                return i;
            }
        }

        fatal("Could not find suitable memory type!");
    }
}
