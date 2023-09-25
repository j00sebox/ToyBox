#pragma once
#include "CommonTypes.hpp"

#include <vulkan/vulkan.hpp>
#include <vk_mem_alloc.h>

const u8                        k_max_descriptor_set_layouts = 8; // max layouts per pipeline
const u16                       k_max_descriptors_per_set = 16;

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

    vk::PipelineBindPoint       vk_bindpoint = vk::PipelineBindPoint();

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

    vk::DescriptorSetLayout             layout;
    u32                                 num_resources;
};
