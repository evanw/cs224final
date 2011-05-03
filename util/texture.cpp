#include "texture.h"
#include "util.h"
#include <qgl.h>
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

void Texture::startDrawingTo()
{
    if (!framebuffer) glGenFramebuffers(1, &framebuffer);
    if (!renderbuffer) glGenRenderbuffers(1, &renderbuffer);
    if (bufferWidth != width || bufferHeight != height)
    {
        glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, width, height);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, renderbuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, target, id, 0);
    switch (glCheckFramebufferStatus(GL_FRAMEBUFFER))
    {
        case GL_FRAMEBUFFER_UNDEFINED: cout << "GL_FRAMEBUFFER_UNDEFINED" << endl; exit(0);
        case GL_FRAMEBUFFER_UNSUPPORTED: cout << "GL_FRAMEBUFFER_UNSUPPORTED" << endl; exit(0);
        case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT: cout << "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT" << endl; exit(0);
        case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER: cout << "GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER" << endl; exit(0);
        case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER: cout << "GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER" << endl; exit(0);
        case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE: cout << "GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE" << endl; exit(0);
        case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT: cout << "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT" << endl; exit(0);
    }
    glGetIntegerv(GL_VIEWPORT, viewport);
    glViewport(0, 0, width, height);
}

void Texture::stopDrawingTo()
{
    glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
}
