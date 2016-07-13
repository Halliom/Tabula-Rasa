#include "GuiSystem.h"

#include <algorithm>

#ifdef _WIN32
#include "SDL2/SDL.h"
#include "SDL2/SDL_syswm.h"
#elif __APPLE__
#include "SDL2OSX/SDL.h"
#include "SDL2OSX/SDL_syswm.h"
#endif

#include "glm/common.hpp"
#include "GL/glew.h"

#include "GUI/imgui/imgui.h"

#include "Shader.h"
#include "Texture.h"

#include "../Platform/Platform.h"

#include "../Engine/Engine.h"
#include "../Engine/Input.h"
#include "../Engine/Core/Memory.h"
#include "../Engine/Console.h"

#include "../Game/Player.h"
#include "../Game/World.h"

#include "../Engine/Noise.h"

/**
 * Predefined color values
 */
const Color Color::BLACK	= Color(0x000000);
const Color Color::WHITE	= Color(0xFFFFFF);
const Color Color::GRAY		= Color(0xD3D3D3);

const Color Color::RED		= Color(0xFF0000);
const Color Color::GREEN	= Color(0x008000);
const Color Color::BLUE		= Color(0x3232FF);
const Color Color::YELLOW	= Color(0xFFFF00);
const Color Color::PINK		= Color(0xFFC0CB);
const Color Color::ORANGE	= Color(0xFFA500);
const Color Color::PURPLE	= Color(0x800080);
const Color Color::CYAN		= Color(0x00FFFF);
const Color Color::BROWN	= Color(0x87421F);

GUIRenderable GUIRenderer::CreateText(const char* Text, size_t StringLength, glm::ivec2& OutDimensions, Color Color, unsigned int Layer, TrueTypeFont* Font)
{
	GUIRenderable Result;

	TrueTypeFont FontToUse;
	if (Font == NULL)
	{
		FontToUse = g_Engine->g_FontLibrary->GetFont(0);
	}
	else
	{
		FontToUse = *Font;
	}

	glGenVertexArrays(1, &Result.VAO);
	glBindVertexArray(Result.VAO);

	glGenBuffers(1, &Result.VBO);
	glGenBuffers(1, &Result.IBO);

	List<float> Vertices = List<float>(g_Engine->g_MemoryManager->m_pGameMemory);
	Vertices.Reserve(StringLength * 20);
	List<unsigned short> Indices = List<unsigned short>(g_Engine->g_MemoryManager->m_pGameMemory);
	Indices.Reserve(StringLength * 6);
	unsigned int NumGlyphs = 0;

	int MaxWidth = 0;
	int MaxHeight = 0;

	float OffsetX = 0.0f;
	float OffsetY = FontToUse.Size;
	for (unsigned int CurrentChar = 0; CurrentChar < StringLength; ++CurrentChar)
	{
		if (Text[CurrentChar] == '\n')
		{
			OffsetY += FontToUse.Size;

            MaxWidth = glm::max(MaxWidth, (int)OffsetX);
			OffsetX = 0.0f;
			continue;
		}

		TrueTypeGlyph CurrentGlyph = FontToUse.Glyphs[Text[CurrentChar]];
		float BaseX = OffsetX + CurrentGlyph.BearingX;
		float BaseY = OffsetY + (CurrentGlyph.Height - CurrentGlyph.BearingY);

		// What index (of vertex) we are at right now
		// There are 5 floats per vertex so to get the index
		// of the entire vertex we divide the total number of
		// floats by 5.
		int BaseIndex = Vertices.Size / 5;

		// (0, 0)
		Vertices.Push(BaseX);
		Vertices.Push(BaseY);
		Vertices.Push(1.0f);
		Vertices.Push(CurrentGlyph.TexCoordBottomX);
		Vertices.Push(CurrentGlyph.TexCoordTopY);

		// (0, 1)
		Vertices.Push(BaseX);
		Vertices.Push(BaseY - CurrentGlyph.Height);
		Vertices.Push(1.0f);
		Vertices.Push(CurrentGlyph.TexCoordBottomX);
		Vertices.Push(CurrentGlyph.TexCoordBottomY);

		// (1, 1)
		Vertices.Push(BaseX + CurrentGlyph.Width);
		Vertices.Push(BaseY - CurrentGlyph.Height);
		Vertices.Push(1.0f);
		Vertices.Push(CurrentGlyph.TexCoordTopX);
		Vertices.Push(CurrentGlyph.TexCoordBottomY);

		// (1, 0)
		Vertices.Push(BaseX + CurrentGlyph.Width);
		Vertices.Push(BaseY);
		Vertices.Push(1.0f);
		Vertices.Push(CurrentGlyph.TexCoordTopX);
		Vertices.Push(CurrentGlyph.TexCoordTopY);

		Indices.Push(BaseIndex + 0); //3
		Indices.Push(BaseIndex + 1); //2
		Indices.Push(BaseIndex + 2); //0
		Indices.Push(BaseIndex + 0); //2
		Indices.Push(BaseIndex + 2); //1
		Indices.Push(BaseIndex + 3); //0

		OffsetX += CurrentGlyph.Advance;
		++NumGlyphs;
	}

    MaxWidth = glm::max(MaxWidth, (int)OffsetX);
	MaxHeight = -(int)OffsetY;

	glBindBuffer(GL_ARRAY_BUFFER, Result.VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * Vertices.Size, Vertices.Data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Result.IBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned short) * Indices.Size, Indices.Data(), GL_STATIC_DRAW);

	glEnableVertexAttribArray(0); // Vertex position
	glEnableVertexAttribArray(1); // Vertex texture coordinate

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (void*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (void*)(sizeof(float) * 3));

	Result.NumIndices = Indices.Size;
	//Result.Layer = Layer;
	Result.Texture = FontToUse.GlyphBitmap;
	Result.Color = Color;
	glBindVertexArray(0);

	OutDimensions.x = MaxWidth;
	OutDimensions.y = MaxHeight;

	return Result;
}

