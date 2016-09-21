#include "DebugGUIShader.h"

#include "../../Platform/Platform.h"

void DebugGUIShader::Use()
{
    IShader::Use();

    SetUniform(m_ProjectionUniformLocation, m_ProjectionMatrix);
}

void DebugGUIShader::Initialize()
{
    IShader::Initialize();

    m_ProjectionUniformLocation = GetUniform("g_ProjectionMatrix");
}

void DebugGUIShader::LoadShader(GLint Program)
{
    // These are allocated on the transient memory so they don't need to be freed
    const char* VertexShaderSource = PlatformFileSystem::LoadFileIntoBuffer(DT_SHADERS, "VertexDebugGUI.glsl");
    const char* FragmentShaderSource = PlatformFileSystem::LoadFileIntoBuffer(DT_SHADERS, "FragmentDebugGUI.glsl");

    LinkToShaderFromSource(VertexShaderSource, GL_VERTEX_SHADER);
    LinkToShaderFromSource(FragmentShaderSource, GL_FRAGMENT_SHADER);
}
