#pragma once
#include "CommonTypes.hpp"

#include <vulkan/vulkan.hpp>
#include <vk_mem_alloc.h>

const u8                        k_max_descriptor_set_layouts = 8; // max layouts per pipeline
const u16                       k_max_descriptors_per_set = 16;
const u16                       k_max_shaders = 4;
const u16                       k_max_vertex_attributes = 6;

const SamplerHandle             k_invalid_sampler_handle = { 0xffffffff };

struct Buffer
{
    VkBuffer                        vk_buffer;
    VmaAllocation                   vma_allocation;

    u32                             size = 0;
    u8*                             mapped_data = nullptr;
};

struct Texture
{
    VkImage                         vk_image;
    VkImageView                     vk_image_view;
    VkFormat                        vk_format;
    VkImageLayout                   vk_image_layout;
    VmaAllocation                   vma_allocation;

    u16                             width = 1;
    u16                             height = 1;
    u16                             depth = 1;
    u8                              mipmaps = 1;

    const char*                     name;
};

struct Sampler
{
    vk::Sampler                       vk_sampler;

    vk::Filter                        min_filter;
    vk::Filter                        mag_filter;

    vk::SamplerAddressMode            address_mode_u;
    vk::SamplerAddressMode            address_mode_v;
    vk::SamplerAddressMode            address_mode_w;
};

struct DescriptorSetLayout
{
    vk::DescriptorSetLayout         vk_descriptor_set_layout;
};

struct DescriptorSet
{
    vk::DescriptorSet                   vk_descriptor_set;

    u32*                                resources = nullptr;
    u32*                                samplers = nullptr;
    u16*                                bindings = nullptr;

    u32                                 num_resources = 0;
};

struct Pipeline
{
    vk::Pipeline                vk_pipeline;
    vk::PipelineLayout          vk_pipeline_layout;

    vk::PipelineBindPoint       vk_bindpoint            = vk::PipelineBindPoint();

    const char*                 name                    = nullptr;
};

struct BufferConfig
{
    vk::BufferUsageFlags            usage;
    u32                             size;
    bool                            persistent = false;
    void*                           data;
};

struct TextureConfig
{
    vk::Format                      format          = vk::Format::eUndefined;
    vk::ImageCreateFlags            flags           = vk::ImageCreateFlags();
    const char*                     image_src;
};

struct SamplerConfig
{
    vk::Filter                      min_filter;
    vk::Filter                      mag_filter;

    vk::SamplerAddressMode          u_mode;
    vk::SamplerAddressMode          v_mode;
    vk::SamplerAddressMode          w_mode;
};

struct DescriptorSetLayoutConfig
{
    struct Binding
    {
        u32                                 binding_index       = 0;
        vk::DescriptorType                  type                = vk::DescriptorType();
        vk::ShaderStageFlags                stage_flags         = vk::ShaderStageFlags();
        u32                                 descriptor_count    = 1;
    };

    Binding                                 bindings[k_max_descriptors_per_set];
    u32                                     num_bindings        = 0;
    u32                                     set_index           = 0;
    vk::DescriptorSetLayoutCreateFlags      flags               = vk::DescriptorSetLayoutCreateFlags();
    void*                                   extension           = nullptr;
};

struct DescriptorSetConfig
{
    ResourceHandle                      resource_handles[k_max_descriptors_per_set];
    SamplerHandle                       sampler_handles[k_max_descriptors_per_set];
    u16                                 bindings[k_max_descriptors_per_set];
    vk::DescriptorType                  types[k_max_descriptors_per_set];

    vk::DescriptorSetLayout             layout          = vk::DescriptorSetLayout();
    u32                                 num_resources   = 0;
};

struct ShaderStageConfig
{
    const char*                 shader_file    = nullptr;
    vk::ShaderStageFlagBits     stage_flags    = vk::ShaderStageFlagBits::eAll;
    const char*                 entry_point    = "main";
};

struct RasterizationConfig
{
    vk::PolygonMode     polygon_mode;
    vk::CullModeFlags   cull_mode;
    vk::FrontFace       front_face;
};

struct VertexBindingDescription
{
    u32 binding     = 0;
    u32 stride      = 0;
};

struct VertexAttribute
{
    u32                 location         = 0;
    vk::Format          format           = vk::Format::eUndefined;
    u32                 offset           = 0;
};

// FIXME: magic numbers
struct PipelineConfig
{
    ShaderStageConfig                                       shader_stages[k_max_shaders];
    vk::VertexInputBindingDescription                       vertex_binding_description;
    VertexAttribute                                         vertex_attributes[k_max_vertex_attributes];
    vk::PipelineInputAssemblyStateCreateInfo                pipeline_input_assembly;
    vk::PipelineRasterizationStateCreateInfo                rasterization;
    vk::PipelineColorBlendAttachmentState                   colour_attachments[2];
    vk::PipelineDepthStencilStateCreateInfo                 depth_stencil_attachment = vk::PipelineDepthStencilStateCreateInfo{};
    DescriptorSetLayoutHandle                               descriptor_set_layouts[k_max_descriptor_set_layouts];
    vk::PushConstantRange                                   push_constants[6];
    vk::RenderPass                                          renderPass;

    u32                                                     shader_count = 0;
    u32                                                     vertex_attribute_count = 0;
    u32                                                     colour_attachment_count = 0;
    u32                                                     descriptor_set_layout_count = 0;
    u32                                                     push_constant_count = 0;

    // will create the given cache file if it doesn't currently exist
    const char*                                             pipeline_cache_location = nullptr;

    void set_rasterizer(const RasterizationConfig& rasterization_config);
    void set_binding_description(const VertexBindingDescription& vertex_binding_description);
    void set_input_assembly(const vk::PrimitiveTopology& topology);
    void add_shader_stage(const ShaderStageConfig& shader_config);
    void add_vertex_attribute(const VertexAttribute& attribute);
    void add_descriptor_set_layout(DescriptorSetLayoutHandle descriptor_set_layout_handle);
    void add_push_constant(vk::PushConstantRange push_constant_config);
    void add_colour_attachment(const vk::PipelineColorBlendAttachmentState& colour_blend_attachment);
    void add_depth_stencil_attachment(const vk::PipelineDepthStencilStateCreateInfo& depth_stencil_create_info);
};
