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
	memset(m_pConsoleInputLine, 0, MAX_INPUT_LINE_SIZE);
}

Console::~Console()
{
	Clear();
}

void Console::PrintLine(const char* Line, EConsoleMessageType Type, int Length)
{
	// If no length was specified, calculate it via strlen
	if (Length == -1)
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
	PrintLine(Line.c_str(), Type, Line.size());
}

void Console::PrintLineF(const char* Format, ...)
{
	char Buffer[1024];
	va_list Arguments;
	va_start(Arguments, Format);
	vsnprintf(Buffer, 1024, Format, Arguments);
	Buffer[1024 - 1] = 0;
	va_end(Arguments);
	PrintLine(Buffer);
}

void Console::SetTitle(const char* Title)
{
	m_pConsoleTitle = (char*)Title;
}

void Console::Clear()
{
	for (unsigned int i = 0; i < m_NumConsoleMessages; ++i)
	{
		//delete m_pMessageBuffer[i].Message;
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

		// Begins a child region within the window for the output
		ImGui::BeginChild("ScrollingRegion", ImVec2(0, -ImGui::GetItemsLineHeightWithSpacing()), true, ImGuiWindowFlags_HorizontalScrollbar);
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 1));
		for (int i = 0; i < m_NumConsoleMessages; ++i)
		{
			ImVec4 TextColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
			switch (m_pMessageBuffer[i].MessageType)
			{
				case MESSAGE_TYPE_WARNING:
				{
					// Warning yello (255, 204, 0)
					TextColor = ImVec4(1.0f, 204.0f / 255.0f, 0.0f, 1.0f);
					break;
				}
				case MESSAGE_TYPE_ERROR:
				{
					// Error red (204, 0, 0)
					TextColor = ImVec4(204.0f / 255.0f, 0.0f, 0.0f, 1.0f);
					break;
				}
				case MESSAGE_TYPE_INFO:
				{
					// Info purple (128, 0, 128)
					TextColor = ImVec4(0.6f, 0.0f, 0.6f, 1.0f);
					break;
				}
			}

			// Set the color (and then pop it) and render the text
			ImGui::PushStyleColor(ImGuiCol_Text, TextColor);
			ImGui::TextUnformatted(m_pMessageBuffer[i].Message);
			ImGui::PopStyleColor();
		}
		ImGui::PopStyleVar(); // Pops the ItemSpacing var
		ImGui::EndChild();

		// Just a line
		ImGui::Separator();

		ImGui::SetKeyboardFocusHere();
		if (ImGui::InputText("", m_pConsoleInputLine, MAX_INPUT_LINE_SIZE, ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_NoHorizontalScroll))
		{
			PrintLine(m_pConsoleInputLine, MESSAGE_TYPE_NORMAL);
			strcpy(m_pConsoleInputLine, "");
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
