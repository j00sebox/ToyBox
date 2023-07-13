#pragma once

#include "Texture.h"

namespace AttachmentTypes {
    enum : unsigned char 
    {
        Colour  = 1,
        Depth   = 2,
        Stencil = 4
    };
}

class FrameBuffer
{
public:
    explicit FrameBuffer(int width = 800, int height = 600, unsigned int samples = 1);
    FrameBuffer(FrameBuffer&& fbo) noexcept;
    ~FrameBuffer();

    void blit(unsigned dest_buffer) const;
    void attach_texture(unsigned char attachment);
    void attach_texture(unsigned char attachment, TextureBase&& texture);
    void attach_renderbuffer(unsigned char attachment);

    void bind() const;
    void unbind() const;

    [[nodiscard]] bool is_complete() const;

    [[nodiscard]] unsigned get_id() const { return m_id; }
    [[nodiscard]] unsigned get_colour_attachment() const { return m_colour_attachment; }
    [[nodiscard]] unsigned get_depth_attachment() const { return m_depth_attachment; }
    [[nodiscard]] unsigned get_stencil_attachment() const { return m_stencil_attachment; }

private:
    unsigned m_id;
    unsigned m_colour_attachment;
    unsigned m_depth_attachment;
    unsigned m_stencil_attachment;

    int m_width, m_height;
    unsigned m_samples;
};