#include "pch.h"
#include "FrameBuffer.h"

#include "GLError.h"

#include <glad/glad.h>

FrameBuffer::FrameBuffer(int width, int height)
    : m_width(width), m_height(height) 
{
    glGenFramebuffers(1, &m_id);
}

FrameBuffer::FrameBuffer(FrameBuffer&& fbo)
{
    m_id = fbo.m_id;
    fbo.m_id = 0;
}

FrameBuffer::~FrameBuffer()
{
    glDeleteFramebuffers(1, &m_id);
}

void FrameBuffer::attach_texture(AttachmentType attachment)
{
    switch (attachment)
    {
        case AttachmentType::Colour:
        {
            glGenTextures(1, &m_colour_attachment);
            glBindTexture(GL_TEXTURE_2D, m_colour_attachment);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_width, m_height, 0, GL_RGB, GL_UNSIGNED_INT, NULL);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_colour_attachment, 0);
            break;   
        }

        case AttachmentType::Depth:
        {
            glGenTextures(1, &m_depth_attachment);
            glBindTexture(GL_TEXTURE_2D, m_depth_attachment);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, m_width, m_height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, NULL);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_depth_attachment, 0);
            break;   
        }

        case AttachmentType::Stencil:
        {
            glGenTextures(1, &m_stencil_attachment);
            glBindTexture(GL_TEXTURE_2D, m_stencil_attachment);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_STENCIL_INDEX, m_width, m_height, 0, GL_STENCIL_INDEX, GL_UNSIGNED_INT, NULL);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D, m_stencil_attachment, 0);
            break;   
        }
    
    default:
        break;
    }
}

void FrameBuffer::attach_renderbuffer(AttachmentType attachment)
{
    switch (attachment)
    {
        case AttachmentType::Colour:
        {
            glGenRenderbuffers(1, &m_colour_attachment);
            glBindRenderbuffer(GL_RENDERBUFFER, m_colour_attachment);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_RGB, m_width, m_height);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, m_colour_attachment);
            break;   
        }

        case AttachmentType::Depth:
        {
            glGenRenderbuffers(1, &m_depth_attachment);
            glBindRenderbuffer(GL_RENDERBUFFER, m_depth_attachment);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, m_width, m_height);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depth_attachment);
            break;   
        }

        case AttachmentType::Stencil:
        {
            glGenRenderbuffers(1, &m_stencil_attachment);
            glBindRenderbuffer(GL_RENDERBUFFER, m_stencil_attachment);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_STENCIL_INDEX, m_width, m_height);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_stencil_attachment);
            break;   
        }
    
    default:
        break;
    }
}

void FrameBuffer::bind() const
{
    glBindFramebuffer(GL_FRAMEBUFFER, m_id);
}

void FrameBuffer::unbind() const
{
    // go back to using default frame buffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// fbo needs to be bound first
bool FrameBuffer::is_complete() const
{
    return (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
}
