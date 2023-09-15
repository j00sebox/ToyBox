#pragma once
#include "Types.hpp"
//#include "scene/Scene.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vulkan/vulkan.hpp>
#include <vk_mem_alloc.h>

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
    [[nodiscard]] std::vector<const char*> get_required_extensions() const;

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

