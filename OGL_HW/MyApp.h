#pragma once

// C++ includes
#include <memory>
#include <array>

// GLEW
#include <GL/glew.h>

// SDL
#include <SDL.h>
#include <SDL_opengl.h>

// GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform2.hpp>

#include "gCamera.h"

#include "ProgramObject.h"
#include "BufferObject.h"
#include "VertexArrayObject.h"
#include "TextureObject.h"

class CMyApp
{
public:
	CMyApp(void);
	~CMyApp(void);

	bool Init();
	void Clean();

	void Update();
	void Render();

	void KeyboardDown(SDL_KeyboardEvent&);
	void KeyboardUp(SDL_KeyboardEvent&);
	void MouseMove(SDL_MouseMotionEvent&);
	void MouseDown(SDL_MouseButtonEvent&);
	void MouseUp(SDL_MouseButtonEvent&);
	void MouseWheel(SDL_MouseWheelEvent&);
	void Resize(int, int);
protected:
	gCamera				m_camera;

	ProgramObject		m_programPixelShader;
	ProgramObject		m_programSkybox;

	VertexArrayObject	m_vao;
	IndexBuffer			m_gpuBufferIndices;
	ArrayBuffer			m_gpuBufferPos;

	GLuint				m_skyboxTexture;
	void TextureFromFileAttach(const char* filename, GLuint role) const;

	float				screenWidth;
	float				screenHeight;

	// variables for calculating real time fps - modified in Update()
	double				m_delta_time;
};