GUIRenderable GUIRenderer::CreateSprite(const char* TextureName, glm::ivec2 SpriteUV1, glm::ivec2 SpriteUV2, glm::vec2 Size)
{
	GUIRenderable Result;

	// TODO: Load from cache
	unsigned int Width;
	unsigned int Height;
	Result.Texture = PlatformFileSystem::LoadImageFromFile(std::string(TextureName), Width, Height);
	Result.Color = Color::WHITE;

	glGenVertexArrays(1, &Result.VAO);
	glBindVertexArray(Result.VAO);

	glGenBuffers(1, &Result.VBO);
	glGenBuffers(1, &Result.IBO);

	glm::vec2 RelativeUVBottomLeft = glm::vec2((float) SpriteUV1.x / (float) Width, (float) SpriteUV1.y / (float) Height);
	glm::vec2 RelativeUVTopRight = glm::vec2((float) SpriteUV2.x / (float) Width, (float) SpriteUV2.y / (float) Height);
	float Vertices[] = {
		0.0f,		0.0f,		1.0f,
		RelativeUVBottomLeft.x, RelativeUVBottomLeft.y,
		0.0f,		Size.y,		1.0f,
		RelativeUVBottomLeft.x, RelativeUVTopRight.y,
		Size.x,		Size.y,		1.0f,
		RelativeUVTopRight.x,	RelativeUVTopRight.y,
		Size.x,		0.0f,		1.0f,
		RelativeUVTopRight.x,	RelativeUVBottomLeft.y
	};

	unsigned short Indices[] = { 0, 1, 2, 0, 2, 3 };

	glBindBuffer(GL_ARRAY_BUFFER, Result.VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 20, Vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Result.IBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned short) * 6, Indices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0); // Vertex position
	glEnableVertexAttribArray(1); // Vertex texture coordinate

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (void*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (void*)(sizeof(float) * 3));

	Result.NumIndices = 6;
	glBindVertexArray(0);

	return Result;
}

