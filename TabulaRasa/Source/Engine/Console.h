#pragma once

#include "../Rendering/TextRenderer.h"

#include "SDL2\SDL.h"

#define MAX_INPUT_LENGTH 128

class Console
{
public:

	Console();

	~Console();

	void Print(char* Message);

	void Print(std::string& Message);

	void PrintLine(char* Message);

	void PrintLine(std::string& Message);

	void ReceiveTextInput(SDL_Keycode* KeyCode, bool IsShiftDown, bool IsAltDown);

	void OnUpdateInputMode();

	void RedrawTextBuffer();

	char *ExecuteCommand(char *Command);

	bool m_bIsActive;

private:

	std::string m_CurrentlyTyping;

	std::string m_TextBuffer;

	TextRenderData2D* m_pTextBufferRenderData;

	TextRenderData2D* m_pActiveLineText;

	RectRenderData2D* m_pBackgroundRect;

};