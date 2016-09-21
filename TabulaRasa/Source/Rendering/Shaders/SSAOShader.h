#pragma once

#include "Shader.h"

#include "glm/glm.hpp"

class SSAOShader : public IShader
{
public:

    SSAOShader() { }

    void Use() override;

    void Initialize() override;

    glm::mat4   m_ProjectionMatrix;
    glm::vec2   m_ScreenDimension;
    float*      m_SSAOSamples;

protected:

    void LoadShader(GLint Program) override;

private:

    GLint m_ProjectionUniformLocation;
    GLint m_ScreenDimensionUniformLocation;
    GLint m_SSAOSamplesUniformLocation;
    GLint m_TextureSamplers[6];
};
