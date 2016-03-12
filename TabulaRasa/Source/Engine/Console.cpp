#include "Console.h"

Console::Console() : 
	m_bIsActive(false)
{
}

Console::~Console()
{
	if (m_pActiveLineText)
	{
		TextRenderer::RemoveText(m_pActiveLineText);
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
				if (m_CurrentlyTyping.size() > 0)
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
				// TODO: Send command
				break;
			}
			default:
			{
				if (*KeyCode <= SDLK_z)
				{
					const char* Symbol = SDL_GetKeyName(*KeyCode);
					if (IsShiftDown)
					{
						m_CurrentlyTyping.append(Symbol);
					}
					else
					{
						char AddBuffer[2];
						AddBuffer[0] = tolower(Symbol[0]);
						isalnum();
						AddBuffer[1] = '\0'; // You NEED to null terminate
						m_CurrentlyTyping.append(AddBuffer);
					}
				}
			}
		}

		if (m_pActiveLineText)
			TextRenderer::RemoveText(m_pActiveLineText);

		m_pActiveLineText = TextRenderer::AddTextToRender(m_CurrentlyTyping.c_str(), 100.0f, 100.0f);
	}
}

void Console::OnUpdateInputMode()
{
	if (!m_bIsActive)
	{
		TextRenderer::RemoveText(m_pActiveLineText);
	}
}

void Console::Update()
{
}
