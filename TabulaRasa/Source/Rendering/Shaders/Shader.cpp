#include "Shader.h"

#include <assert.h>

#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "../GuiSystem.h"

IShader::IShader() :
    m_Program(0)
{
}

IShader::~IShader()
{
    glDeleteProgram(m_Program);
}

void IShader::Use()
{
    glUseProgram(m_Program);
}

void IShader::Initialize()
{
    m_Program = glCreateProgram();
    assert(m_Program >= 0);
    
    LoadShader(m_Program);
    
    CompileShader();
}

GLint IShader::GetUniform(const char* UniformName)
{
    return glGetUniformLocation(m_Program, UniformName);
}

template<>
void IShader::SetUniform(GLint Uniform, const int& Value)
{
    glUniform1iv(Uniform, 1, &Value);
}

template<>
void IShader::SetUniform(GLint Uniform, const float& Value)
{
    glUniform1fv(Uniform, 1, &Value);
}

template<>
void IShader::SetUniform(GLint Uniform, const glm::vec2& Value)
{
    glUniform2fv(Uniform, 1, glm::value_ptr(Value));
}

template<>
void IShader::SetUniform(GLint Uniform, const glm::ivec2& Value)
{
    glUniform2iv(Uniform, 1, glm::value_ptr(Value));
}

template<>
void IShader::SetUniform(GLint Uniform, const glm::vec3& Value)
{
    glUniform3fv(Uniform, 1, glm::value_ptr(Value));
}

template<>
void IShader::SetUniform(GLint Uniform, const glm::ivec3& Value)
{
    glUniform3iv(Uniform, 1, glm::value_ptr(Value));
}

template<>
void IShader::SetUniform(GLint Uniform, const glm::vec4& Value)
{
    glUniform4fv(Uniform, 1, glm::value_ptr(Value));
}

template<>
void IShader::SetUniform(GLint Uniform, const glm::ivec4& Value)
{
    glUniform4iv(Uniform, 1, glm::value_ptr(Value));
}

template<>
void IShader::SetUniform(GLint Uniform, const Color& Value)
{
    glUniform4fv(Uniform, 1, &Value.R);
}

template<>
void IShader::SetUniform(GLint Uniform, const glm::mat2& Value)
{
    glUniformMatrix2fv(Uniform, 1, GL_FALSE, glm::value_ptr(Value));
}

template<>
void IShader::SetUniform(GLint Uniform, const glm::mat3& Value)
{
    glUniformMatrix3fv(Uniform, 1, GL_FALSE, glm::value_ptr(Value));
}

template<>
void IShader::SetUniform(GLint Uniform, const glm::mat4& Value)
{
    glUniformMatrix4fv(Uniform, 1, GL_FALSE, glm::value_ptr(Value));
}

void IShader::LoadShader(GLint Program)
{
}

void IShader::LinkToShaderFromSource(const char* Source, GLenum ShaderType)
{
    GLint Shader = glCreateShader(ShaderType);
    
    // Send the shader source to OpenGL and compile it
    glShaderSource(Shader, 1, &Source, NULL);
    glCompileShader(Shader);
    
    GLint CompileStatus;
    glGetShaderiv(Shader, GL_COMPILE_STATUS, &CompileStatus);
    
    if (CompileStatus == GL_TRUE)
    {
        // Compilation successful, carry on
        glAttachShader(m_Program, Shader);
        
        // This is OK, since the program now tracks the "Shader" object and
        // will not be deleted until it is detached with glDetachShader()
        glDeleteShader(Shader);
    }
    else
    {
        GLint LogBufferLength;
        glGetShaderiv(Shader, GL_INFO_LOG_LENGTH, &LogBufferLength);
        char* ShaderInfoLog = new char[LogBufferLength];
        
        //TODO: Show the ShaderLog in some clever way
        glGetShaderInfoLog(Shader, LogBufferLength, NULL, ShaderInfoLog);
        assert(false);
    }
}

void IShader::CompileShader()
{
    glLinkProgram(m_Program);
    
    GLint LinkStatus;
    glGetProgramiv(m_Program, GL_LINK_STATUS, &LinkStatus);
    
    if (LinkStatus == GL_TRUE)
    {
        // We have succesfully "compiled" (linked) the program
    }
    else
    {
        GLint LogBufferLength;
        glGetProgramiv(m_Program, GL_INFO_LOG_LENGTH, &LogBufferLength);
        char* ProgramInfoLog = new char[LogBufferLength];
        
        glGetProgramInfoLog(m_Program, LogBufferLength, NULL, ProgramInfoLog);
        
        //TODO: Show the ProgramLog in some clever way
        glDeleteProgram(m_Program);
        m_Program = 0;
        assert(false);
    }
}