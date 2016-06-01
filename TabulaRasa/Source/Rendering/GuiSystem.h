#pragma once

#include <string>

#include "GL\glew.h"
#include "glm\glm.hpp"
#include "glm\gtc\matrix_transform.hpp"

#include "Fonts.h"

#include "..\Engine\Core\List.h"

template<typename T>
struct MemoryPool;
class GLShaderProgram;

struct Color
{
	static const Color BLACK;
	static const Color WHITE;
	static const Color GRAY;

	static const Color RED;
	static const Color GREEN;
	static const Color BLUE;
	static const Color YELLOW;
	static const Color PINK;
	static const Color ORANGE;
	static const Color PURPLE;
	static const Color CYAN;
	static const Color BROWN;

	Color() : R(0.0f), G(0.0f), B(0.0f), A(0.0f) {}
	Color(float f) : R(f), G(f), B(f), A(f) {}

	Color(int AlphaHex)
	{
		R = (float)((AlphaHex & 0xFF0000) >> 16) / 255.0f;
		G = (float)((AlphaHex & 0x00FF00) >> 8) / 255.0f;
		B = (float)(AlphaHex & 0x0000FF) / 255.0f;
		A = 1.0f;
	}

	Color(float r, float g, float b) : R(r), G(g), B(b), A(1.0f) {}
	Color(float r, float g, float b, float a) : R(r), G(g), B(b), A(a) {}

	Color(int r, int g, int b) : R((float)r / 255.0f), G((float)g / 255.0f), B((float)b / 255.0f), A(1.0f) {}
	Color(int r, int g, int b, int a) : R((float)r / 255.0f), G((float)g / 255.0f), B((float)b / 255.0f), A(a) {}

	Color(unsigned char r, unsigned char g, unsigned char b) : R((float)r / 255.0f), G((float)g / 255.0f), B((float)b / 255.0f), A(1.0f) {}
	Color(unsigned char r, unsigned char g, unsigned char b, unsigned char a) : R((float)r / 255.0f), G((float)g / 255.0f), B((float)b / 255.0f), A((float)a / 255.0f) {}
	
	float R;
	float G;
	float B;
	float A;
};

// TODO: Change the vertex array, vertex buffer, index buffer and texture object to be classes so they can be rendering-backend agnostic
struct GUIRenderable
{
	GLuint VAO;
	GLuint VBO;
	GLuint IBO;
	unsigned short NumIndices;

	GLuint Texture;

	Color Color;
};

class IGUIElement
{
public:

	IGUIElement(IGUIElement* Parent) :
		m_Position(glm::ivec2(0)),
		m_pParent(Parent),
		m_Dimensions(glm::ivec2(0))
	{}

	virtual void Initialize(class GUIRenderer* Renderer) { m_pRenderer = Renderer; }

	virtual void OnHover() {}

	virtual void OnBeginClick(int MouseButton) {}
	virtual void OnEndClick(int MouseButton) {}

	virtual void Render() {}

	glm::ivec2 GetPosition()
	{
		if (m_pParent != NULL)
			return m_pParent->GetPosition() + m_Position;
		else 
			return m_Position;
	}

	class GUIRenderer* m_pRenderer;

	IGUIElement* m_pParent;

	/**
	 * The position, in screen coordinates that the element is at,
	 * this is the lower left corner of the element
	 */
	glm::ivec2 m_Position;

	/**
	 * The width and height, which together with the m_PosX and m_PosY
	 * make up the entire bounding box of the element where
	 * (m_Position.x + m_Dimensions.x, m_Position.y + m_Dimensions.y) 
	 * would be the top right corner
	 */
	glm::ivec2 m_Dimensions;
};

class GUIRenderer
{
public:

	static GUIRenderable CreateText(const char* Text, size_t StringLength, glm::ivec2& OutDimensions, Color Color = Color::WHITE, unsigned int Layer = 0, TrueTypeFont* Font = NULL);

	static GUIRenderable CreateSprite(const char* TextureName, glm::ivec2 SpriteUV1, glm::ivec2 SpriteUV2, glm::vec2 Size = glm::vec2(10.0f, 10.0f));

	static GUIRenderable CreateRect(glm::vec2 Size, Color Color = Color::WHITE);

public:

	GUIRenderer(int ScreenWidth, int ScreenHeight);

	virtual ~GUIRenderer();

	template<typename T>
	T* AddGUIElement(IGUIElement* Parent);

	/**
	 * Called when the screen changes size (resized or fullscreened")
	 */
	void UpdateScreenDimensions(int NewWidth, int NewHeight);

	/**
	 * Renders all GUI elements currently active
	 */
	void Render();

	/**
	 * Updates the mouse and its state
	 */
	void UpdateMousePositionAndState(glm::ivec2 MousePosition, bool LMouseDown, bool RMouseDown, bool MMouseDown);

	/**
	 * Renders a GUIRenderable at the specified screen coordinate
	 */
	void RenderAtPosition(GUIRenderable Renderable, glm::vec2 Position);

	bool m_bIsMouseActivated;

private:

	List<IGUIElement*> m_Elements;

	GLShaderProgram* m_pTextureShader;

	GLShaderProgram* m_pNoTextureShader;

	glm::ivec2 m_ScreenDimensions;

	glm::ivec2 m_MousePosition;

	glm::mat4 m_ProjectionMatrix;

	bool m_PrevLMouseDown;

	bool m_PrevRMouseDown;

	bool m_PrevMMouseDown;
};

void RenderDrawLists(struct ImDrawData* DrawData);

void SetClipboardText(const char* Text);

const char* GetClipboardText();

class DebugGUIRenderer
{
public:

	DebugGUIRenderer(int ScreenWidth, int ScreenHeight);

	~DebugGUIRenderer();

	/**
	* Called when the screen changes size (resized or fullscreened")
	*/
	void UpdateScreenDimensions(int NewWidth, int NewHeight);

	/**
	 * Sets up the screen for rendering with ImGui
	 */	
	void BeginFrame();

	/**
	* Renders a frame of ImGui elements to the screen
	*/
	void RenderFrame();
};

template<typename T>
T* GUIRenderer::AddGUIElement(IGUIElement* Parent)
{
	// TODO: This should be done by custom allocator
	T* Element = new T(Parent);

	m_Elements.Push(Element);
	Element->Initialize(this);
	return (T*)Element;
}