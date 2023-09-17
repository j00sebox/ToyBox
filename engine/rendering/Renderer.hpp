#pragma once
#include "Types.hpp"
//#include "scene/Scene.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vulkan/vulkan.hpp>
#include <vk_mem_alloc.h>
#include <iostream>

// FIXME
class Scene;

class Renderer
{
public:
    explicit Renderer(GLFWwindow* window);
    ~Renderer();

	void render(Scene* scene);
	void begin_frame();
	void end_frame();

    // resource creation
    vk::ImageView create_image_view(const vk::Image& image, vk::Format format, vk::ImageAspectFlags image_aspect);

private:
    GLFWwindow* m_window;

    vk::Instance m_instance;
    vk::DebugUtilsMessengerEXT m_debug_messenger;
    vk::DispatchLoaderDynamic m_dispatch_loader;
    vk::SurfaceKHR m_surface;
    vk::PhysicalDevice m_physical_device;
    vk::Device m_logical_device;
    vk::PhysicalDeviceProperties m_device_properties;
    VmaAllocator m_allocator;

    vk::SwapchainKHR m_swapchain;
    vk::Format m_swapchain_image_format;
    vk::Extent2D m_swapchain_extent;
    std::vector<vk::Image> m_swapchain_images;
    std::vector<vk::ImageView> m_swapchain_image_views;
    std::vector<vk::Framebuffer> m_swapchain_framebuffers;
    vk::RenderPass m_render_pass;
    vk::DescriptorSetLayout m_descriptor_set_layout;
    vk::DescriptorSetLayout m_camera_data_layout;
    vk::DescriptorSetLayout m_texture_set_layout;
    vk::DescriptorPool m_descriptor_pool;
    vk::DescriptorPool m_imgui_pool;
    std::vector<vk::DescriptorSet> m_descriptor_sets;

    std::set<u32> m_queue_indices;
    vk::Queue m_graphics_queue;
    vk::Queue m_present_queue;
    vk::Queue m_transfer_queue;

	void init_instance();
	void init_surface();
	void init_device();
	void init_swapchain();
	void init_render_pass();
	void init_graphics_pipeline();
	void init_command_pools();
	void init_depth_resources();
	void init_framebuffers();
	void init_descriptor_pools();
	void init_descriptor_sets();
	void init_command_buffers();
	void init_sync_objects();
	void init_imgui();

    // extensions the device needs to have
    const std::vector<const char*> m_required_device_extensions =
    {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME,
            VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME
    };

    const std::vector<const char *> m_validation_layers =
    {
            "VK_LAYER_KHRONOS_validation" // default one with SDK
    };

    [[nodiscard]] bool check_validation_layer_support() const;
    [[nodiscard]] static std::vector<const char*> get_required_extensions() ;

    static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
            VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
            VkDebugUtilsMessageTypeFlagsEXT messageType,
            const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
            void* pUserData
    )
    {
        std::cout << "validation layer: " << pCallbackData->pMessage << std::endl;

        return VK_FALSE;
    }
};

