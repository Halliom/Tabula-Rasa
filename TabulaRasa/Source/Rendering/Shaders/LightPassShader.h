#pragma once

#include "Shader.h"

#include "glm/glm.hpp"

class LightPassShader : public IShader
{
public:

    LightPassShader() {}

    void Use() override;

    void Initialize() override;

protected:

    void LoadShader(GLint Program) override;

private:

    GLint m_TextureSamplers[6];

};
