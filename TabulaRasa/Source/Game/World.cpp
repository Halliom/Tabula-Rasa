#include "World.h"

#include "glm\common.hpp"
#include "glm\gtc\matrix_transform.hpp"
#include "../Rendering/TextRender.h"

struct Vertex
{
	Vertex(glm::vec3 Vert, glm::vec4 Color) :
		Vert(Vert), Color(Color)
	{}
	glm::vec3 Vert;
	glm::vec4 Color;
};

World::World()
{
	Shader = GLShaderProgram::CreateVertexFragmentShaderFromFile(std::string("VertexShader.glsl"), std::string("FragmentShader.glsl"));
	CurrentPlayer = new Player();

	TextRender::Initialize2DTextRendering();
	TextRender::AddTextToRender("Hello World", 36.0f);

	/*for (int i = 0; i < 16; ++i)
	{
		for (int j = 0; j < 16; ++j)
		{
			for (int k = 0; k < 16; ++k)
			{
				Chunk.InsertNode(glm::uvec3(i, j, k), new Voxel());
			}
		}
	}*/
}

World::~World()
{
	TextRender::Destroy2DTextRendering();

	if (Shader)
	{
		delete Shader;
	}
	if (CurrentPlayer)
	{
		delete CurrentPlayer;
	}
}

void World::Update(float DeltaTime)
{
	TextRender::Render();

	static float Angle = 0;
	Angle += DeltaTime;

	CurrentPlayer->Update(DeltaTime);

	/*Shader->Bind();

	glm::mat4 Projection = glm::perspective(glm::radians(70.0f), 4.0f / 3.0f, 0.1f, 100.f);
	glm::mat4 View = CurrentPlayer->GetViewMatrix();
	glm::mat4 Model = glm::mat4(1.0f);
	
	Shader->SetProjectionMatrix(Projection);
	Shader->SetViewMatrixLocation(View);
	Shader->SetModelMatrix(Model);

	Chunk.Render();*/
}
