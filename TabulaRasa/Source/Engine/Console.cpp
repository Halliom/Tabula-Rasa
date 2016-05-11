#include "Console.h"

#include <sstream>
#include <iostream>
#include <stdio.h>
#include <sys/types.h>

#include "../Engine/Core/Memory.h"
#include "../Rendering/RenderingEngine.h"

extern RenderingEngine* g_RenderingEngine;

Console::Console() :
	m_bIsActive(false),
	m_CurrentlyTyping(">"),
	m_LastTyped("")
{
	m_pTextBufferRenderData = NULL;
	m_pActiveLineText = NULL;
	m_pBackgroundRect = NULL;

	m_ConsoleFont = FontLibrary::g_FontLibrary->GetFont(1);
}

Console::~Console()
{
	// We actually don't need to dereference anything here
	// since the console only gets destroyed when the game 
	// closes and then the text/rect rendering has already
	// been destroyed
}

void Console::Print(char* Message)
{
	char a = 'a';
	m_TextBuffer.append(Message);
	RedrawTextBuffer();
}

void Console::Print(std::string& Message)
{
	m_TextBuffer.append(Message);
	RedrawTextBuffer();
}

void Console::PrintLine(char* Message)
{
	m_TextBuffer.append(Message);
	m_TextBuffer.append("\n");
	RedrawTextBuffer();
}

void Console::PrintLine(std::string& Message)
{
	m_TextBuffer.append(Message);
	m_TextBuffer.append("\n");
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
			case SDLK_UP:
			{
				m_CurrentlyTyping = std::string(">").append(m_LastTyped);
				break;
			}
			case SDLK_TAB:
			case SDLK_SPACE:
			{
				m_CurrentlyTyping.append(" ");
				break;
			}
			case SDLK_PERIOD:
			{
				if (IsShiftDown)
					m_CurrentlyTyping.append(":");
				else
					m_CurrentlyTyping.append(".");
				break;
			}
			case SDLK_COMMA:
			{
				if (IsShiftDown)
					m_CurrentlyTyping.append(";");
				else
					m_CurrentlyTyping.append(",");
				break;
			}
			case SDLK_MINUS:
			{
				if (IsShiftDown)
					m_CurrentlyTyping.append("_");
				else
					m_CurrentlyTyping.append("-");
				break;
			}
			case SDLK_RETURN:
			{
				// Remove the '>' character
				char* Command = AllocateWithType<char>(g_MemoryManager->m_pTransientFrameMemory, m_CurrentlyTyping.size() - 1);
				strcpy(Command, m_CurrentlyTyping.c_str() + 1);

				if (strcmp(Command, "clear") == 0)
				{
					m_TextBuffer.clear();
					RedrawTextBuffer();
					m_CurrentlyTyping.clear();
					m_CurrentlyTyping.append(">");
					break;
				}

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

				m_LastTyped = std::string(Command);

				m_TextBuffer.append(Command);
				m_TextBuffer.append("\n");

				char* Result = ExecuteCommand(Command);
				if (Result[0] != '\0')
				{
					// Add a new line and append the result string
					//m_TextBuffer[m_TextBuffer.size()] = '\n';
					m_TextBuffer.append(Result);
					m_TextBuffer.append("\n");
				}

				m_CurrentlyTyping.clear();
				m_CurrentlyTyping.append(">");

				RedrawTextBuffer();
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
									case SDLK_4: { c[0] = '¤'; break; }
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
								case SDLK_3: { c[0] = '£'; break; }
								case SDLK_4: { c[0] = '$'; break; }
								case SDLK_5: { c[0] = '€'; break; }
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

		m_pActiveLineText = TextRenderer::AddTextToRender(
			m_CurrentlyTyping.c_str(), 
			0.0f, 
			g_RenderingEngine->m_ScreenHeight / 2.0f - ((float) m_ConsoleFont.Size * 1.5f), 
			1,
			&m_ConsoleFont);
	}
}

void Console::OnUpdateInputMode()
{
	if (!m_bIsActive)
	{
		TextRenderer::RemoveText(m_pActiveLineText);
		TextRenderer::RemoveText(m_pTextBufferRenderData);
		m_pTextBufferRenderData = NULL;
		m_pActiveLineText = NULL;

		if (m_pBackgroundRect)
			TextRenderer::RemoveRect(m_pBackgroundRect);
	}
	else
	{
		m_pActiveLineText = TextRenderer::AddTextToRender(
			m_CurrentlyTyping.c_str(), 
			0.0f, 
			g_RenderingEngine->m_ScreenHeight / 2.0f - ((float) m_ConsoleFont.Size * 1.5f), 
			1,
			&m_ConsoleFont);

		m_pBackgroundRect = TextRenderer::AddRectToRender(
			0.0f, 
			0.0f, 
			(float) g_RenderingEngine->m_ScreenWidth, 
			(float) g_RenderingEngine->m_ScreenHeight / 2.0f, 
			glm::vec4(1.0f / 255.0f, 25.0f / 255.0f, 0.0f, 0.65f), 
			1);

		RedrawTextBuffer();
	}
}

void Console::RedrawTextBuffer()
{
	if (m_bIsActive)
	{
		TextRenderer::RemoveText(m_pTextBufferRenderData);
		if (m_TextBuffer.length() > MAX_TEXT_BUFFER_LENGTH)
		{
			size_t Overload = m_TextBuffer.length() - MAX_TEXT_BUFFER_LENGTH + BUFFER_CLEANUP;
			m_TextBuffer = m_TextBuffer.substr(Overload, MAX_TEXT_BUFFER_LENGTH - BUFFER_CLEANUP);
			m_TextBuffer.append("");
		}

		unsigned int NumLines = 0;
		for (char c : m_TextBuffer)
		{
			if (c == '\n')
				++NumLines;
		}



		m_pTextBufferRenderData = TextRenderer::AddTextToRenderWithColorAndLength(
			m_TextBuffer.c_str(),
			m_TextBuffer.length(),
			0.0f,
			(g_RenderingEngine->m_ScreenHeight / 2.0f) - (m_ConsoleFont.Size * NumLines) - ((float) m_ConsoleFont.Size * 1.5f),
			glm::vec4(1.0f),
			1,
			&m_ConsoleFont);
	}
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
	//g_ScriptEngine->ExecuteStringInInterpreter(Command);

	return "";
}
