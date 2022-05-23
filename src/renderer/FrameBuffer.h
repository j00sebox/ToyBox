#pragma once

enum class AttachmentType
{
    None = 0,
    Colour,
    Depth,
    Stencil
};

class FrameBuffer
{
public:
    FrameBuffer(int width = 800, int height = 600);
    FrameBuffer(FrameBuffer&& fbo);
    ~FrameBuffer();

    void attach_texture(AttachmentType attachment);
    void attach_renderbuffer(AttachmentType attachment);

    void bind() const;
    void unbind() const;

    [[nodiscard]] bool is_complete() const;

    [[nodiscard]] unsigned int get_colour_attachment() const { return m_colour_attachment; }
    [[nodiscard]] unsigned int get_depth_attachment() const { return m_depth_attachment; }
    [[nodiscard]] unsigned int get_stencil_attachment() const { return m_stencil_attachment; }

private:
    unsigned int m_id;
    unsigned int m_colour_attachment;
    unsigned int m_depth_attachment;
    unsigned int m_stencil_attachment;

    int m_width, m_height;
};