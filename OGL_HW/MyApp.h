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

#include "Mesh_OGL3.h"
#include "ProgramObject.h"
#include "BufferObject.h"
#include "VertexArrayObject.h"
#include "TextureObject.h"

const static unsigned int NUM_POINT_LIGHTS = 100;

class CMyApp
{
public:
	CMyApp(int, int);
	~CMyApp(void);

	bool Init(int, int);
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
	void LoadAssets();
	void CreateForwardBuffer(int, int);
	void DrawScene();

	bool frameBufferCreated;
	GLuint fbo;
	GLuint colorBuffer;
	GLuint normalBuffer;
	GLuint positionBuffer;
	GLuint materialBuffer;
	GLuint depthBuffer;

	gCamera				camera;

	ProgramObject		programForwardRenderer;
	ProgramObject		programLightRenderer;
	ProgramObject		programLightSpheres;

	Texture2D			tex_terrain;
	Texture2D			tex_grass;
	Texture2D			tex_leaves;
	Texture2D			tex_stems;
	Texture2D			tex_plants;
	Texture2D			tex_rocks;
	Texture2D			tex_water;

	std::unique_ptr<Mesh>	mesh_terrain;
	std::unique_ptr<Mesh>	mesh_grass;
	std::unique_ptr<Mesh>	mesh_leaves;
	std::unique_ptr<Mesh>	mesh_stems;
	std::unique_ptr<Mesh>	mesh_plants;
	std::unique_ptr<Mesh>	mesh_rocks;
	std::unique_ptr<Mesh>	mesh_water;

	std::vector<glm::vec3> pointLightPositions;
	std::vector<glm::vec3> pointLightNextPositions;
	ArrayBuffer spherePositions;
	VertexArrayObject spheres_vao;

	double					delta_time;
	bool					frozen;
};

