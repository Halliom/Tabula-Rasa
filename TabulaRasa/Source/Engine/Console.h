#pragma once

#include "../Rendering/TextRenderer.h"

#include "SDL2\SDL.h"

#define MAX_INPUT_LENGTH 128

class Console
{
public:

	Console();

	~Console();

	void ReceiveTextInput(SDL_Keycode* KeyCode, bool IsShiftDown);

	void OnUpdateInputMode();

	bool m_bIsActive;

	void Update();

private:

	std::string m_CurrentlyTyping;

	TextRenderData2D* m_pActiveLineText;

};