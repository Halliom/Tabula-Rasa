#include "Console.h"

#include "../Rendering/GUI/imgui/imgui.h"

#include "../Engine/Input.h"
#include "../Engine/ScriptEngine.h"

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
    m_bDisplayScriptsWindow(false),
    m_pSelectedScriptItems(NULL),
    m_NumSelectedScriptItems(0),
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
#ifdef __APPLE__
        // Set smaller window to deal with Retina displays on Macs
        ImGui::SetNextWindowSize(ImVec2(480, 320), ImGuiSetCond_Once);
#else
		// Sets the window size to 520x600, but only the first time
		ImGui::SetNextWindowSize(ImVec2(520, 600), ImGuiSetCond_Once);
#endif

		// Starts rendering the console, if the user shut down the console
		// by clicking the x this will return false and then the rendering
		// will stop
		if (!ImGui::Begin(m_pConsoleTitle, &m_bShowConsole, ImGuiWindowFlags_MenuBar))
		{
			ImGui::End();
			return;
		}
        
        if (ImGui::BeginMenuBar())
        {
            if (ImGui::MenuItem("Scripts"))
            {
                m_bDisplayScriptsWindow = true;
            }
            ImGui::EndMenuBar();
        }

		// Begins a child region within the window for the output
		ImGui::BeginChild("ScrollingRegion", ImVec2(0, -ImGui::GetItemsLineHeightWithSpacing()), true, ImGuiWindowFlags_HorizontalScrollbar);
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 1));
		for (unsigned int i = 0; i < m_NumConsoleMessages; ++i)
		{
			ImVec4 TextColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
			switch (m_pMessageBuffer[i].MessageType)
			{
				case MESSAGE_TYPE_WARNING:
				{
					// Warning yellow (255, 204, 0)
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
                default:
                {
                    // If none of the above it must be normal which means white color
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

		//ImGui::SetKeyboardFocusHere();
		if (ImGui::InputText("", m_pConsoleInputLine, MAX_INPUT_LINE_SIZE, ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_NoHorizontalScroll))
		{
			PrintLine(m_pConsoleInputLine, MESSAGE_TYPE_NORMAL);

            g_Engine->g_ScriptEngine->ExecuteInInterpreter(m_pConsoleInputLine);

			// Clear the input line
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
    
    if (m_bDisplayScriptsWindow)
    {
        if (!ImGui::Begin("Script Manager", &m_bDisplayScriptsWindow, ImGuiWindowFlags_MenuBar))
        {
            ImGui::End();
            return;
        }
        
        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("New"))
                {
                    
                }
                if (ImGui::MenuItem("Open"))
                {
                    
                }
                if (ImGui::MenuItem("Save"))
                {
                    
                }
                if (ImGui::MenuItem("ReloadScripts"))
                {
                    Log("Reloading Scripts");
                    g_Engine->g_ScriptEngine->ReloadAllScripts();
                }
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }
        
        ImGui::BeginChild("Script list", ImVec2(0, -ImGui::GetItemsLineHeightWithSpacing()), true, ImGuiWindowFlags_HorizontalScrollbar);
        {
            if (m_NumSelectedScriptItems != g_Engine->g_ScriptEngine->m_LoadedScripts.size())
            {
                m_NumSelectedScriptItems = g_Engine->g_ScriptEngine->m_LoadedScripts.size();
                
                if (m_pSelectedScriptItems)
                    delete[] m_pSelectedScriptItems;
                
                m_pSelectedScriptItems = new bool[m_NumSelectedScriptItems];
            }
            auto It = g_Engine->g_ScriptEngine->m_LoadedScripts.begin();
            for (int i = 0;
                 i < g_Engine->g_ScriptEngine->m_LoadedScripts.size();
                 ++i)
            {
                const char* ScriptName = (*It).c_str();
                ImGui::Selectable(ScriptName, &m_pSelectedScriptItems[i]);
                It++;
            }
        }
        ImGui::EndChild();
        
        if (ImGui::Button("Reload selected"))
        {
            
        }
        
        ImGui::End();
    }
}
