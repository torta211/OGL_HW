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
const static int DIR_SHADOW_MAP_RES = 2048;

class CMyApp
{
public:
	CMyApp(int, int);
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
	void LoadAssets();
	void CreateFrameBuffers();
	void DrawScene(glm::mat4);

	int						width;
	int						height;
	bool					frameBufferCreated;

	GLuint					shadow_fbo;
	GLuint					shadow_depth_texture;

	GLuint					fbo;
	GLuint					colorBuffer;
	GLuint					normalBuffer;
	GLuint					positionBuffer;
	GLuint					materialBuffer;
	GLuint					depthBuffer;

	gCamera					camera;

	ProgramObject			programForwardRenderer;
	ProgramObject			programLightRenderer;
	ProgramObject			programLightSpheres;
	ProgramObject			programShadowMapper;
	ProgramObject			programDirectionalLight;

	Texture2D				tex_terrain;
	Texture2D				tex_grass;
	Texture2D				tex_leaves;
	Texture2D				tex_stems;
	Texture2D				tex_plants;
	Texture2D				tex_rocks;
	Texture2D				tex_water;

	std::unique_ptr<Mesh>	mesh_terrain;
	std::unique_ptr<Mesh>	mesh_grass;
	std::unique_ptr<Mesh>	mesh_leaves;
	std::unique_ptr<Mesh>	mesh_stems;
	std::unique_ptr<Mesh>	mesh_plants;
	std::unique_ptr<Mesh>	mesh_rocks;
	std::unique_ptr<Mesh>	mesh_water;

	std::vector<glm::vec3>	pointLightPositions;
	std::vector<glm::vec3>	pointLightNextPositions;
	std::vector<float>		pointLightStrengths;
	std::vector<glm::vec3>	pointLightColors;
	ArrayBuffer				spherePositions;
	VertexArrayObject		spheres_vao;

	double					delta_time;
	float					t;
	bool					frozen;
};

