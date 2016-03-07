#include "GL_shader.h"

#include "../Platform/Platform.h"

GLShaderProgram::~GLShaderProgram()
{
	if (Program)
	{
		glDeleteProgram(Program);
	}
}

GLShaderProgram* GLShaderProgram::CreateVertexFragmentShader(std::string& VertexShaderSource, std::string& FragmentShaderSource)
{
	GLShaderProgram* Program = new GLShaderProgram();

	if (!Program->AttachShaderSource(GLShaderSource(VertexShaderSource, ShaderType::VERTEX)))
	{
		delete Program;
		return NULL;
	}

	if (!Program->AttachShaderSource(GLShaderSource(FragmentShaderSource, ShaderType::FRAGMENT)))
	{
		delete Program;
		return NULL;
	}

	if (!Program->CompileShaderProgram())
	{
		delete Program;
		return NULL;
	}

	Program->GenerateUniformBindings();

	return Program;
}

GLShaderProgram* GLShaderProgram::CreateVertexFragmentShaderFromFile(std::string& VertexShaderFilename, std::string& FragmentShaderFilename)
{
	return CreateVertexFragmentShader(
		PlatformFileSystem::LoadFile(DT_SHADERS, VertexShaderFilename), 
		PlatformFileSystem::LoadFile(DT_SHADERS, FragmentShaderFilename));
}

void GLShaderProgram::GenerateUniformBindings()
{
	ProjectionMatrixLocation =			GetUniform("g_ProjectionMatrix");
	ViewMatrixLocation =				GetUniform("g_ViewMatrix");
	ProjectionViewMatrixLocation =		GetUniform("g_ProjectionViewMatrix");
	ModelMatrixLocation =				GetUniform("g_WorldTransformMatrix");
	PositionOffsetLocation =			GetUniform("g_PositionOffset");

	TextureSamplers[0] = GetUniform("textureSampler0");
	TextureSamplers[1] = GetUniform("textureSampler1");
	TextureSamplers[2] = GetUniform("textureSampler2");
	TextureSamplers[3] = GetUniform("textureSampler3");
}

bool GLShaderProgram::AttachShaderSource(const GLShaderSource& Source)
{
	if (!Program)
	{
		// Create a program if one is not already created
		Program = glCreateProgram();
	}

	GLint Shader = glCreateShader(Source.ShaderType);

	// Create an array of size 1 to hold the shader source
	const char* SourceString[1] = { Source.Source.c_str() };

	glShaderSource(Shader, 1, SourceString, NULL);

	glCompileShader(Shader);

	GLint CompileStatus;
	glGetShaderiv(Shader, GL_COMPILE_STATUS, &CompileStatus);

	if (CompileStatus == GL_TRUE)
	{
		// Compilation successfull, carry on
		glAttachShader(Program, Shader);

		// This is OK, since the program now tracks the "Shader" object and
		// will not be deleted until it is detached with glDetachShader()
		glDeleteShader(Shader);
		return true;
	}
	else
	{
		GLint LogBufferLength;
		glGetShaderiv(Shader, GL_INFO_LOG_LENGTH, &LogBufferLength);
		char* ShaderInfoLog = new char[LogBufferLength];

		glGetShaderInfoLog(Shader, LogBufferLength, NULL, ShaderInfoLog);

		//TODO: Show the ShaderLog in some clever way
		return false;
	}
}

bool GLShaderProgram::AttachShaderSource(std::vector<GLShaderSource>& Sources)
{
	if (!Program)
	{
		// Create a program if one is not already created
		Program = glCreateProgram();
	}

	// Same as the other function but with a loop
	for (std::vector<GLShaderSource>::iterator ListIterator = Sources.begin(); ListIterator != Sources.end(); ++ListIterator)
	{
		GLint Shader = glCreateShader((*ListIterator).ShaderType);

		// Create an array of size 1 to hold the shader source
		const char* SourceString[1];
		SourceString[0] = (*ListIterator).Source.c_str();

		glShaderSource(Shader, 1, SourceString, NULL);

		glCompileShader(Shader);

		GLint CompileStatus;
		glGetShaderiv(Shader, GL_COMPILE_STATUS, &CompileStatus);

		if (CompileStatus == GL_TRUE)
		{
			// Compilation successfull, carry on
			glAttachShader(Program, Shader);

			// This is OK, since the program now tracks the "Shader" object and
			// will not be deleted until it is detached with glDetachShader()
			glDeleteShader(Shader);
		}
		else
		{
			GLint LogBufferLength;
			glGetShaderiv(Shader, GL_INFO_LOG_LENGTH, &LogBufferLength);
			char* ShaderInfoLog = new char[LogBufferLength];

			glGetShaderInfoLog(Shader, LogBufferLength, NULL, ShaderInfoLog);

			//TODO: Show the ShaderLog in some clever way
			return false;
		}
	}
	return true;
}

bool GLShaderProgram::CompileShaderProgram()
{
	if (Program)
	{
		glLinkProgram(Program);

		GLint LinkStatus;
		glGetProgramiv(Program, GL_LINK_STATUS, &LinkStatus);

		if (LinkStatus == GL_TRUE)
		{
			// We have succesfully "compiled" (linked) the program
			return true;
		}
		else 
		{
			GLint LogBufferLength;
			glGetProgramiv(Program, GL_INFO_LOG_LENGTH, &LogBufferLength);
			char* ProgramInfoLog = new char[LogBufferLength];

			glGetProgramInfoLog(Program, LogBufferLength, NULL, ProgramInfoLog);

			//TODO: Show the ProgramLog in some clever way
			glDeleteProgram(Program);
			Program = 0;
			return false;
		}
	}
	else
	{
		return false;
	}
}