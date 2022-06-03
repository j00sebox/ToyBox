#include "pch.h"
#include "ViewPort.h"

#include "Log.h"
//#include "FrameBuffer.h"
#include "EventList.h"

#include <cassert>
#include <imgui.h>
#include <imgui_internal.h>

ViewPort::ViewPort(int width, int height, int samples)
    : m_width(width), m_height(height), m_samples(samples),
    m_prev_fb_width((float)width), m_prev_fb_height((float)height)
{
    resize(width, height, samples);
}

void ViewPort::begin_frame() const
{
    m_multisample_frame_buffer->bind();
}

void ViewPort::end_frame() const
{
    m_frame_buffer->unbind();
}

void ViewPort::display()
{
    ImVec2 avail_size = ImGui::GetContentRegionAvail();
    ImVec2 pos = ImGui::GetCursorScreenPos();

    if(m_prev_fb_width != avail_size.x || m_prev_fb_height != avail_size.y)
    {
        info("New screen size [x: {}, y: {}]\n", avail_size.x, avail_size.y);
        resize((int)avail_size.x, (int)avail_size.y, m_samples);
        EventList::e_resize.execute_function((int)avail_size.x, (int)avail_size.y);
        m_prev_fb_width = avail_size.x;
        m_prev_fb_height = avail_size.y;
    }
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    m_multisample_frame_buffer->blit(m_frame_buffer->get_id());
    drawList->AddImage((void*)m_frame_buffer->get_colour_attachment(),
       pos,
       ImVec2(pos.x + avail_size.x, pos.y + avail_size.y),
       ImVec2(0, 1),
       ImVec2(1, 0));
}

void ViewPort::resize(int width, int height, int samples)
{
    m_multisample_frame_buffer = std::make_unique<FrameBuffer>(width, height, 4);
    m_multisample_frame_buffer->bind();

    m_multisample_frame_buffer->attach_texture(AttachmentTypes::Colour);
    m_multisample_frame_buffer->attach_renderbuffer(AttachmentTypes::Depth | AttachmentTypes::Stencil); // create one render buffer object for both

    assert(m_multisample_frame_buffer->is_complete());

    m_multisample_frame_buffer->unbind();

    m_frame_buffer = std::make_unique<FrameBuffer>();
    m_frame_buffer->bind();

    // want the main buffer to have a texture colour for imgui
    m_frame_buffer->attach_texture(AttachmentTypes::Colour);
    m_frame_buffer->attach_renderbuffer(AttachmentTypes::Depth | AttachmentTypes::Stencil); // create one render buffer object for both

    assert(m_frame_buffer->is_complete());

    m_frame_buffer->unbind();
}