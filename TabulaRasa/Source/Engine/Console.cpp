#include "Console.h"

#include "../Rendering/RenderingEngine.h"

extern RenderingEngine* g_RenderingEngine;

Console::Console() : 
	m_bIsActive(false),
	m_CurrentlyTyping(">"),
	m_BufferLength(0)
{
}

Console::~Console()
{
	if (m_pActiveLineText)
	{
		TextRenderer::RemoveText(m_pActiveLineText);
		TextRenderer::RemoveRect(m_pBackgroundRect);

		for (int i = 0; i < m_BufferLength; ++i)
		{
			TextRenderer::RemoveText(m_pCommandBuffer[i]);
		}
	}
}

void Console::ReceiveTextInput(SDL_Keycode* KeyCode, bool IsShiftDown)
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
				if (m_BufferLength < MAX_COMMAND_BUFFER_SIZE)
				{
					char* Command = new char[m_CurrentlyTyping.size() - 1]; // Remove the '>' character
					strcpy(Command, m_CurrentlyTyping.c_str() + 1); // Ignore the first '>' character

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

					m_pCommandBuffer[m_BufferLength++] = TextRenderer::AddTextToRender(
						m_CurrentlyTyping.substr(1, m_CurrentlyTyping.size() - 1).c_str(), // Skip the first character
						0.0f, 
						g_RenderingEngine->m_ScreenHeight / 2.0f - 24.0f - (20.0f * 1),
						16.0f);

					m_CurrentlyTyping.erase(++m_CurrentlyTyping.begin(), m_CurrentlyTyping.end());

					// Move everything else in the list up one
					for (int i = 0; i < m_BufferLength - 1; ++i)
					{
						m_pCommandBuffer[i]->Position -= glm::vec3(0.0f, 20.0f, 0.0f);
					}
				}
				// TODO: Send command
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
								}
								m_CurrentlyTyping.append(c);
							}
							else
							{
								m_CurrentlyTyping.append(Symbol);
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

		if (m_pActiveLineText)
			TextRenderer::RemoveText(m_pActiveLineText);

		m_pActiveLineText = TextRenderer::AddTextToRender(m_CurrentlyTyping.c_str(), 0.0f, g_RenderingEngine->m_ScreenHeight / 2.0f - 24.0f, 16.0f);
	}
}

void Console::OnUpdateInputMode()
{
	if (!m_bIsActive)
	{
		TextRenderer::RemoveText(m_pActiveLineText);
		TextRenderer::RemoveRect(m_pBackgroundRect);

		for (int i = 0; i < m_BufferLength; ++i)
		{
			TextRenderer::RemoveText(m_pCommandBuffer[i]);
		}
	}
	else
	{
		m_pActiveLineText = TextRenderer::AddTextToRender(m_CurrentlyTyping.c_str(), 0.0f, g_RenderingEngine->m_ScreenHeight / 2.0f - 24.0f, 16.0f);
		m_pBackgroundRect = TextRenderer::AddRectToRender(0.0f, 0.0f, g_RenderingEngine->m_ScreenWidth, g_RenderingEngine->m_ScreenHeight / 2.0f, glm::vec4(0.0f, 0.0f, 0.0f, 0.65f));
	}
}

void Console::Update()
{
}
