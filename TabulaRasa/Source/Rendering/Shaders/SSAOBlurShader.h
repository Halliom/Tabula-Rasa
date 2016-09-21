#pragma once

#include "Shader.h"

#include "glm/glm.hpp"

class SSAOBlurShader : public IShader
{
public:

    SSAOBlurShader() {}

    void Use() override;

    void Initialize() override;

protected:

    void LoadShader(GLint Program) override;

private:

    GLint m_TextureSamplers[6];
    
};
