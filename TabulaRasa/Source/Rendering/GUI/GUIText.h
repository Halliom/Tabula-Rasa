#pragma once

#include "../GuiSystem.h"

class GUIText : public IGUIElement
{
public:

	GUIText(IGUIElement* Parent);

	~GUIText();

	void SetText(char* DisplayString);

	virtual void Initialize(GUIRenderer* Renderer) override;

	virtual void Render() override;

private:

	char* m_pDisplayString;

	GUIRenderable m_Renderable;

	GUIRenderable m_BackgroundRect;

};