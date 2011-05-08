#include "texture.h"
#define GL_GLEXT_PROTOTYPES
#include <qgl.h>
#include <assert.h>
#include <iostream>
using namespace std;

Texture::~Texture()
{
    glDeleteTextures(1, &id);
}

void Texture::init(int newTarget, int newWidth, int newHeight, int format, int internalFormat, int type, int wrap, int filter)
{
    target = newTarget;
    width = newWidth;
    height = newHeight;
    if (!id) glGenTextures(1, &id);
    glBindTexture(target, id);
    glTexParameteri(target, GL_TEXTURE_WRAP_S, wrap);
    glTexParameteri(target, GL_TEXTURE_WRAP_T, wrap);
    glTexParameteri(target, GL_TEXTURE_MAG_FILTER, filter);
    glTexParameteri(target, GL_TEXTURE_MIN_FILTER, filter);
    glTexImage2D(target, 0, internalFormat, width, height, 0, format, type, NULL);
}

void Texture::bind(int unit)
{
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(target, id);
}

void Texture::unbind(int unit)
{
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(target, 0);
}

static int viewport[4];
static int bufferWidth = 0;
static int bufferHeight = 0;
static unsigned int framebuffer = 0;
static unsigned int renderbuffer = 0;

void checkStatusAndSetViewport(int width, int height)
{
    switch (glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT))
    {
        case GL_FRAMEBUFFER_UNSUPPORTED_EXT: cout << "GL_FRAMEBUFFER_UNSUPPORTED" << endl; exit(0);
        case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT: cout << "GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS" << endl; exit(0);
        case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT: cout << "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT" << endl; exit(0);
        case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT: cout << "GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER" << endl; exit(0);
        case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT: cout << "GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER" << endl; exit(0);
        case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE_EXT: cout << "GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE" << endl; exit(0);
        case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT: cout << "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT" << endl; exit(0);
    }
    glGetIntegerv(GL_VIEWPORT, viewport);
    glViewport(0, 0, width, height);
}

void Texture::startDrawingTo()
{
    if (!framebuffer) glGenFramebuffersEXT(1, &framebuffer);
    if (!renderbuffer) glGenRenderbuffersEXT(1, &renderbuffer);
    if (bufferWidth != width || bufferHeight != height)
    {
        glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, renderbuffer);
        glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT16, width, height);
        glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, 0);
    }
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, framebuffer);
    glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, renderbuffer);
    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, target, id, 0);
    checkStatusAndSetViewport(width, height);
}

void Texture::startDrawingTo(const Texture &depthTexture)
{
    assert(width == depthTexture.width && height == depthTexture.height);
    if (!framebuffer) glGenFramebuffersEXT(1, &framebuffer);
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, framebuffer);
    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, depthTexture.target, depthTexture.id, 0);
    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, target, id, 0);
    checkStatusAndSetViewport(width, height);
}

void Texture::stopDrawingTo()
{
    glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
}
