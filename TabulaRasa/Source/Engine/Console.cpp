#include "Console.h"

#include "../Rendering/GUI/imgui/imgui.h"

#include "Input.h"

bool IsSingleWord(char* String)
{
	bool FoundWord = false;
	unsigned int i = 0;
	while (String[0] != '\0')
	{
		if (String[0] == ' ')
		{
			if (FoundWord)
				return false;
			FoundWord = true;
		}
		++String;
		++i;
	}
	if (FoundWord)
		return false;

	String -= i;
	return true;
}

Console::Console() : 
	m_bShowConsole(false),
	m_pConsoleTitle("Console"),
	m_NumConsoleMessages(0)
{
}

Console::~Console()
{
	Clear();
}

void Console::PrintLine(const char* Line, int Length, EConsoleMessageType Type)
{
	// If no length was specified, calculate it via strlen
	if (Length = -1)
	{
		Length = strlen(Line);
	}

	char* Message = new char[Length];
	strcpy(Message, Line);

	if (m_NumConsoleMessages < MAX_BUFFER_SIZE)
	{
		m_pMessageBuffer[m_NumConsoleMessages].Message = Message;
		m_pMessageBuffer[m_NumConsoleMessages].MessageType = Type;
		++m_NumConsoleMessages;
	}
	else // We have reached the maximum allowance of messages in the buffer
	{
		// Delete the first element and shuffle the rest downwards
		delete[] m_pMessageBuffer[0].Message;
		memcpy(m_pMessageBuffer, m_pMessageBuffer + 1, sizeof(ConsoleMessage) * (m_NumConsoleMessages - 1));
		--m_NumConsoleMessages;
	}
}

void Console::PrintLine(std::string& Line, EConsoleMessageType Type)
{
	PrintLine(Line.c_str(), Line.size(), Type);
}

void Console::SetTitle(const char* Title)
{
	m_pConsoleTitle = (char*)Title;
}

void Console::Clear()
{
	for (unsigned int i = 0; i < m_NumConsoleMessages; ++i)
	{
		delete[] m_pMessageBuffer[i].Message;
	}
	m_NumConsoleMessages = 0;
}

void Console::ShowConsole(bool Show)
{
	m_bShowConsole = Show;
}

char* Console::ExecuteCommand(const char* Command)
{
	return NULL;
}

void Console::Draw()
{
	if (m_bShowConsole) // Don't draw the console if we're not supposed to
	{
		// Sets the window size to 520x600, but only the first time
		ImGui::SetNextWindowSize(ImVec2(520, 600), ImGuiSetCond_Once);

		// Starts rendering the console, if the user shut down the console
		// by clicking the x this will return false and then the rendering
		// will stop
		if (!ImGui::Begin(m_pConsoleTitle, &m_bShowConsole))
		{
			ImGui::End();
			return;
		}

		// If the user pressed the "Clear" button on the console window this
		// frame, clear the console
		if (ImGui::Button("Clear"))
		{
			Clear();
		}

		// End drawmode
		ImGui::End();
	}
}
