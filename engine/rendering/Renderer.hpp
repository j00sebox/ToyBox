#pragma once
#include "Types.hpp"
#include "GpuResources.hpp"
#include "CommandBuffer.hpp"
#include "Memory.hpp"

//#include "scene/Scene.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>
#include <vk_mem_alloc.h>
#include <TaskScheduler.h>

// FIXME: REMOOOOOVE
//#include <glm/mat4x4.hpp>
//struct Mesh
//{
//    BufferHandle             vertex_buffer;
//    BufferHandle             index_buffer;
//    u32             index_count = 0;
//};
//
//struct Material
//{
//    DescriptorSetHandle descriptor_set;
//    TextureHandle textures[4];
//    SamplerHandle sampler;
//};
//
//struct Model
//{
//    std::vector<Mesh>           meshes;
//    std::vector<Material>       materials;
//    std::vector<glm::mat4>      transforms;
//    glm::mat4                   transform{1.f};
//};
//
//class Renderer;
//
//#include "scene/Camera.h"
//class Scene
//{
//public:
//    Camera camera;
//    std::vector<Model> models;
//
//    void add_model(const Model& model)
//    {
//        models.push_back(model);
//    }
//
//    void update(f32 delta_time)
//    {
//        camera.update(delta_time);
//    }
//
//    void close(Renderer* renderer);
//};

class Renderer
{
public:
    explicit Renderer(GLFWwindow* window, enki::TaskScheduler* scheduler);
    ~Renderer();

	void render(Scene* scene);
	void begin_frame();
	void end_frame();

    void wait_for_device_idle() const { m_logical_device.waitIdle(); }

    // resource creation
    BufferHandle create_buffer(const BufferCreationInfo& buffer_creation);
    TextureHandle create_texture(const TextureCreationInfo& texture_creation);
    SamplerHandle create_sampler(const SamplerCreationInfo& sampler_creation);
    DescriptorSetHandle create_descriptor_set(const DescriptorSetCreationInfo& descriptor_set_creation);
    void create_image(u32 width, u32 height, vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties, vk::Image& image, vk::DeviceMemory& image_memory);
    void create_image(u32 width, u32 height, vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties, vk::Image& image, VmaAllocation& image_vma);
    vk::ImageView create_image_view(const vk::Image& image, vk::Format format, vk::ImageAspectFlags image_aspect);
    vk::ShaderModule create_shader_module(const std::vector<char>& code);

    // resource destruction
    void destroy_buffer(BufferHandle buffer_handle);
    void destroy_texture(TextureHandle texture_handle);
    void destroy_sampler(SamplerHandle sampler_handle);

    [[nodiscard]] Buffer* get_buffer(BufferHandle buffer_handle) { return static_cast<Buffer*>(m_buffer_pool.access(buffer_handle)); }
    [[nodiscard]] const vk::DescriptorSetLayout& get_texture_layout() const { return m_texture_set_layout; }
    [[nodiscard]] TextureHandle get_null_texture_handle() const { return m_null_texture; }
    [[nodiscard]] const vk::PipelineLayout& get_pipeline_layout() { return m_pipeline_layout; }

    void update_texture_set(TextureHandle* texture_handles, u32 num_textures);

private:
    GLFWwindow* m_window;
    enki::TaskScheduler* m_scheduler;

    u32 m_current_frame = 0; // current frame index
    u32 m_current_cb_index = 0; // command buffer to write to
    u32 m_image_index = 0; // image of the swapchain being used

    // constants
    static const u32 k_max_frames_in_flight = 3;
    const u32 k_max_bindless_resources = 1024;
    const u32 k_max_descriptor_sets = 20;
    const u32 k_bindless_texture_binding = 10;
    const u32 k_bindless_image_binding = 11;

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

    // command pools
    vk::CommandPool m_main_command_pool;
    vk::CommandPool m_extra_command_pool;
    std::vector<vk::CommandPool> m_command_pools;

    // command buffers
    std::array<CommandBuffer, k_max_frames_in_flight> m_primary_command_buffers;
    std::vector<CommandBuffer> m_command_buffers;
    std::array<CommandBuffer, k_max_frames_in_flight> m_extra_draw_commands;
    std::array<CommandBuffer, k_max_frames_in_flight> m_imgui_commands;

    // sync objects
    std::array<vk::Semaphore, k_max_frames_in_flight> m_image_available_semaphores;
    std::array<vk::Semaphore, k_max_frames_in_flight> m_render_finished_semaphores;
    std::array<vk::Fence, k_max_frames_in_flight> m_in_flight_fences;

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
    void init_descriptor_pools();
    void init_descriptor_sets();
	void init_graphics_pipeline();
	void init_command_pools();
    void init_command_buffers();
    void init_depth_resources();
    void init_framebuffers();
	void init_sync_objects();
	void init_imgui();

    vk::CommandBuffer begin_single_time_commands();
    void end_single_time_commands(vk::CommandBuffer command_buffer);

    void cleanup_swapchain();
    void recreate_swapchain();
    [[nodiscard]] size_t pad_uniform_buffer(size_t original_size) const;

    // image operations
    void copy_buffer_to_image(vk::Buffer buffer, vk::Image image, u32 width, u32 height);
    void transition_image_layout(vk::Image image, vk::Format format, vk::ImageLayout old_layout, vk::ImageLayout new_layout);

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

