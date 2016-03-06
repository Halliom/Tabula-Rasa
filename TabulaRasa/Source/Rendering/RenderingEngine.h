#pragma once

#include "GL\glew.h"

#include "../Game/World.h"

class RenderingEngine
{
public:

	~RenderingEngine();

	void Initialize(const bool& UseDepthTest);

	void RenderFrame(World* RenderWorld, const float& DeltaTime);

};