GUIRenderable GUIRenderer::CreateRect(glm::vec2 Size, Color Color)
{
	GUIRenderable Result;

	Result.Color = Color;

	glGenVertexArrays(1, &Result.VAO);
	glBindVertexArray(Result.VAO);

	glGenBuffers(1, &Result.VBO);
	glGenBuffers(1, &Result.IBO);

	float Vertices[12] = {
		0.0f,		0.0f,		1.0f,
		0.0f,		Size.y,		1.0f,
		Size.x,		Size.y,		1.0f,
		Size.x,		0.0f,		1.0f
	};

	//unsigned short Indices[6] = { 0, 1, 2, 0, 2, 3 };
	unsigned short Indices[6] = { 3, 2, 0, 2, 1, 0 };

	glBindBuffer(GL_ARRAY_BUFFER, Result.VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 12, Vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Result.IBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned short) * 6, Indices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	Result.NumIndices = 6;
	glBindVertexArray(0);

	return Result;
}

void GUIRenderer::RenderAtPosition(GUIRenderable Renderable, glm::vec2 Position)
{
	if (Renderable.Texture.m_TextureId != 0)
	{
		m_pTextureShader->Bind();
		glBindVertexArray(Renderable.VAO);

		Renderable.Texture.Use();

		m_pTextureShader->SetColor(Renderable.Color);
		m_pTextureShader->SetProjectionMatrix(m_ProjectionMatrix);
		m_pTextureShader->SetPositionOffset(glm::vec3(Position, 0.0f));

		glDrawElements(GL_TRIANGLES, Renderable.NumIndices, GL_UNSIGNED_SHORT, (void*)0);
	}
	else
	{
		m_pNoTextureShader->Bind();
		glBindVertexArray(Renderable.VAO);

		m_pNoTextureShader->SetColor(Renderable.Color);
		m_pNoTextureShader->SetProjectionMatrix(m_ProjectionMatrix);
		m_pNoTextureShader->SetPositionOffset(glm::vec3(Position, 1.0f));

		glDrawElements(GL_TRIANGLES, Renderable.NumIndices, GL_UNSIGNED_SHORT, (void*)0);
	}
}

GUIRenderer::GUIRenderer(int ScreenWidth, int ScreenHeight) :
    m_bIsMouseActivated(false),
    m_Elements(List<IGUIElement*>(g_Engine->g_MemoryManager->m_pGameMemory)), // TODO: Should this be moved to the rendering memory?
	m_ScreenDimensions(glm::ivec2(ScreenWidth, ScreenHeight)),
	m_MousePosition(glm::ivec2(0, 0))
{
	m_ProjectionMatrix = glm::ortho(0.0f, (float)ScreenWidth, (float)ScreenHeight, 0.0f);

	m_pTextureShader = GLShaderProgram::CreateVertexFragmentShaderFromFile(std::string("VertexTextured2D.glsl"), std::string("FragmentTextured2D.glsl"));
	m_pNoTextureShader = GLShaderProgram::CreateVertexFragmentShaderFromFile(std::string("Vertex2D.glsl"), std::string("Fragment2D.glsl"));
}

GUIRenderer::~GUIRenderer()
{
	delete m_pTextureShader;
	delete m_pNoTextureShader;
}

void GUIRenderer::UpdateScreenDimensions(int NewWidth, int NewHeight)
{
	// Update the screen dimensions in case they are needed for any calculations
	m_ScreenDimensions = glm::ivec2(NewWidth, NewHeight);

	// Update the projection matrix so that everything renders correctly
	m_ProjectionMatrix = glm::ortho(0.0f, (float)NewWidth, (float)NewHeight, 0.0f);
}

void GUIRenderer::Render()
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	for (size_t i = 0; i < m_Elements.Size; ++i)
	{
		m_Elements[i]->Render();
	}

	glBindVertexArray(0);
	glDisable(GL_BLEND);
}

