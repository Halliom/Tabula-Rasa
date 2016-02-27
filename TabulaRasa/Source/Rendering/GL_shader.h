#ifndef __GL_SHADER_H__
#define __GL_SHADER_H__

#include <string>
#include <vector>

#include "GL\glew.h"
#include "glm\common.hpp"
#include "glm\gtc\type_ptr.hpp"

enum ShaderType
{
	VERTEX = GL_VERTEX_SHADER,
	FRAGMENT = GL_FRAGMENT_SHADER,
#if defined(GL_VERSION_4_3) || defined(GL_VERSION_4_4) || defined(GL_VERSION_4_5)
	COMPUTE = GL_COMPUTE_SHADER,
#endif
	TESSELATE_CONTROL = GL_TESS_CONTROL_SHADER,
	TESSELATE_EVAL = GL_TESS_EVALUATION_SHADER,
	GEOMETRY = GL_GEOMETRY_SHADER
};

struct GLShaderSource
{
	GLShaderSource(std::string& Source, GLint& ShaderType) : Source(Source), ShaderType(ShaderType) {}
	GLShaderSource(std::string& Source, ShaderType ShaderType) : Source(Source), ShaderType(static_cast<GLint>(ShaderType)) {}
	std::string Source;
	GLint		ShaderType;
};

class GLShaderProgram
{
public:
	GLShaderProgram() : Program(0) {}
	~GLShaderProgram();

	static GLShaderProgram* CreateVertexFragmentShader(std::string& VertexShaderSource, std::string& FragmentShaderSource);

	static GLShaderProgram* CreateVertexFragmentShaderFromFile(std::string& VertexShaderFilename, std::string& FragmentShaderFilename);

	void GenerateUniformBindings();

	__forceinline GLint GetUniform(const char* UniformName)
	{
		return glGetUniformLocation(Program, UniformName);
	}

	bool AttachShaderSource(const GLShaderSource& Source);

	bool AttachShaderSource(std::vector<GLShaderSource>& Sources);

	bool CompileShaderProgram();

	__forceinline void Bind() { glUseProgram(Program); }

	__forceinline void Unbind() { glUseProgram(0); }

	__forceinline void SetProjectionMatrix(const glm::mat4& ProjectionMatrix)
	{
		glUniformMatrix4fv(ProjectionMatrixLocation, 1, GL_FALSE, glm::value_ptr(ProjectionMatrix));
	}
	
	__forceinline void SetModelMatrix(const glm::mat4& ModelMatrix)
	{
		glUniformMatrix4fv(ModelMatrixLocation, 1, GL_FALSE, glm::value_ptr(ModelMatrix));
	}

	__forceinline void SetViewMatrixLocation(const glm::mat4& ViewMatrix)
	{
		glUniformMatrix4fv(ViewMatrixLocation, 1, GL_FALSE, glm::value_ptr(ViewMatrix));
	}

private:

	GLint Program;

	GLint ProjectionMatrixLocation;

	GLint ViewMatrixLocation;

	GLint ModelMatrixLocation;

	GLint TextureSamplers[4];
};

#endif