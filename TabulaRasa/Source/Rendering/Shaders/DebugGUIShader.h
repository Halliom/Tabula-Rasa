#pragma once

#include "Shader.h"

#include "glm/glm.hpp"

class DebugGUIShader : public IShader
{
public:
    
    DebugGUIShader() { }

    void Use() override;

    void Initialize() override;

    glm::mat4 m_ProjectionMatrix;

protected:

    void LoadShader(GLint Program) override;

private:

    GLint m_ProjectionUniformLocation;
};
