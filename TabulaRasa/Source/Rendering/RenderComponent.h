#pragma once

#include "GL\glew.h"

class RenderComponent
{
public:
	RenderComponent();

	virtual ~RenderComponent();

	virtual void Render(float DeltaTime);

};