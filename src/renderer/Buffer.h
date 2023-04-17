#pragma once

#include <vector>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/matrix.hpp>

#include "Log.h"

enum class BufferType : int
{
    VERTEX = 0,
    INDEX,
    UNIFORM,
    SHADER_STORAGE
};

class Buffer
{
public:
    Buffer(BufferType buffer_type);
    explicit Buffer(uint64_t size, BufferType buffer_type);
    Buffer(Buffer&& ubo) noexcept;
    ~Buffer();

    void allocate_memory(uint64_t size) const;

    void link(unsigned int binding_point) const;
    void bind() const;
    void unbind() const;

    template<typename T> void set_data(int offset, T&& data) const;
    template<typename T> void set_data(int offset, const std::vector<T>& data) const;

    void operator= (Buffer&& other_buffer);

private:
    unsigned int m_id;
    BufferType m_buffer_type;

    void set_data(int offset, uint64_t size, const void* data) const;
};

template<typename T>
void Buffer::set_data(int offset, T&& data) const
{
    uint64_t size = sizeof(data);
    set_data(offset, size, (void*)&data);
}

template<typename T>
void Buffer::set_data(int offset, const std::vector<T>& data) const
{
    uint64_t size = data.size() * sizeof(T);
    set_data(offset, size, data.data());
}
