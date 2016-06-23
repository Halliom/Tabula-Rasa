#pragma once

#include <string>
#include <vector>

#ifdef _WIN32
#include "GL/glew.h"
#elif __APPLE__
#include <OpenGL/gl3.h>
#endif
#include "glm/common.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "GuiSystem.h"
#include "../Engine/Engine.h"

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
	GLShaderSource(const std::string& Source, GLint& ShaderType) : Source(Source), ShaderType(ShaderType) {}
	GLShaderSource(const std::string& Source, ShaderType ShaderType) : Source(Source), ShaderType(static_cast<GLint>(ShaderType)) {}
	std::string Source;
	GLint		ShaderType;
};

class GLShaderProgram
{
public:
	GLShaderProgram() : Program(0) {}
	~GLShaderProgram();

	static GLShaderProgram* CreateVertexFragmentShader(const std::string& VertexShaderSource, const std::string& FragmentShaderSource, bool IsSSAOShader = false);

	static GLShaderProgram* CreateVertexFragmentShaderFromFile(const std::string& VertexShaderFilename, const std::string& FragmentShaderFilename, bool IsSSAOShader = false);

	void GenerateUniformBindings();

	FORCEINLINE GLint GetUniform(const char* UniformName)
	{
		return glGetUniformLocation(Program, UniformName);
	}

	bool AttachShaderSource(const GLShaderSource& Source);

	bool AttachShaderSource(std::vector<GLShaderSource>& Sources);

	bool CompileShaderProgram();

	FORCEINLINE void Bind() { glUseProgram(Program); }

	FORCEINLINE void Unbind() { glUseProgram(0); }

	void SetDefaultSamplers();

	FORCEINLINE void SetColor(const glm::vec4& Color)
	{
		glUniform4fv(ColorVectorLocation, 1, glm::value_ptr(Color));
	}

	FORCEINLINE void SetColor(const Color& Color)
	{
		glUniform4fv(ColorVectorLocation, 1, &Color.R);
	}

	FORCEINLINE void SetProjectionMatrix(const glm::mat4& ProjectionMatrix)
	{
		glUniformMatrix4fv(ProjectionMatrixLocation, 1, GL_FALSE, glm::value_ptr(ProjectionMatrix));
	}

	FORCEINLINE void SetModelMatrix(const glm::mat4& ModelMatrix)
	{
		glUniformMatrix4fv(ModelMatrixLocation, 1, GL_FALSE, glm::value_ptr(ModelMatrix));
	}

	FORCEINLINE void SetViewMatrix(const glm::mat4& ViewMatrix)
	{
		glUniformMatrix4fv(ViewMatrixLocation, 1, GL_FALSE, glm::value_ptr(ViewMatrix));
	}

	FORCEINLINE void SetPositionOffset(const glm::vec3& PositionOffset)
	{
		glUniform3fv(PositionOffsetLocation, 1, glm::value_ptr(PositionOffset));
	}

	FORCEINLINE void SetProjectionViewMatrix(const glm::mat4& ViewProjectionMatrix)
	{
		glUniformMatrix4fv(ProjectionViewMatrixLocation, 1, GL_FALSE, glm::value_ptr(ViewProjectionMatrix));
	}

	FORCEINLINE void SetSSAOSamples(float Samples[192]) // 64 * 3 = 192
	{
		glUniform3fv(SSAOSamplesLocation, 64, Samples);
	}

	FORCEINLINE void SetScreenDimension(const glm::vec2& ScreenDimension)
	{
		glUniform2fv(ScreenDimensionLocation, 1, glm::value_ptr(ScreenDimension));
	}

	bool m_bIsSSAOShader;

	GLint Program;

private:

	GLint ColorVectorLocation;

	GLint ProjectionMatrixLocation;

	GLint ViewMatrixLocation;

	GLint ProjectionViewMatrixLocation;

	GLint ModelMatrixLocation;

	GLint PositionOffsetLocation;

	GLint TextureSamplers[6];

	GLint SSAOSamplesLocation;

	GLint ScreenDimensionLocation;
};