void GUIRenderer::UpdateMousePositionAndState(glm::ivec2 MousePosition, bool LMouseDown, bool RMouseDown, bool MMouseDown)
{
	for (size_t i = 0; i < m_Elements.Size; ++i)
	{
		glm::ivec2 ElementPosition = m_Elements[i]->GetPosition();
		glm::ivec2 FarthestCorner = ElementPosition + m_Elements[i]->m_Dimensions;

		if (MousePosition.x >= ElementPosition.x && MousePosition.x <= FarthestCorner.x &&
			MousePosition.y >= ElementPosition.y && MousePosition.y <= FarthestCorner.y)
		{
			// If we just started a left mouse button click
			if (LMouseDown && !m_PrevLMouseDown)
			{
				m_Elements[i]->OnBeginClick(0);
				continue;
			}
			else if (!LMouseDown && m_PrevLMouseDown)
			{
				m_Elements[i]->OnEndClick(0);
				continue;
			}

			// If we just started a left mouse button click
			if (RMouseDown && !m_PrevRMouseDown)
			{
				m_Elements[i]->OnBeginClick(1);
				continue;
			}
			else if (!RMouseDown && m_PrevRMouseDown)
			{
				m_Elements[i]->OnEndClick(1);
				continue;
			}

			// If we just started a middle mouse button click
			if (MMouseDown && !m_PrevMMouseDown)
			{
				m_Elements[i]->OnBeginClick(2);
				continue;
			}
			else if (!MMouseDown && m_PrevMMouseDown)
			{
				m_Elements[i]->OnEndClick(2);
				continue;
			}

			if (!LMouseDown && !RMouseDown && !MMouseDown)
			{
				m_Elements[i]->OnHover();
			}
		}
	}
}

static struct
{
	GLShaderProgram* Shader = NULL;
	GLuint VAO = 0;
	GLuint VBO = 0;
	GLuint IBO = 0;
    Texture TextureAtlas;
	glm::mat4 ProjectionMatrix;
} DebugGui;

void RenderDrawLists(ImDrawData* DrawData)
{
	ImGuiIO& IO = ImGui::GetIO();

	int Width = (int)(IO.DisplaySize.x * IO.DisplayFramebufferScale.x);
	int Height = (int)(IO.DisplaySize.y * IO.DisplayFramebufferScale.y);
	DrawData->ScaleClipRects(IO.DisplayFramebufferScale);

	// Enable blending, disable culling and depth test, enable scissor test
	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_SCISSOR_TEST);

    glViewport(0, 0, Width, Height);
    
	glBindVertexArray(DebugGui.VAO);

	DebugGui.Shader->Bind();
	DebugGui.Shader->SetProjectionMatrix(DebugGui.ProjectionMatrix);

	for (int i = 0; i < DrawData->CmdListsCount; ++i)
	{
		// Every CommandList has a few draw commands in a draw list
		const ImDrawList* CommandList = DrawData->CmdLists[i];
		const ImDrawIdx* IndexBufferOffset = 0;

		// Send all the vertices of this command buffer to the graphics card
		glBindBuffer(GL_ARRAY_BUFFER, DebugGui.VBO);
		glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)CommandList->VtxBuffer.size() * sizeof(ImDrawVert), (GLvoid*)&CommandList->VtxBuffer.front(), GL_STREAM_DRAW);

		// Send all the indices of this command buffer to the graphics card
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, DebugGui.IBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr)CommandList->IdxBuffer.size() * sizeof(ImDrawIdx), (GLvoid*)&CommandList->IdxBuffer.front(), GL_STREAM_DRAW);

		// TODO: Maybe move this into the constructor?
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), (void*)offsetof(ImDrawVert, pos));
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), (void*)offsetof(ImDrawVert, uv));
		glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(ImDrawVert), (void*)offsetof(ImDrawVert, col));

		// Loop through the draw commands (which all render different sections of the vertices we just sent to the GPU
		for (const ImDrawCmd* DrawCommand = CommandList->CmdBuffer.begin();
			 DrawCommand != CommandList->CmdBuffer.end();
			 ++DrawCommand)
		{
			if (DrawCommand->UserCallback)
			{
				DrawCommand->UserCallback(CommandList, DrawCommand);
			}
			else
			{
				// Bind the texture for the drawcommand
                GLuint TextureId = (GLuint)(intptr_t)DrawCommand->TextureId;
                glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, TextureId);

				// Define a scissor box defined by the ClipRect where anything outside does not get drawn
				glScissor(
					(int)DrawCommand->ClipRect.x,
					(int)(Height - DrawCommand->ClipRect.w),
					(int)(DrawCommand->ClipRect.z - DrawCommand->ClipRect.x),
					(int)(DrawCommand->ClipRect.w - DrawCommand->ClipRect.y));
				// Draw the vertices of the current DrawCommand
				glDrawElements(GL_TRIANGLES, (GLsizei)DrawCommand->ElemCount, sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, IndexBufferOffset);
			}
			// Advance in the buffer offset by how many indices we just rendered
			IndexBufferOffset += DrawCommand->ElemCount;
		}
	}

	glBindVertexArray(0);

	// Restore the state to what it was previously
	glDisable(GL_BLEND);
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_SCISSOR_TEST);
}

