#include "Texture.h"

Texture::Texture() :
    m_TextureId(0),
    m_BindingPoint(0),
    m_ReferenceCount(new unsigned int)
{
    CreateResource();
    *m_ReferenceCount = 1;
}

Texture::Texture(GLuint TextureId) :
    m_TextureId(TextureId),
    m_BindingPoint(0),
    m_ReferenceCount(new unsigned int)
{
    *m_ReferenceCount = 1;
}

Texture::Texture(const Texture& Copy) :
    m_TextureId(Copy.m_TextureId),
    m_BindingPoint(Copy.m_BindingPoint),
    m_ReferenceCount(Copy.m_ReferenceCount)
{
    ++(*m_ReferenceCount);
}

Texture& Texture::operator=(const Texture& Copy)
{
    if (Copy.m_TextureId == m_TextureId)
    {
        m_BindingPoint = Copy.m_BindingPoint;
        ++(*m_ReferenceCount);
    }
    else
    {
        if (--(*m_ReferenceCount) == 0)
        {
            DeleteResource();
            delete m_ReferenceCount;
        }
        m_TextureId = Copy.m_TextureId;
        m_BindingPoint = Copy.m_BindingPoint;
        m_ReferenceCount = Copy.m_ReferenceCount;
        ++(*m_ReferenceCount);
    }
    return *this;
}

Texture::~Texture()
{
    if (--(*m_ReferenceCount) == 0)
    {
        DeleteResource();
        delete m_ReferenceCount;
    }
}

void Texture::LoadFromBuffer(unsigned char* Pixels, unsigned int Width, unsigned int Height, GLenum Format)
{
    assert(m_TextureId);
    
    // We actually don't need to specify which binding point to use
    // (via glActiveTexture()) so just bind to GL_TEXTURE_2D target
    glBindTexture(GL_TEXTURE_2D, m_TextureId);
    
    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 Format,
                 Width,
                 Height,
                 0,
                 Format,
                 GL_UNSIGNED_BYTE,
                 Pixels);

}

void Texture::LoadFromBuffer(float* Pixels, unsigned int Width, unsigned int Height, GLenum Format)
{
    assert(m_TextureId);
    
    // We actually don't need to specify which binding point to use
    // (via glActiveTexture()) so just bind to GL_TEXTURE_2D target
    glBindTexture(GL_TEXTURE_2D, m_TextureId);
    
    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 Format,
                 Width,
                 Height,
                 0,
                 Format,
                 GL_FLOAT,
                 Pixels);
}

void Texture::Use()
{
    // TODO: Add support for other targets than GL_TEXTURE_2D?
    glActiveTexture(GL_TEXTURE0 + m_BindingPoint);
    glBindTexture(GL_TEXTURE_2D, m_TextureId);
}

void Texture::CreateResource()
{
    glGenTextures(1, &m_TextureId);
}

void Texture::DeleteResource()
{
    glDeleteTextures(1, &m_TextureId);
}
