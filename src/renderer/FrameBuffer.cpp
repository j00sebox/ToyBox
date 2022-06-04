#include "pch.h"
#include "FrameBuffer.h"

#include "GLError.h"
#include "Log.h"

#include <cassert>
#include <glad/glad.h>

FrameBuffer::FrameBuffer(int width, int height, unsigned int samples)
    : m_width(width), m_height(height), m_samples(samples)
{
    assert(samples != 0);
    GL_CALL(glGenFramebuffers(1, &m_id));
    m_colour_attachment = 0;
    m_depth_attachment = 0;
    m_stencil_attachment = 0;
}

FrameBuffer::FrameBuffer(FrameBuffer&& fbo) noexcept
{
    m_id = fbo.m_id;
    m_width = fbo.m_width;
    m_height = fbo.m_height;
    m_samples = fbo.m_samples;
    m_colour_attachment = fbo.m_colour_attachment;
    m_depth_attachment = fbo.m_depth_attachment;
    m_stencil_attachment = fbo.m_stencil_attachment;
    fbo.m_colour_attachment = 0;
    fbo.m_depth_attachment = 0;
    fbo.m_stencil_attachment = 0;
    fbo.m_id = 0;
}

FrameBuffer::~FrameBuffer()
{
    GL_CALL(glDeleteFramebuffers(1, &m_id));
}

void FrameBuffer::blit(unsigned int dest_buffer) const
{
    GL_CALL(glBindFramebuffer(GL_READ_FRAMEBUFFER, m_id));
    GL_CALL(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dest_buffer));
    GL_CALL(glBlitFramebuffer(0, 0, m_width, m_height, 0, 0, m_width, m_height, GL_COLOR_BUFFER_BIT, GL_NEAREST));
}

void FrameBuffer::attach_texture(unsigned char attachment)
{
    switch (attachment)
    {
        case AttachmentTypes::Colour:
        {
            GL_CALL(glGenTextures(1, &m_colour_attachment));

            if(m_samples > 1)
            {
                GL_CALL(glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_colour_attachment));
                GL_CALL(glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, m_samples, GL_RGBA, m_width, m_height, GL_TRUE));
                GL_CALL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, m_colour_attachment, 0));
                GL_CALL(glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0));
            }
            else
            {
                GL_CALL(glBindTexture(GL_TEXTURE_2D, m_colour_attachment));
                GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_INT, nullptr));
                GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
                GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
                GL_CALL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_colour_attachment, 0));
            }
            break;
        }

        case AttachmentTypes::Depth:
        {
            GL_CALL(glGenTextures(1, &m_depth_attachment));

            if(m_samples > 1)
            {
                GL_CALL(glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_depth_attachment));
                GL_CALL(glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, m_samples, GL_RGBA, m_width, m_height, GL_TRUE));
                GL_CALL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D_MULTISAMPLE, m_depth_attachment, 0));
            }
            else
            {
                GL_CALL(glBindTexture(GL_TEXTURE_2D, m_depth_attachment));
                GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, m_width, m_height, 0, GL_DEPTH_COMPONENT32F, GL_UNSIGNED_INT, nullptr));
                GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
                GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
                GL_CALL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_depth_attachment, 0));
            }
            break;
        }

        case AttachmentTypes::Stencil:
        {
            GL_CALL(glGenTextures(1, &m_stencil_attachment));

            if(m_samples > 1)
            {
                GL_CALL(glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_stencil_attachment));
                GL_CALL(glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, m_samples, GL_RGBA, m_width, m_height, GL_TRUE));
                GL_CALL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D_MULTISAMPLE, m_stencil_attachment, 0));
            }
            else
            {
                GL_CALL(glBindTexture(GL_TEXTURE_2D, m_stencil_attachment));
                GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_STENCIL_INDEX, m_width, m_height, 0, GL_STENCIL_INDEX, GL_UNSIGNED_INT, nullptr));
                GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
                GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
                GL_CALL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D, m_stencil_attachment, 0));
            }
            break;
        }
    
    default:
        break;
    }
}

void FrameBuffer::attach_renderbuffer(unsigned char attachment)
{
    switch (attachment)
    {
        case AttachmentTypes::Colour:
        {
            GL_CALL(glGenRenderbuffers(1, &m_colour_attachment));
            GL_CALL(glBindRenderbuffer(GL_RENDERBUFFER, m_colour_attachment));

            if(m_samples > 1)
            {
                GL_CALL(glRenderbufferStorageMultisample(GL_RENDERBUFFER, m_samples, GL_RGBA, m_width, m_height));
            }
            else
            {
                GL_CALL(glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA, m_width, m_height));
            }

            GL_CALL(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, m_colour_attachment));
            break;   
        }

        case AttachmentTypes::Depth:
        {
            GL_CALL(glGenRenderbuffers(1, &m_depth_attachment));
            GL_CALL(glBindRenderbuffer(GL_RENDERBUFFER, m_depth_attachment));

            if(m_samples > 1)
            {
                GL_CALL(glRenderbufferStorageMultisample(GL_RENDERBUFFER, m_samples, GL_DEPTH_COMPONENT32F, m_width, m_height));
            }
            else
            {
                GL_CALL(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32F, m_width, m_height));
            }

            GL_CALL(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depth_attachment));
            break;   
        }

        case AttachmentTypes::Stencil:
        {
            GL_CALL(glGenRenderbuffers(1, &m_stencil_attachment));
            GL_CALL(glBindRenderbuffer(GL_RENDERBUFFER, m_stencil_attachment));

            if(m_samples > 1)
            {
                GL_CALL(glRenderbufferStorageMultisample(GL_RENDERBUFFER, m_samples, GL_STENCIL_INDEX, m_width, m_height));
            }
            else
            {
                GL_CALL(glRenderbufferStorage(GL_RENDERBUFFER, GL_STENCIL_INDEX, m_width, m_height));
            }

            GL_CALL(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_stencil_attachment));
            break;   
        }

        case (AttachmentTypes::Depth | AttachmentTypes::Stencil):
        {
            GL_CALL(glGenRenderbuffers(1, &m_depth_attachment));
            GL_CALL(glBindRenderbuffer(GL_RENDERBUFFER, m_depth_attachment));

            if(m_samples > 1)
            {
                GL_CALL(glRenderbufferStorageMultisample(GL_RENDERBUFFER, m_samples, GL_DEPTH24_STENCIL8, m_width, m_height));
            }
            else
            {
                GL_CALL(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, m_width, m_height));
            }

            GL_CALL(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_depth_attachment));
            break;
        }

    default:
        break;
    }
}

void FrameBuffer::bind() const
{
    GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, m_id));
}

void FrameBuffer::unbind() const
{
    // go back to using default frame buffer
    GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

// fbo needs to be bound first
bool FrameBuffer::is_complete() const
{
    auto fb_status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    info("Frame Buffer Status: {0:#x}\n", fb_status);
    return (fb_status == GL_FRAMEBUFFER_COMPLETE);
}
