#pragma once

#include "../Rendering/TextRenderer.h"

#include "SDL2\SDL.h"

#define MAX_INPUT_LENGTH 128
#define MAX_TEXT_BUFFER_LENGTH 1024
#define BUFFER_CLEANUP 64

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

	std::string m_LastTyped;

	TrueTypeFont m_ConsoleFont;

	TextRenderData2D* m_pTextBufferRenderData;

	TextRenderData2D* m_pActiveLineText;

	RectRenderData2D* m_pBackgroundRect;

};

extern Console* g_Console;

#define LogLn(str) g_Console->PrintLine(str);
#define Log(str) g_Console->Print(str);
