#include "Shader.h"

#include "../Platform/Platform.h"

GLShaderProgram::~GLShaderProgram()
{
	if (Program)
	{
		glDeleteProgram(Program);
	}
}

GLShaderProgram* GLShaderProgram::CreateVertexFragmentShader(const std::string& VertexShaderSource, const std::string& FragmentShaderSource, bool IsSSAOShader)
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

	Program->m_bIsSSAOShader = IsSSAOShader;

    // Bind the shader so we can generate the uniform bindings
    Program->Bind();
	Program->GenerateUniformBindings();

	return Program;
}

GLShaderProgram* GLShaderProgram::CreateVertexFragmentShaderFromFile(const std::string& VertexShaderFilename, const std::string& FragmentShaderFilename, bool IsSSAOShader)
{
	return CreateVertexFragmentShader(
		PlatformFileSystem::LoadFile(DT_SHADERS, VertexShaderFilename),
		PlatformFileSystem::LoadFile(DT_SHADERS, FragmentShaderFilename), IsSSAOShader);
}

void GLShaderProgram::GenerateUniformBindings()
{
	ColorVectorLocation =				GetUniform("g_Color");
	ProjectionMatrixLocation =			GetUniform("g_ProjectionMatrix");
	ViewMatrixLocation =				GetUniform("g_ViewMatrix");
	ProjectionViewMatrixLocation =		GetUniform("g_ProjectionViewMatrix");
	ModelMatrixLocation =				GetUniform("g_ModelMatrix");
	PositionOffsetLocation =			GetUniform("g_PositionOffset");
	ScreenDimensionLocation =			GetUniform("ScreenDimension");

	TextureSamplers[0] = GetUniform("textureSampler0");
	TextureSamplers[1] = GetUniform("textureSampler1");
	TextureSamplers[2] = GetUniform("textureSampler2");
	TextureSamplers[3] = GetUniform("textureSampler3");
	TextureSamplers[4] = GetUniform("textureSampler4");
	TextureSamplers[5] = GetUniform("textureSampler5");

	if (m_bIsSSAOShader)
	{
		SSAOSamplesLocation = GetUniform("g_Samples");
	}
    
    SetDefaultSamplers();
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

void GLShaderProgram::SetDefaultSamplers()
{
	if (TextureSamplers[0] != -1)
		glUniform1i(TextureSamplers[0], 0);
	if (TextureSamplers[1] != -1)
		glUniform1i(TextureSamplers[1], 1);
	if (TextureSamplers[2] != -1)
		glUniform1i(TextureSamplers[2], 2);
	if (TextureSamplers[3] != -1)
		glUniform1i(TextureSamplers[3], 3);
	if (TextureSamplers[4] != -1)
		glUniform1i(TextureSamplers[4], 4);
	if (TextureSamplers[5] != -1)
		glUniform1i(TextureSamplers[5], 5);
}