void SetClipboardText(const char* Text)
{
	SDL_SetClipboardText(Text);
}

const char* GetClipboardText()
{
	return SDL_GetClipboardText();
}

#define IMCOLOR(r, g, b) ImVec4((float)r / 255.0f, (float)g / 255.0f, (float)b / 255.0f, 1.0f)
#define IMCOLOR_A(r, g, b, a) ImVec4((float)r / 255.0f, (float)g / 255.0f, (float)b / 255.0f, (float)a / 255.0f)

DebugGUIRenderer::DebugGUIRenderer(int ScreenWidth, int ScreenHeight)
{
	ImGuiIO& IO = ImGui::GetIO();
    
    UpdateScreenDimensions(ScreenWidth, ScreenHeight);

	// The padding (space between the window and the elements) and rounding (corners)
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10, 10));
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 2.0f);

	// The space between each item
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10, 10));

	// The padding (distance inside a slider for example) and the rounding on the slider background
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8, 8));
	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.0f);
    
	ImGui::PushStyleColor(ImGuiCol_Border, IMCOLOR(255, 255, 255));

	ImGui::PushStyleColor(ImGuiCol_TitleBg, IMCOLOR(88, 100, 29));
	ImGui::PushStyleColor(ImGuiCol_TitleBgActive, IMCOLOR(88, 100, 29));
	ImGui::PushStyleColor(ImGuiCol_TitleBgCollapsed, IMCOLOR(88, 100, 29));

	ImGui::PushStyleColor(ImGuiCol_WindowBg, IMCOLOR_A(250, 250, 250, 128));

	ImGui::PushStyleColor(ImGuiCol_FrameBg, IMCOLOR(123, 144, 75));
	ImGui::PushStyleColor(ImGuiCol_FrameBgActive, IMCOLOR(123, 144, 75));
	ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, IMCOLOR(123, 144, 75));

	ImGui::PushStyleColor(ImGuiCol_Button, IMCOLOR(88, 100, 29));
	ImGui::PushStyleColor(ImGuiCol_SliderGrab, IMCOLOR(88, 100, 29));
	ImGui::PushStyleColor(ImGuiCol_ScrollbarGrab, IMCOLOR(88, 100, 29));

	ImGui::PushStyleColor(ImGuiCol_ButtonActive, IMCOLOR(59, 67, 20));
	ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, IMCOLOR(59, 67, 20));
	ImGui::PushStyleColor(ImGuiCol_ScrollbarGrabActive, IMCOLOR(59, 67, 20));

	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IMCOLOR(98, 110, 42));
	ImGui::PushStyleColor(ImGuiCol_ScrollbarGrabHovered, IMCOLOR(98, 110, 42));

	IO.KeyMap[ImGuiKey_Tab] = SDLK_TAB;
	IO.KeyMap[ImGuiKey_LeftArrow] = SDL_SCANCODE_LEFT;
	IO.KeyMap[ImGuiKey_RightArrow] = SDL_SCANCODE_RIGHT;
	IO.KeyMap[ImGuiKey_UpArrow] = SDL_SCANCODE_UP;
	IO.KeyMap[ImGuiKey_DownArrow] = SDL_SCANCODE_DOWN;
	IO.KeyMap[ImGuiKey_PageUp] = SDL_SCANCODE_PAGEUP;
	IO.KeyMap[ImGuiKey_PageDown] = SDL_SCANCODE_PAGEDOWN;
	IO.KeyMap[ImGuiKey_Home] = SDL_SCANCODE_HOME;
	IO.KeyMap[ImGuiKey_End] = SDL_SCANCODE_END;
	IO.KeyMap[ImGuiKey_Delete] = SDLK_DELETE;
	IO.KeyMap[ImGuiKey_Backspace] = SDLK_BACKSPACE;
	IO.KeyMap[ImGuiKey_Enter] = SDLK_RETURN;
	IO.KeyMap[ImGuiKey_Escape] = SDLK_ESCAPE;
	IO.KeyMap[ImGuiKey_A] = SDLK_a;
	IO.KeyMap[ImGuiKey_C] = SDLK_c;
	IO.KeyMap[ImGuiKey_V] = SDLK_v;
	IO.KeyMap[ImGuiKey_X] = SDLK_x;
	IO.KeyMap[ImGuiKey_Y] = SDLK_y;
	IO.KeyMap[ImGuiKey_Z] = SDLK_z;

	// Generate the vertexarrays, vertexbuffers index buffers and texture objects to be used each frame
	glGenVertexArrays(1, &DebugGui.VAO);
	glGenBuffers(1, &DebugGui.VBO);
	glGenBuffers(1, &DebugGui.IBO);

	unsigned char* pixels;
	int TextureWidth;
	int TextureHeight;
	IO.Fonts->GetTexDataAsRGBA32(&pixels, &TextureWidth, &TextureHeight);

    DebugGui.TextureAtlas.LoadFromBuffer(pixels, TextureWidth, TextureHeight, GL_RGBA, GL_RGBA);
	DebugGui.TextureAtlas.SetFilteringMode(GL_LINEAR);

    IO.Fonts->TexID = (void *)(intptr_t)DebugGui.TextureAtlas.m_TextureId;

	// This tells it to send all the draw calls to RenderDrawLists
	// when ImGui::Render is called
	IO.RenderDrawListsFn = &RenderDrawLists;

	// Functions for manipulating the clipboard (when the user Ctrl+C's
	// or Ctrl+V's)
	IO.SetClipboardTextFn = &SetClipboardText;
	IO.GetClipboardTextFn = &GetClipboardText;

