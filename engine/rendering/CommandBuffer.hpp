#pragma once
#include "CommonTypes.hpp"
#include "rendering/RenderTypes.hpp"

#include <vulkan/vulkan.hpp>

struct CommandBuffer
{
    void begin(vk::CommandBufferUsageFlags flags = vk::CommandBufferUsageFlags());
    void begin(vk::CommandBufferInheritanceInfo inheritance_info);
    void begin_renderpass(const vk::RenderPass& renderpass, const vk::Framebuffer& framebuffer, vk::Extent2D swapchain_extent, vk::SubpassContents subpass_contents) const;
    void bind_pipeline(const Pipeline* pipeline) const;
    void set_viewport(u32 width, u32 height) const;
    void set_scissor(vk::Extent2D extent) const;
    void bind_descriptor_set(const Pipeline* pipeline, u32 set, u32 descriptor_count, const DescriptorSet* descriptor_set) const;
    void push_constants(const Pipeline* pipeline, vk::ShaderStageFlagBits shader_stage, u32 offset, size_t size, const void* data) const;
    void bind_vertex_buffer(Buffer* vertex_buffer) const;
    void bind_index_buffer(Buffer* index_buffer) const;
    void draw_indexed(u32 index_count) const;
    void end_renderpass() const;
    void end();
    void execute_command(const CommandBuffer* command_buffer) const;

    [[nodiscard]] vk::CommandBuffer& get() { return vk_command_buffer; }
    [[nodiscard]] const vk::CommandBuffer& get() const { return vk_command_buffer; }

private:
    vk::CommandBuffer vk_command_buffer;
    bool is_recording = false;
};


