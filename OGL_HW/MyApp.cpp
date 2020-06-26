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
#include "ObjParser_OGL3.h"

CMyApp::CMyApp(int w_init, int h_init)
{
	frameBufferCreated = false;
	camera.SetView(glm::vec3(5, 5, 5), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
	camera.SetProj(45.0f, float(w_init) / float(h_init), 0.01f, 1000.0f);
	camera.SetSpeed(50.0f);
}

CMyApp::~CMyApp(void)
{
	std::cout << "dtor!\n";
}

void CMyApp::LoadAssets()
{
	//mesh_terrain = ObjParser::parse("terrain.obj");
	//tex_terrain.FromFile("sand.jpg");
	//std::cout << "terrain assets loaded\n";

	mesh_grass = ObjParser::parse("grass.obj");
	tex_grass.FromFile("grass.jpg");
	std::cout << "grass assets loaded\n";

	mesh_leaves = ObjParser::parse("leaves.obj");
	tex_leaves.FromFile("leave.jpg");
	std::cout << "leaves assets loaded\n";

	mesh_stems = ObjParser::parse("stems.obj");
	tex_stems.FromFile("palmstem.jpg");
	std::cout << "stems assets loaded\n";

	mesh_plants = ObjParser::parse("plants.obj");
	tex_plants.FromFile("plant.jpg");
	std::cout << "plants assets loaded\n";

	mesh_rocks = ObjParser::parse("rocks.obj");
	tex_rocks.FromFile("rock.jpg");
	std::cout << "rocks assets loaded\n";

	mesh_water = ObjParser::parse("water.obj");
	tex_water.FromFile("water.jpg");
	std::cout << "water assets loaded\n";
}

inline void setTexture2DParameters(GLenum magfilter = GL_LINEAR, GLenum minfilter = GL_LINEAR, GLenum wrap_s = GL_CLAMP_TO_EDGE, GLenum wrap_t = GL_CLAMP_TO_EDGE)
{
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magfilter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minfilter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap_s);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap_t);
}

void CMyApp::CreateForwardBuffer(int width, int height)
{
	// Clear if the function is not being called for the first time
	if (frameBufferCreated)
	{
		glDeleteTextures(1, &colorBuffer);
		glDeleteTextures(1, &normalBuffer);
		glDeleteTextures(1, &positionBuffer);
		glDeleteTextures(1, &materialBuffer);
		glDeleteRenderbuffers(1, &depthBuffer);
		glDeleteFramebuffers(1, &fbo);
	}

	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	// (Attachment 0.) Target for the base (texture) color of pixels
	glGenTextures(1, &colorBuffer);
	glBindTexture(GL_TEXTURE_2D, colorBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_FLOAT, nullptr); // last 3 parameters are only for initial values
	setTexture2DParameters(GL_NEAREST, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorBuffer, 0);
	if (glGetError() != GL_NO_ERROR) {
		std::cout << "Error creating color attachment 0" << GLenum(glGetError()) << std::endl;
		exit(1);
	}

	// (Attachment 1.) Target for normal vectors of pixels
	glGenTextures(1, &normalBuffer);
	glBindTexture(GL_TEXTURE_2D, normalBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16_SNORM, width, height, 0, GL_RGBA, GL_FLOAT, nullptr); // last 3 parameters are only for initial values
	setTexture2DParameters(GL_NEAREST, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, normalBuffer, 0);
	if (glGetError() != GL_NO_ERROR) {
		std::cout << "Error creating color attachment 1" << std::endl;
		exit(1);
	}

	// (Attachment 2.) Target for world coordinates of pixels
	glGenTextures(1, &positionBuffer);
	glBindTexture(GL_TEXTURE_2D, positionBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr); // last 3 parameters are only for initial values
	setTexture2DParameters(GL_NEAREST, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, positionBuffer, 0);
	if (glGetError() != GL_NO_ERROR) {
		std::cout << "Error creating color attachment 2" << std::endl;
		exit(1);
	}

	// (Attachment 3.) Target for material properties of pixels
	glGenTextures(1, &materialBuffer);
	glBindTexture(GL_TEXTURE_2D, materialBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr); // last 3 parameters are only for initial values
	setTexture2DParameters(GL_NEAREST, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, materialBuffer, 0);
	if (glGetError() != GL_NO_ERROR) {
		std::cout << "Error creating color attachment 3" << std::endl;
		exit(1);
	}

	// Depth renderbuffer
	glGenRenderbuffers(1, &depthBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);
	if (glGetError() != GL_NO_ERROR) {
		std::cout << "Error creating depth attachment" << std::endl;
		exit(1);
	}

	//Specifying which color outputs are active
	GLenum drawBuffers[4] = { GL_COLOR_ATTACHMENT0,
							  GL_COLOR_ATTACHMENT1,
							  GL_COLOR_ATTACHMENT2,
							  GL_COLOR_ATTACHMENT3 };
	glDrawBuffers(4, drawBuffers);

	// Completeness check
	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cout << "Incomplete framebuffer (";
		switch (status)
		{
		case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
			std::cout << "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT";
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
			std::cout << "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT";
			break;
		case GL_FRAMEBUFFER_UNSUPPORTED:
			std::cout << "GL_FRAMEBUFFER_UNSUPPORTED";	
			break;
		}
		std::cout << ")" << std::endl;
		exit(1);
	}

	// Unbind framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	frameBufferCreated = true;
}

