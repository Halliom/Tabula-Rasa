#pragma once

#include "Shader.h"

#include "glm/glm.hpp"

class ChunkRenderShader : public IShader
{
public:
    
    ChunkRenderShader() {}
    
    void Use() override;

    void Initialize() override;
    
    glm::mat4 m_ProjectionMatrix;
    glm::mat4 m_ViewMatrix;
    glm::mat4 m_ModelMatrix;

protected:
    
    void LoadShader(GLint Program) override;
    
private:
    
    GLint m_ProjectionUniformLocation;
    GLint m_ViewUniformLocation;
    GLint m_ModelUniformLocation;
};