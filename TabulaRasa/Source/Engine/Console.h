#pragma once

#include <string>
#include <stdarg.h>

#include "../Engine/Engine.h"

#define MAX_INPUT_LINE_SIZE 140
#define MAX_BUFFER_SIZE 200

enum EConsoleMessageType
{
	MESSAGE_TYPE_NORMAL,
	MESSAGE_TYPE_WARNING,
	MESSAGE_TYPE_ERROR,
	MESSAGE_TYPE_INFO
};

struct ConsoleMessage
{
	EConsoleMessageType MessageType;
	const char*			Message;
};

class Console
{
public:

	Console();

	~Console();

	/**
	 * Prints a line to the console
	 */
	void PrintLine(const char* Message, EConsoleMessageType Type = EConsoleMessageType::MESSAGE_TYPE_NORMAL, int Length = -1);
	void PrintLine(std::string& Message, EConsoleMessageType Type = EConsoleMessageType::MESSAGE_TYPE_NORMAL);

	void PrintLineF(const char* Format, ...);

	/**
	 * Sets the title of the console window
	 */
	void SetTitle(const char* Title);

	// Clears the buffer in the console
	void Clear();

	// Toggles the console to be visible or not by setting the argument Show
	// to true or false, this can also be toggled internally by the console
	void ShowConsole(bool Show = true);

	char* ExecuteCommand(const char *Command);

	// Draws the entire console, if the console is shown
	void Draw();

	bool			m_bShowConsole;

private:

	char			m_pConsoleInputLine[MAX_INPUT_LINE_SIZE];
	char*			m_pConsoleTitle;

	ConsoleMessage	m_pMessageBuffer[MAX_BUFFER_SIZE];
	unsigned int	m_NumConsoleMessages;
};

#define Log(str) g_Engine->g_Console->PrintLine(str)
#define LogF(fmt, ...) g_Engine->g_Console->PrintLineF(fmt, __VA_ARGS__)
