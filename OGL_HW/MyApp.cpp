#include "MyApp.h"

#include <math.h>
#include <vector>

#include <array>
#include <list>
#include <tuple>
#include <imgui/imgui.h>
#include <random>
#include <cmath>
#include "glm/ext.hpp"

CMyApp::CMyApp(void)
{
	m_camera.SetView(glm::vec3(5, 5, 5), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
}


CMyApp::~CMyApp(void)
{
	std::cout << "dtor!\n";
}

bool CMyApp::Init()
{	
	std::cout << "Card used: " << glGetString(GL_RENDERER) << "\n";
	// Query aspect ratio
	double viewPortParams[4];
	glGetDoublev(GL_VIEWPORT, viewPortParams);
	screenWidth = (float)viewPortParams[2];
	screenHeight = (float)viewPortParams[3];

	// Clear color will be blueish
	glClearColor(0.125f, 0.25f, 0.5f, 1.0f);

	// Enable depth test
	// We keep both faces of each fragment
	glEnable(GL_DEPTH_TEST);

	// Fast creation of shader programs (3 function calls inside a single call)
	m_programPixelShader.Init(
	{
		{ GL_VERTEX_SHADER, "fractal.vert" },
		{ GL_FRAGMENT_SHADER, "fractal.frag" }
	},
	{
		{ 0, "vs_in_pos" },
	}
	);
	m_programSkybox.Init(
		{
			{ GL_VERTEX_SHADER, "skybox.vert" },
			{ GL_FRAGMENT_SHADER, "skybox.frag" }
		},
	{
		{ 0, "vs_in_pos" },
	}
	);


	//
	// Defining geometry (std::vector<...>) and upload to GPU buffers (m_buffer*) with BufferData
	//

	// Position of vertices:
	/*
	The constructor of m_gpuBufferPos has already created a GPU buffer identifier, and the following BufferData call will
	1. bind this to GL_ARRAY_BUFFER (because the type of m_gpuBufferPos is ArrayBuffer) and
	2. upload the values of the container given in the argument to the GPU by calling glBufferData
	*/
	m_gpuBufferPos.BufferData(
		std::vector<glm::vec3>{
			// square on z = -1
			glm::vec3(-1, -1, -1),
			glm::vec3(1, -1, -1),
			glm::vec3(1, 1, -1),
			glm::vec3(-1, 1, -1),
			// square on z = 1
			glm::vec3(-1, -1, 1),
			glm::vec3(1, -1, 1),
			glm::vec3(1, 1, 1),
			glm::vec3(-1, 1, 1),
	}
	);

	// And the indices which the primitives are constructed by (from the arrays defined above) - prepared to draw them as a triangle list
	m_gpuBufferIndices.BufferData(
		std::vector<int>{
			// fullscreen quad and back face of cubemap
			0, 1, 2, 2, 3, 0,
			// other cubemap faces
			4, 6, 5, 6, 4, 7,
			0, 3, 4, 4, 3, 7,
			1, 5, 2, 5, 6, 2,
			1, 0, 4, 1, 4, 5,
			3, 2, 6, 3, 6, 7,
	}
	);

	// Registering geometry in VAO
	m_vao.Init(
	{
		// Attribute 0 is "practically" an array of glm::vec3 and the data is in the GPU buffer (m_gpuBufferPos)
		{ CreateAttribute<		0,						// Channel 0
								glm::vec3,				// CPU-side data type which is used to define attributes of channel 0 <- the procedure deducts that the attribute 0 is made of 3 floats from the glm::vec3
								0,						// offset: The offset of the attribute, considered from the beginning of the container
								sizeof(glm::vec3)		// stride: This attribute of the next vertex will be this many bytes from the current
							>, m_gpuBufferPos },		
	},
	m_gpuBufferIndices
	);

	// Skybox
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	glGenTextures(1, &m_skyboxTexture);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_skyboxTexture);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	TextureFromFileAttach("xpos.png", GL_TEXTURE_CUBE_MAP_POSITIVE_X);
	TextureFromFileAttach("xneg.png", GL_TEXTURE_CUBE_MAP_NEGATIVE_X);
	TextureFromFileAttach("ypos.png", GL_TEXTURE_CUBE_MAP_POSITIVE_Y);
	TextureFromFileAttach("yneg.png", GL_TEXTURE_CUBE_MAP_NEGATIVE_Y);
	TextureFromFileAttach("zpos.png", GL_TEXTURE_CUBE_MAP_POSITIVE_Z);
	TextureFromFileAttach("zneg.png", GL_TEXTURE_CUBE_MAP_NEGATIVE_Z);

	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	// Camera
	m_camera.SetProj(45.0f, 640.0f / 480.0f, 0.01f, 1000.0f);

	return true;
}

