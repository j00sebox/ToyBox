#pragma once
#include <cstdint>

typedef char                     u8;
typedef uint16_t                u16;
typedef uint32_t                u32;
typedef uint64_t                u64;

typedef int8_t                  i8;
typedef int16_t                 i16;
typedef int32_t                 i32;
typedef int64_t                 i64;

typedef float                   f32;
typedef double                  f64;

struct ResourceHandle
{
    u32 index;
};

struct BufferHandle : ResourceHandle {};
struct TextureHandle : ResourceHandle {};
struct SamplerHandle : ResourceHandle {};
struct DescriptorSetLayoutHandle : ResourceHandle {};
struct DescriptorSetHandle : ResourceHandle {};

// TODO: move somewhere else ?
struct Material
{
    DescriptorSetHandle     descriptor_set;
    TextureHandle           textures[4];
    SamplerHandle           sampler;
};

struct Mesh
{
    BufferHandle            vertex_buffer;
    BufferHandle            index_buffer;
    u32                     index_count = 0;
};
