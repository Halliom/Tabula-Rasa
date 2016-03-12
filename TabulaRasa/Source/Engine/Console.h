#pragma once

#include "../Rendering/TextRenderer.h"

#include "SDL2\SDL.h"

#define MAX_INPUT_LENGTH 128
#define MAX_COMMAND_BUFFER_SIZE 32

class Console
{
public:

	Console();

	~Console();

	void ReceiveTextInput(SDL_Keycode* KeyCode, bool IsShiftDown);

	void OnUpdateInputMode();

	bool m_bIsActive;

private:

	std::string m_CurrentlyTyping;

	TextRenderData2D* m_pCommandBuffer[MAX_COMMAND_BUFFER_SIZE];
	unsigned int m_BufferLength;

	TextRenderData2D* m_pActiveLineText;

	RectRenderData2D* m_pBackgroundRect;

};