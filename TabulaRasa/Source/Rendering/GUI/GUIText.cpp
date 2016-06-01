#include "GUIText.h"

GUIText::GUIText(IGUIElement* Parent) : 
	IGUIElement(Parent),
	m_pDisplayString("")
{
}

void GUIText::SetText(char* DisplayString)
{
	m_pDisplayString = DisplayString;

	// TODO: delete the renderable
	m_Renderable = m_pRenderer->CreateText(m_pDisplayString, strlen(m_pDisplayString), m_Dimensions);
	m_BackgroundRect = m_pRenderer->CreateRect(glm::ivec2(100, 100), Color::RED);
}

void GUIText::Initialize(GUIRenderer* Renderer)
{
	IGUIElement::Initialize(Renderer);
	m_Renderable = m_pRenderer->CreateText(m_pDisplayString, strlen(m_pDisplayString), m_Dimensions);
}

void GUIText::Render()
{
	m_pRenderer->RenderAtPosition(m_Renderable, GetPosition());
	m_pRenderer->RenderAtPosition(m_BackgroundRect, GetPosition());
}

GUIText::~GUIText()
{
	// TODO: delete the renderable

	delete[] m_pDisplayString;
}