bool CMyApp::Init(int w_init, int h_init)
{	
	// Set clear color
	glClearColor(0.2f, 0.4f, 0.7f, 1);
	// For this scene we just keep all faces
	//glEnable(GL_CULL_FACE);

	programForwardRenderer.Init({
		{ GL_VERTEX_SHADER, "forward.vert" },
		{ GL_FRAGMENT_SHADER, "forward.frag" }
	});

	programLightRenderer.Init({
		{ GL_VERTEX_SHADER, "deferredPoint.vert" },
		{ GL_FRAGMENT_SHADER, "deferredPoint.frag" }
	});

	LoadAssets();

	CreateForwardBuffer(w_init, h_init);

	return true;
}


void CMyApp::Clean()
{
	//glDeleteTextures();
}

void CMyApp::Update()
{
	// static declaration runs only once per application
	static Uint32 last_time = SDL_GetTicks();
	delta_time = (SDL_GetTicks() - last_time) / 1000.0f;

	camera.Update(static_cast<float>(delta_time));

	last_time = SDL_GetTicks();
}

void CMyApp::DrawScene()
{
	programForwardRenderer.Use();

	programForwardRenderer.SetUniform("MVP", camera.GetViewProj());
	programForwardRenderer.SetUniform("eye_pos", camera.GetEye());
	programForwardRenderer.SetUniform("Ka", 0.5f);
	programForwardRenderer.SetUniform("Kd", 0.6f);
	programForwardRenderer.SetUniform("Ks", 0.05f);
	programForwardRenderer.SetUniform("specular_power", 50.0f);

	//programForwardRenderer.SetTexture("texImage", 0, tex_terrain);
	//mesh_terrain->draw();
	programForwardRenderer.SetTexture("texImage", 0, tex_grass);
	mesh_grass->draw();

	programForwardRenderer.SetTexture("texImage", 0, tex_leaves);
	mesh_leaves->draw();

	programForwardRenderer.SetTexture("texImage", 0, tex_stems);
	mesh_stems->draw();

	programForwardRenderer.SetTexture("texImage", 0, tex_plants);
	mesh_plants->draw();

	programForwardRenderer.SetUniform("Kd", 0.2f);
	programForwardRenderer.SetTexture("texImage", 0, tex_rocks);
	mesh_rocks->draw();

	programForwardRenderer.SetUniform("Kd", 0.6f);
	programForwardRenderer.SetUniform("Ks", 0.2f);
	programForwardRenderer.SetUniform("specular_power", 30.0f);
	programForwardRenderer.SetTexture("texImage", 0, tex_water);
	mesh_water->draw();

	programForwardRenderer.Unuse();
}

void CMyApp::Render()
{
	// "Forward rendering": rendering the geometry into the framebuffer's attachements
	// Bind target
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	// Clear it
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// Enable depth test for this
	glEnable(GL_DEPTH_TEST);
	// Run shader program
	DrawScene();

	// Lights
	// Bind back the frontbuffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	programLightRenderer.Use();
	programLightRenderer.SetUniform("eye_pos", camera.GetEye());
	programLightRenderer.SetTexture("colorTexture", 0, colorBuffer);
	programLightRenderer.SetTexture("normalTexture", 1, normalBuffer);
	programLightRenderer.SetTexture("positionTexture", 2, positionBuffer);
	programLightRenderer.SetTexture("materialTexture", 3, materialBuffer);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	ImGui::SetNextWindowPos(ImVec2(300, 400), ImGuiSetCond_FirstUseEver);
	if (ImGui::Begin("Test window")) // Note that ImGui returns false when window is collapsed so we can early-out
	{
		ImGui::Image((ImTextureID)colorBuffer, ImVec2(256, 256), ImVec2(0, 1), ImVec2(1, 0));
		ImGui::Image((ImTextureID)normalBuffer, ImVec2(256, 256), ImVec2(0, 1), ImVec2(1, 0));
		ImGui::Image((ImTextureID)positionBuffer, ImVec2(256, 256), ImVec2(0, 1), ImVec2(1, 0));
		ImGui::Image((ImTextureID)materialBuffer, ImVec2(256, 256), ImVec2(0, 1), ImVec2(1, 0));
	}
	ImGui::End(); // In either case, ImGui::End() needs to be called for ImGui::Begin().
		// Note that other commands may work differently and may not need an End* if Begin* returned false.
}

void CMyApp::KeyboardDown(SDL_KeyboardEvent& key)
{
	camera.KeyboardDown(key);
}

void CMyApp::KeyboardUp(SDL_KeyboardEvent& key)
{
	camera.KeyboardUp(key);
}

void CMyApp::MouseMove(SDL_MouseMotionEvent& mouse)
{
	camera.MouseMove(mouse);
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
	camera.Resize(_w, _h);
	CreateForwardBuffer(_w, _h);
	std::cout << "new width = " << _w << " | new height = " << _h << "\n";
}