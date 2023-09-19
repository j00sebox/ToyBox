#pragma once
#include "Types.hpp"
#include "GpuResources.hpp"
#include "Memory.hpp"
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

    // resource creation
    BufferHandle create_buffer(const BufferCreationInfo& buffer_creation);
    TextureHandle create_texture(const TextureCreationInfo& texture_creation);
    SamplerHandle create_sampler(const SamplerCreationInfo& sampler_creation);
    DescriptorSetHandle create_descriptor_set(const DescriptorSetCreationInfo& descriptor_set_creation);
    vk::ImageView create_image_view(const vk::Image& image, vk::Format format, vk::ImageAspectFlags image_aspect);

    // resource destruction
    void destroy_buffer(BufferHandle buffer_handle);
    void destroy_texture(TextureHandle texture_handle);
    void destroy_sampler(SamplerHandle sampler_handle);

private:
    GLFWwindow* m_window;

    // constants
    static const u32 k_max_frames_in_flight = 3;
    const u32 k_max_bindless_resources = 1024;
    const u32 k_max_descriptor_sets = 20;

    vk::Instance m_instance;
    vk::DebugUtilsMessengerEXT m_debug_messenger;
    vk::DispatchLoaderDynamic m_dispatch_loader;
    vk::SurfaceKHR m_surface;
    vk::PhysicalDevice m_physical_device;
    vk::Device m_logical_device;
    vk::PhysicalDeviceProperties m_device_properties;
    VmaAllocator m_allocator;
    PoolAllocator m_pool_allocator;

    // swapchain stuff
    vk::SwapchainKHR m_swapchain;
    vk::Format m_swapchain_image_format;
    vk::Extent2D m_swapchain_extent;
    std::vector<vk::Image> m_swapchain_images;
    std::vector<vk::ImageView> m_swapchain_image_views;
    std::vector<vk::Framebuffer> m_swapchain_framebuffers;

    vk::RenderPass m_render_pass;
    vk::PipelineLayout m_pipeline_layout;
    vk::Pipeline m_graphics_pipeline;

    // descriptor set layouts
    vk::DescriptorSetLayout m_descriptor_set_layout;
    vk::DescriptorSetLayout m_texture_set_layout;
    vk::DescriptorSetLayout m_camera_data_layout;

    std::vector<vk::DescriptorSet> m_descriptor_sets;
    vk::DescriptorPool m_descriptor_pool;
    vk::DescriptorPool m_imgui_pool;

    // descriptors
    std::vector<DescriptorSetHandle> m_camera_sets;
    DescriptorSetHandle m_texture_set;

    // queues
    vk::Queue m_graphics_queue;
    vk::Queue m_present_queue;
    vk::Queue m_transfer_queue;

    // depth buffer
    vk::Image m_depth_image;
    vk::DeviceMemory m_depth_image_memory;
    vk::ImageView m_depth_image_view;

    // uniform buffers
    std::array<BufferHandle, k_max_frames_in_flight> m_camera_buffers;

    // resource pools
    ResourcePool m_buffer_pool;
    ResourcePool m_texture_pool;
    ResourcePool m_sampler_pool;
    ResourcePool m_descriptor_set_pool;

    // texture used when loader can't find one
    TextureHandle m_null_texture;

    // for most texture use
    SamplerHandle m_default_sampler;

    std::map<std::string, TextureHandle> m_texture_map;

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

    void cleanup_swapchain();
    void recreate_swapchain();
    [[nodiscard]] size_t pad_uniform_buffer(size_t original_size) const;

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

