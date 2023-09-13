#pragma once

#include "FrameBuffer.h"

class ViewPort
{
public:
    ViewPort(int width, int height, int samples = 1);

    void begin_frame() const;
    void end_frame() const;

    void display();
    void resize(int width, int height, int samples);
    void bind_framebuffer() const;

    [[nodiscard]] std::pair<int, int> get_dimensions() const { return { m_width, m_height }; }
    [[nodiscard]] int get_sample_amount() const { return m_samples; }

private:
    int m_width, m_height;
    int m_samples;
    float m_prev_fb_width = 0.f, m_prev_fb_height = 0.f;
    std::unique_ptr<FrameBuffer> m_frame_buffer;
    std::unique_ptr<FrameBuffer> m_multisample_frame_buffer;
};