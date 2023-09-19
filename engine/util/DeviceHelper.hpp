#pragma once
#include "Types.hpp"

#include <vulkan/vulkan.hpp>
#include <optional>

// a bunch of helper functions that can extract more information from a device
namespace devh
{
    struct QueueFamilies
    {
        std::optional<unsigned> graphics_family;
        std::optional<unsigned> present_family;
        std::optional<unsigned> transfer_family;

        [[nodiscard]] bool is_complete() const { return (graphics_family.has_value() && present_family.has_value() && transfer_family.has_value()); }
    };

    struct SwapChainSupportDetails
    {
        vk::SurfaceCapabilitiesKHR capabilities;
        std::vector<vk::SurfaceFormatKHR> formats;
        std::vector<vk::PresentModeKHR> present_modes;
    };

    vk::PhysicalDevice              pick_physical_device(const vk::Instance& instance, const std::vector<const char*>& required_extensions);
    QueueFamilies                   find_queue_families(vk::PhysicalDevice device, vk::SurfaceKHR surface);
    bool                            check_device_extension_support(vk::PhysicalDevice device, const std::vector<const char*>& required_extensions);
    bool                            is_device_suitable(vk::PhysicalDevice device, const std::vector<const char*>& required_extensions);
    u32                             rate_device_suitability(vk::PhysicalDevice device);
    SwapChainSupportDetails         query_swap_chain_support(vk::PhysicalDevice device, vk::SurfaceKHR surface);
    vk::SurfaceFormatKHR            choose_swap_surface_format(const std::vector<vk::SurfaceFormatKHR>& available_formats);
    vk::PresentModeKHR              choose_swap_present_mode(const std::vector<vk::PresentModeKHR>& available_present_modes);
    vk::Extent2D                    choose_swap_extent(const vk::SurfaceCapabilitiesKHR& capabilities, vk::Extent2D extent);
    u32                             find_memory_type(u32 type_filter, vk::MemoryPropertyFlags properties, vk::PhysicalDevice physical_device);
}
