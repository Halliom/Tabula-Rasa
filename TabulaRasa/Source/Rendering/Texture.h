#pragma once

#include "GL/glew.h"

#include "../Engine/Engine.h"

typedef unsigned int* ReferenceCount;

class Texture
{
public:
    
    /**
     * Initializes the texture with an empty object
     */
    Texture();
    
    /**
     * Creates a texture from an existing OGL object, however this
     * method is not preferred, instead create an empty Texture object
     * and use the methods LoadFromBuffer() for example.
     */
    explicit Texture(GLuint TextureId);
    
    Texture(const Texture& Copy);
    
    Texture& operator=(const Texture& Copy);
    
    ~Texture();
    
    void LoadFromBuffer(unsigned char* Pixels, unsigned int Width, unsigned int Height, GLint InternalFormat, GLenum Format);
    
    void LoadFromBuffer(float* Pixels, unsigned int Width, unsigned int Height, GLint InternalFormat, GLenum Format);

	void SetFilteringMode(GLint Mode);

	void SetWrapMode(GLint Mode);
    
    void Use();

	GLuint          m_TextureId;
	unsigned int    m_BindingPoint;
    
private:
    
    void CreateResource();
    
    void DeleteResource();
    
    ReferenceCount  m_ReferenceCount;
};