#ifdef _WIN32
	/*SDL_SysWMinfo wmInfo;
	SDL_VERSION(&wmInfo.version);
	SDL_GetWindowWMInfo(SDLWindow, &wmInfo);
	IO.ImeWindowHandle = wmInfo.info.win.window;*/
#endif

	if (DebugGui.Shader == NULL)
	{
		DebugGui.Shader = GLShaderProgram::CreateVertexFragmentShaderFromFile(std::string("VertexDebugGUI.glsl"), std::string("FragmentDebugGUI.glsl"));
	}
}

DebugGUIRenderer::~DebugGUIRenderer()
{
	if (DebugGui.Shader)
	{
		delete DebugGui.Shader;
	}
	if (DebugGui.VAO)
	{
		glDeleteVertexArrays(1, &DebugGui.VAO);
	}
	if (DebugGui.VBO)
	{
		glDeleteBuffers(1, &DebugGui.VBO);
	}
	if (DebugGui.IBO)
	{
		glDeleteBuffers(1, &DebugGui.IBO);
	}

	ImGui::Shutdown();
}

void DebugGUIRenderer::UpdateScreenDimensions(int NewWidth, int NewHeight)
{
    ImGuiIO& IO = ImGui::GetIO();
    
#if 0
    IO.DisplaySize = ImVec2(NewWidth, NewHeight);
    IO.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);
    
    DebugGui.ProjectionMatrix = glm::ortho(0.0f, (float)NewWidth, (float)NewHeight, 0.0f);
