#include "pch.h"
#include "CommandBuffer.hpp"

void CommandBuffer::begin(vk::CommandBufferUsageFlags flags)
{
    vk_command_buffer.reset();
    vk::CommandBufferBeginInfo begin_info{};
    begin_info.sType = vk::StructureType::eCommandBufferBeginInfo;
    begin_info.flags = flags;
    begin_info.pInheritanceInfo = nullptr; // only relevant to secondary command buffers

    if (vk_command_buffer.begin(&begin_info)!= vk::Result::eSuccess)
    {
        throw std::runtime_error("failed to begin recording of framebuffer!");
    }
    is_recording = true;
}

void CommandBuffer::begin(vk::CommandBufferInheritanceInfo inheritance_info)
{
    vk_command_buffer.reset();

    vk::CommandBufferBeginInfo secondary_begin_info{};
    secondary_begin_info.flags = vk::CommandBufferUsageFlagBits::eRenderPassContinue | vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
    secondary_begin_info.pInheritanceInfo = &inheritance_info;

    vk_command_buffer.begin(secondary_begin_info);

    is_recording = true;
}

void CommandBuffer::begin_renderpass(const vk::RenderPass& renderpass, const vk::Framebuffer& framebuffer, vk::Extent2D swapchain_extent, vk::SubpassContents subpass_contents) const
{
    vk::RenderPassBeginInfo renderpass_begin_info{};
    renderpass_begin_info.sType = vk::StructureType::eRenderPassBeginInfo;
    renderpass_begin_info.renderPass = renderpass;
    renderpass_begin_info.framebuffer = framebuffer;
    renderpass_begin_info.renderArea.offset = vk::Offset2D{0, 0};
    renderpass_begin_info.renderArea.extent = swapchain_extent;

    std::array<vk::ClearValue, 2> clear_values{};
    vk::ClearColorValue value;
    value.float32[0] = 0.f; value.float32[1] = 0.f; value.float32[2] = 0.f; value.float32[3] = 1.f;
    clear_values[0].color = value;
    clear_values[1].depthStencil = vk::ClearDepthStencilValue{1.f, 0};

    renderpass_begin_info.clearValueCount = static_cast<u32>(clear_values.size());
    renderpass_begin_info.pClearValues = clear_values.data();

    vk_command_buffer.beginRenderPass(&renderpass_begin_info, subpass_contents);
}

void CommandBuffer::bind_pipeline(const Pipeline* pipeline) const
{
    vk_command_buffer.bindPipeline(pipeline->vk_bindpoint, pipeline->vk_pipeline);
}

void CommandBuffer::set_viewport(u32 width, u32 height) const
{
    vk::Viewport viewport{};
    viewport.x = 0.f;
    viewport.y = 0.f;
    viewport.width = static_cast<float>(width);
    viewport.height = static_cast<float>(height);
    viewport.minDepth = 0.f;
    viewport.maxDepth = 1.f;
    vk_command_buffer.setViewport(0, 1, &viewport);
}

void CommandBuffer::set_scissor(vk::Extent2D extent) const
{
    vk::Rect2D scissor{};
    scissor.offset = vk::Offset2D{0, 0};
    scissor.extent = extent;
    vk_command_buffer.setScissor(0, 1, &scissor);
}

void CommandBuffer::bind_descriptor_set(const Pipeline* pipeline, u32 set, u32 descriptor_count, const DescriptorSet* descriptor_set) const
{
    vk_command_buffer.bindDescriptorSets(pipeline->vk_bindpoint,
                                         pipeline->vk_pipeline_layout,
                                         set, descriptor_count,
                                         &descriptor_set->vk_descriptor_set,
                                         0, nullptr);
}

void CommandBuffer::push_constants(const Pipeline *pipeline, vk::ShaderStageFlagBits shader_stage, u32 offset, size_t size, const void *data) const
{
    vk_command_buffer.pushConstants(pipeline->vk_pipeline_layout,
                                      shader_stage,
                                      offset,
                                      size,
                                      data);
}

void CommandBuffer::bind_vertex_buffer(Buffer* vertex_buffer) const
{
    vk::Buffer vertex_buffers[] = {vertex_buffer->vk_buffer};
    vk::DeviceSize offsets[] = {0};
    vk_command_buffer.bindVertexBuffers(0, 1, vertex_buffers, offsets);
}

void CommandBuffer::bind_index_buffer(Buffer* index_buffer) const
{
    vk_command_buffer.bindIndexBuffer(index_buffer->vk_buffer, 0, vk::IndexType::eUint32);
}

void CommandBuffer::draw_indexed(u32 index_count) const
{
    vk_command_buffer.drawIndexed(index_count, 1, 0, 0, 0);
}

void CommandBuffer::end_renderpass() const
{
    vk_command_buffer.endRenderPass();
}

void CommandBuffer::end()
{
    if(is_recording)
    {
        vk_command_buffer.end();
        is_recording = false;
    }
}

void CommandBuffer::execute_command(const CommandBuffer* command_buffer) const
{
    vk_command_buffer.executeCommands(1, &command_buffer->vk_command_buffer);
}