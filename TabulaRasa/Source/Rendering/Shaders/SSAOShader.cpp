#include "SSAOShader.h"

#include "../../Platform/Platform.h"

void SSAOShader::Use()
{
    IShader::Use();

    // TODO: This should be made through calls to the IShader class and also:
    // are these calls really necessary?
    if (m_TextureSamplers[0] != -1)
    		glUniform1i(m_TextureSamplers[0], 0);
    if (m_TextureSamplers[1] != -1)
    		glUniform1i(m_TextureSamplers[1], 1);
    if (m_TextureSamplers[2] != -1)
    		glUniform1i(m_TextureSamplers[2], 2);
    if (m_TextureSamplers[3] != -1)
    		glUniform1i(m_TextureSamplers[3], 3);
    if (m_TextureSamplers[4] != -1)
    		glUniform1i(m_TextureSamplers[4], 4);
    if (m_TextureSamplers[5] != -1)
    		glUniform1i(m_TextureSamplers[5], 5);

    SetUniform(m_ProjectionUniformLocation, m_ProjectionMatrix);
    SetUniform3(m_SSAOSamplesUniformLocation, 64, m_SSAOSamples);
}

void SSAOShader::Initialize()
{
    IShader::Initialize();

    m_ProjectionUniformLocation = GetUniform("g_ProjectionMatrix");
    m_ScreenDimensionUniformLocation = GetUniform("ScreenDimension");
    m_SSAOSamplesUniformLocation = GetUniform("g_Samples");

    // TODO: Maybe remove?
    m_TextureSamplers[0] = GetUniform("textureSampler0");
	m_TextureSamplers[1] = GetUniform("textureSampler1");
	m_TextureSamplers[2] = GetUniform("textureSampler2");
	m_TextureSamplers[3] = GetUniform("textureSampler3");
	m_TextureSamplers[4] = GetUniform("textureSampler4");
	m_TextureSamplers[5] = GetUniform("textureSampler5");
}

void SSAOShader::LoadShader(GLint Program)
{
    // These are allocated on the transient memory so they don't need to be freed
    const char* VertexShaderSource = PlatformFileSystem::LoadFileIntoBuffer(DT_SHADERS, "VertexPassthrough.glsl");
    const char* FragmentShaderSource = PlatformFileSystem::LoadFileIntoBuffer(DT_SHADERS, "FragmentShaderSSAO.glsl");

    LinkToShaderFromSource(VertexShaderSource, GL_VERTEX_SHADER);
    LinkToShaderFromSource(FragmentShaderSource, GL_FRAGMENT_SHADER);
}