#else
    SDL_Window* MainWindow = PlatformWindow::GlobalWindow->GetWindow();
    int Width, Height;
    int DrawWidth, DrawHeight;
    SDL_GetWindowSize(MainWindow, &Width, &Height);
    SDL_GL_GetDrawableSize(MainWindow, &DrawWidth, &DrawHeight);
    
    IO.DisplaySize = ImVec2((float)Width, (float)Height);
    IO.DisplayFramebufferScale = ImVec2(Width > 0 ? ((float)DrawWidth / Width) : 0, Height > 0 ? ((float)DrawHeight / Height) : 0);
    
    DebugGui.ProjectionMatrix = glm::ortho(0.0f, (float)Width, (float)Height, 0.0f);
#endif
}

void DebugGUIRenderer::BeginFrame(float DeltaTime)
{
	ImGuiIO& IO = ImGui::GetIO();
    
	// TODO: This should not be gotten from SDL directly rather it should go through the input system

	// Get the mouse state
	int MouseX;
	int MouseY;
	Uint32 MouseState = SDL_GetMouseState(&MouseX, &MouseY);
    
    IO.DeltaTime = DeltaTime;
    
	// Only set the mouse position if the window/mouse is 'focused'
	if (SDL_GetWindowFlags(PlatformWindow::GlobalWindow->GetWindow()) & SDL_WINDOW_MOUSE_FOCUS)
		IO.MousePos = ImVec2((float)MouseX, (float)MouseY);   // Mouse position, in pixels (set to -1,-1 if no mouse / on another screen, etc.)
	else
		IO.MousePos = ImVec2(-1, -1);

	IO.MouseDown[0] = Input::MouseButtons[0] || (MouseState & SDL_BUTTON(SDL_BUTTON_LEFT)) != 0;
	IO.MouseDown[1] = Input::MouseButtons[1] || (MouseState & SDL_BUTTON(SDL_BUTTON_RIGHT)) != 0;
	IO.MouseDown[2] = Input::MouseButtons[2] || (MouseState & SDL_BUTTON(SDL_BUTTON_MIDDLE)) != 0;

	IO.MouseWheel = Input::MouseWheel;

	// Hide OS mouse cursor if ImGui is drawing it
	SDL_ShowCursor(IO.MouseDrawCursor ? 0 : 1);

	ImGui::NewFrame();
}

#include "../Engine/Camera.h"

void DebugGUIRenderer::RenderFrame(int FramesPerSecond, float FrameTime)
{
	glm::vec3 PlayerPos = g_Engine->g_World->m_pCurrentPlayer->GetPosition();
	glm::ivec3 PlayerChunkPos = glm::ivec3(PlayerPos) / 32;
	// Display FPS
	ImGui::SetNextWindowPos(ImVec2(10, 10));
	ImGui::Begin("Debug info:", (bool*)0, ImVec2(0, 0), 0.3f, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings);
	ImGui::Text("Last frame time: %.2f ms", FrameTime * 1000.0f);
	ImGui::Text("Current FPS: %d", FramesPerSecond);
	ImGui::Separator();
	ImGui::Text("World Pos: (%.2f, %.2f, %.2f)", PlayerPos.x, PlayerPos.y, PlayerPos.z);
	ImGui::Text("Chunk Pos: (%d, %d, %d)", PlayerChunkPos.x, PlayerChunkPos.y, PlayerChunkPos.z);
	ImGui::End();

	// Temporary crosshair rendering
	int CrosshairSize = 6;
	int WindowWidth;
	int WindowHeight;
	SDL_GL_GetDrawableSize(PlatformWindow::GlobalWindow->GetWindow(), &WindowWidth, &WindowHeight);
	glEnable(GL_SCISSOR_TEST);
	glScissor((WindowWidth / 2) - (CrosshairSize / 2), (WindowHeight / 2) - (CrosshairSize / 2), CrosshairSize, CrosshairSize);
	glClearColor(1.0f, 1.0f, 1.0f, 0.75f);
	glClear(GL_COLOR_BUFFER_BIT);
	glDisable(GL_SCISSOR_TEST);

	g_Engine->g_Console->Draw();

	ImGui::Render();
}
