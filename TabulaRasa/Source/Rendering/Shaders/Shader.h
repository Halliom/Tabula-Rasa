#pragma once

#include "GL/glew.h"

class IShader
{
public:
    
    IShader();
    
    virtual ~IShader();
    
    /**
     * Override this and call super first thing. Then you can send
     * any information to any uniforms you set up in the Initialize()
     * function or do anything else before rendering commands are issued
     */
    virtual void Use();
    
    /**
     * Override this and call super first thing. Then you can
     * initialize all your uniforms and variables and layout locations
     * and whatnot
     */
    virtual void Initialize();
    
    /**
     * Gets a reference to a uniform that you can then use to set 
     * uniforms using the provided template specializations
     */
    GLint GetUniform(const char* UniformName);
    
    template<typename T>
    void SetUniform(GLint Uniform, const T& Value);
    
    template<typename T>
    void SetUniform3(GLint Uniform, int Count, const T* Value);
    
protected:
    
    virtual void LoadShader(GLint Program);
    
    void LinkToShaderFromSource(const char* Source, GLenum ShaderType);
    
private:
    
    void CompileShader();
    
    GLint   m_Program;
};