void CMyApp::TextureFromFileAttach(const char* filename, GLuint role) const
{
	SDL_Surface* loaded_img = IMG_Load(filename);

	int img_mode = 0;

	if (loaded_img == 0)
	{
		std::cout << "[TextureFromFile] Error loading the image: " << filename << std::endl;
		return;
	}

#if SDL_BYTEORDER == SDL_LIL_ENDIAN
	if (loaded_img->format->BytesPerPixel == 4)
		img_mode = GL_BGRA;
	else
		img_mode = GL_BGR;
#else
	if (loaded_img->format->BytesPerPixel == 4)
		img_mode = GL_RGBA;
	else
		img_mode = GL_RGB;
#endif

	glTexImage2D(role, 0, GL_RGBA, loaded_img->w, loaded_img->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, loaded_img->pixels);

	SDL_FreeSurface(loaded_img);
}

void CMyApp::Clean()
{
	glDeleteTextures(1, &m_skyboxTexture);
}

void CMyApp::Update()
{
	// static declaration runs only once per application
	static Uint32 last_time = SDL_GetTicks();
	m_delta_time = (SDL_GetTicks() - last_time) / 1000.0f;

	m_camera.Update(static_cast<float>(m_delta_time));

	last_time = SDL_GetTicks();
}

void CMyApp::Render()
{
	// Delete the frame buffer (GL_COLOR_BUFFER_BIT) and the depth (Z) buffer (GL_DEPTH_BUFFER_BIT)
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	m_vao.Bind();

	// Pixel shading shadertoy style
	m_programPixelShader.Use();
	m_programPixelShader.SetUniform("cam_pos", m_camera.GetEye());
	m_programPixelShader.SetUniform("cam_forward", m_camera.GetForward());
	m_programPixelShader.SetUniform("cam_up", m_camera.GetUp());
	m_programPixelShader.SetUniform("cam_right", m_camera.GetRight());
	m_programPixelShader.SetUniform("screen_width", screenWidth);
	m_programPixelShader.SetUniform("screen_height", screenHeight);
	m_programPixelShader.SetUniform("time", SDL_GetTicks());

	
	// For this we only use the first 2 triangles
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

	// Rendering the cubemap (skybox)
	// Save previous depth function
	GLint prevDepthFnc;
	glGetIntegerv(GL_DEPTH_FUNC, &prevDepthFnc);
	// Set less than equal
	glDepthFunc(GL_LEQUAL);
	// Shader program
	m_programSkybox.Use();
	m_programSkybox.SetUniform("MVP", m_camera.GetViewProj() * glm::translate(m_camera.GetEye()));
	// Set cubemap texture
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_skyboxTexture);
	glUniform1i(m_programSkybox.GetLocation("skyboxTexture"), 0);
	// Draw all triangles in the vao
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, nullptr);
	// set back depth function
	glDepthFunc(prevDepthFnc);
}

void CMyApp::KeyboardDown(SDL_KeyboardEvent& key)
{
	m_camera.KeyboardDown(key);
}

void CMyApp::KeyboardUp(SDL_KeyboardEvent& key)
{
	m_camera.KeyboardUp(key);
}

void CMyApp::MouseMove(SDL_MouseMotionEvent& mouse)
{
	m_camera.MouseMove(mouse);
}

void CMyApp::MouseDown(SDL_MouseButtonEvent& mouse)
{
}

void CMyApp::MouseUp(SDL_MouseButtonEvent& mouse)
{
}

void CMyApp::MouseWheel(SDL_MouseWheelEvent& wheel)
{
}

// _w and _h are the width and height of the window's size
void CMyApp::Resize(int _w, int _h)
{
	glViewport(0, 0, _w, _h );

	m_camera.Resize(_w, _h);
	screenWidth = float(_w);
	screenHeight = float(_h);
}