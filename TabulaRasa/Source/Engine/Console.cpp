#include "Console.h"

#include <sstream>
#include <iostream>
#include <stdio.h>
#include <sys/types.h>

#include "../Rendering/RenderingEngine.h"
#include "../Engine/PythonScript.h"

extern RenderingEngine* g_RenderingEngine;
extern PythonScriptEngine* g_ScriptEngine;

Console::Console() :
	m_bIsActive(false),
	m_CurrentlyTyping(">")
{
	m_pTextBufferRenderData = TextRenderer::AddTextToRender("");
	m_pActiveLineText = TextRenderer::AddTextToRender("");
}

Console::~Console()
{
	TextRenderer::RemoveText(m_pActiveLineText);
	TextRenderer::RemoveText(m_pTextBufferRenderData);

	TextRenderer::RemoveRect(m_pBackgroundRect);
}

void Console::PrintMessage(char* Message)
{
	m_TextBuffer.append(Message);
	RedrawTextBuffer();
}

void Console::PrintMessage(std::string& Message)
{
	m_TextBuffer.append(Message);
	RedrawTextBuffer();
}

void Console::ReceiveTextInput(SDL_Keycode* KeyCode, bool IsShiftDown, bool IsAltDown)
{
	if (m_bIsActive)
	{
		switch (*KeyCode)
		{
			case SDLK_BACKSPACE:
			{
				if (m_CurrentlyTyping.size() > 1) // Don't remove first character since it is the '>'
				{
					m_CurrentlyTyping.pop_back();
				}
				break;
			}
			case SDLK_TAB:
			case SDLK_SPACE:
			{
				m_CurrentlyTyping.append(" ");
				break;
			}
			case SDLK_RETURN:
			{
				// Remove the '>' character
				char* Command = new char[m_CurrentlyTyping.size() - 1]; 
				strcpy(Command, m_CurrentlyTyping.c_str() + 1);

				bool OnlyWhiteSpace = true;
				for (int i = 0; *(Command + i); ++i)
				{
					if (!isspace(*(Command + i)))
					{
						OnlyWhiteSpace = false;
						break;
					}
				}

				if (OnlyWhiteSpace)
				{
					return;
				}

				char* Result = ExecuteCommand(Command);
				if (Result[0] != '\0')
				{
					// Add a new line and append the result string
					//m_TextBuffer[m_TextBuffer.size()] = '\n';
					m_TextBuffer.append(Result);
					m_TextBuffer.append("\n");
					RedrawTextBuffer();
				}

				m_CurrentlyTyping.erase(++m_CurrentlyTyping.begin(), m_CurrentlyTyping.end());

				break;
			}
			default:
			{
				if (*KeyCode <= SDLK_z)
				{
					const char* Symbol = SDL_GetKeyName(*KeyCode);
					if (isdigit(Symbol[0]))
					{
						if (*KeyCode <= SDLK_9) // Not a numpad key
						{
							if (IsShiftDown)
							{
								char c[2] = " ";
								switch (*KeyCode)
								{
									case SDLK_0: { c[0] = '='; break; }
									case SDLK_1: { c[0] = '!'; break; }
									case SDLK_2: { c[0] = '"'; break; }
									case SDLK_3: { c[0] = '#'; break; }
									case SDLK_4: { c[0] = '�'; break; }
									case SDLK_5: { c[0] = '%'; break; }
									case SDLK_6: { c[0] = '&'; break; }
									case SDLK_7: { c[0] = '/'; break; }
									case SDLK_8: { c[0] = '('; break; }
									case SDLK_9: { c[0] = ')'; break; }
									case SDLK_PLUS: { c[0] = '?'; break; }
								}
								m_CurrentlyTyping.append(c);
								break;
							}
							else if (IsAltDown)
							{
								char c[2] = " ";
								switch (*KeyCode)
								{
								case SDLK_0: { c[0] = '}'; break; }
								case SDLK_1: { c[0] = '1'; break; }
								case SDLK_2: { c[0] = '@'; break; }
								case SDLK_3: { c[0] = '�'; break; }
								case SDLK_4: { c[0] = '$'; break; }
								case SDLK_5: { c[0] = '�'; break; }
								case SDLK_6: { c[0] = '6'; break; }
								case SDLK_7: { c[0] = '{'; break; }
								case SDLK_8: { c[0] = '['; break; }
								case SDLK_9: { c[0] = ']'; break; }
								case SDLK_PLUS: { c[0] = '\\'; break; }
								}
								m_CurrentlyTyping.append(c);
								break;
							}
							else
							{
								m_CurrentlyTyping.append(Symbol);
								break;
							}
						}
					}
					else
					{
						if (IsShiftDown)
						{
							m_CurrentlyTyping.append(Symbol);
						}
						else
						{
							char AddBuffer[2];
							AddBuffer[0] = tolower(Symbol[0]);
							AddBuffer[1] = '\0'; // You NEED to null terminate
							m_CurrentlyTyping.append(AddBuffer);
						}
					}
				}
			}
		}

		TextRenderer::RemoveText(m_pActiveLineText);

		m_pActiveLineText = TextRenderer::AddTextToRender(m_CurrentlyTyping.c_str(), 0.0f, g_RenderingEngine->m_ScreenHeight / 2.0f - 24.0f, 16.0f);
	}
}

void Console::OnUpdateInputMode()
{
	if (!m_bIsActive)
	{
		TextRenderer::RemoveText(m_pActiveLineText);
		TextRenderer::RemoveText(m_pTextBufferRenderData);

		TextRenderer::RemoveRect(m_pBackgroundRect);
	}
	else
	{
		m_pActiveLineText = TextRenderer::AddTextToRender(m_CurrentlyTyping.c_str(), 0.0f, g_RenderingEngine->m_ScreenHeight / 2.0f - 24.0f, 16.0f);
		m_pBackgroundRect = TextRenderer::AddRectToRender(
			0.0f, 
			0.0f, 
			(float) g_RenderingEngine->m_ScreenWidth, 
			(float) g_RenderingEngine->m_ScreenHeight / 2.0f, 
			glm::vec4(1.0f / 255.0f, 25.0f / 255.0f, 0.0f, 0.65f));

		RedrawTextBuffer();
	}
}

void Console::RedrawTextBuffer()
{
	TextRenderer::RemoveText(m_pTextBufferRenderData);
	unsigned int NumLines = 0; // Always start with one line since we need to skip the input line
	for (char c : m_TextBuffer)
	{
		if (c == '\n')
			++NumLines;
	}

	m_pTextBufferRenderData = TextRenderer::AddTextToRender(
		m_TextBuffer.c_str(),
		0.0f,
		(g_RenderingEngine->m_ScreenHeight / 2.0f) - (16.0f * NumLines) - 24.0f,
		16.0f);
}

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

char* Console::ExecuteCommand(char* Command)
{
	//if (IsSingleWord(Command))
	//{
	//	return g_ScriptEngine->GetVariableValue(Command);
	//}
	g_ScriptEngine->ExecuteStringInInterpreter(Command);

	return "";
}
