#include "ChunkRenderShader.h"

#include "../../Platform/Platform.h"

void ChunkRenderShader::Use()
{
    IShader::Use();
    
    SetUniform(m_ProjectionUniformLocation, m_ProjectionMatrix);
    SetUniform(m_ViewUniformLocation, m_ViewMatrix);
    SetUniform(m_ModelUniformLocation, m_ModelMatrix);
}

void ChunkRenderShader::Initialize()
{
    IShader::Initialize();
    
    m_ProjectionUniformLocation = GetUniform("g_ProjectionMatrix");
    m_ViewUniformLocation = GetUniform("g_ViewMatrix");
    m_ModelUniformLocation = GetUniform("g_ModelMatrix");
}

void ChunkRenderShader::LoadShader(GLint Program)
{
    // These are allocated on the transient memory so they don't need to be freed
    const char* VertexShaderSource = PlatformFileSystem::LoadFileIntoBuffer(DT_SHADERS, "VertexShader.glsl");
    const char* FragmentShaderSource = PlatformFileSystem::LoadFileIntoBuffer(DT_SHADERS, "FragmentShader.glsl");
    
    LinkToShaderFromSource(VertexShaderSource, GL_VERTEX_SHADER);
    LinkToShaderFromSource(FragmentShaderSource, GL_FRAGMENT_SHADER);
}