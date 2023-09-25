#include "pch.h"
#include "RenderTypes.hpp"

void PipelineConfig::set_rasterizer(const RasterizationConfig& rasterization_config)
{
    rasterization.sType = vk::StructureType::ePipelineRasterizationStateCreateInfo;
    rasterization.depthClampEnable = false;
    rasterization.rasterizerDiscardEnable = false;
    rasterization.polygonMode = rasterization_config.polygon_mode;
    rasterization.lineWidth = 1.f;
    rasterization.cullMode = rasterization_config.cull_mode;
    rasterization.frontFace = rasterization_config.front_face;
    rasterization.depthBiasEnable = false;
    rasterization.depthBiasConstantFactor = 0.f;
    rasterization.depthBiasClamp = 0.f;
    rasterization.depthBiasSlopeFactor = 0.f;
}

void PipelineConfig::set_binding_description(const VertexBindingDescription& binding_description)
{
    vertex_binding_description.binding = binding_description.binding;
    vertex_binding_description.stride = binding_description.stride;
    vertex_binding_description.inputRate = vk::VertexInputRate::eVertex;
}

void PipelineConfig::set_input_assembly(const vk::PrimitiveTopology& topology)
{
    pipeline_input_assembly.sType = vk::StructureType::ePipelineInputAssemblyStateCreateInfo;
    pipeline_input_assembly.topology = topology;
    pipeline_input_assembly.primitiveRestartEnable = false;
}

void PipelineConfig::add_shader_stage(const ShaderStageConfig& shader_config)
{
    shader_stages[shader_count++] = shader_config;
}

void PipelineConfig::add_vertex_attribute(const VertexAttribute& attribute)
{
    vertex_attributes[vertex_attribute_count++] = attribute;
}

void PipelineConfig::add_descriptor_set_layout(DescriptorSetLayoutHandle descriptor_set_layout_handle)
{
    descriptor_set_layouts[descriptor_set_layout_count++] = descriptor_set_layout_handle;
}

void PipelineConfig::add_push_constant(vk::PushConstantRange push_constant_config)
{
    push_constants[push_constant_count++] = push_constant_config;
}

void PipelineConfig::add_colour_attachment(const vk::PipelineColorBlendAttachmentState& colour_blend_attachment)
{
    colour_attachments[colour_attachment_count++] = colour_blend_attachment;
}

void PipelineConfig::add_depth_stencil_attachment(const vk::PipelineDepthStencilStateCreateInfo& depth_stencil_create_info)
{
    depth_stencil_attachment = depth_stencil_create_info;
}