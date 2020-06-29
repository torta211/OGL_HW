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
	t = 0.0f;
	frameBufferCreated = false;
	frozen = false;
	width = w_init;
	height = h_init;
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
	mesh_terrain = ObjParser::parse("terrain.obj");
	tex_terrain.FromFile("sand.jpg");
	std::cout << "terrain assets loaded\n";

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

void CMyApp::CreateFrameBuffers()
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
		glDeleteTextures(1, &shadow_depth_texture);
		glDeleteFramebuffers(1, &shadow_fbo);
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

	// Now the fbo to render from the light
	glGenFramebuffers(1, &shadow_fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, shadow_fbo);

	glGenTextures(1, &shadow_depth_texture);
	glBindTexture(GL_TEXTURE_2D, shadow_depth_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, DIR_SHADOW_MAP_RES, DIR_SHADOW_MAP_RES, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
	setTexture2DParameters(GL_NEAREST, GL_NEAREST);

	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, shadow_depth_texture, 0);
	if (glGetError() != GL_NO_ERROR) {
		std::cout << "Error creating depth attachment for shadow map" << std::endl;
		exit(1);
	}

	// No need for any color output!
	glDrawBuffer(GL_NONE);
	   		
	// Completeness check
	status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
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

	// Unbind
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	frameBufferCreated = true;
}

bool CMyApp::Init()
{	
	// Set clear color
	glClearColor(0.0f, 0.0f, 0.0f, 1);
	// For this scene we just keep all faces
	//glEnable(GL_CULL_FACE);
		
	programShadowMapper.Init({ // Shadow shader
		{ GL_VERTEX_SHADER,		"shadow_map.vert" },
		{ GL_FRAGMENT_SHADER,	"shadow_map.frag" }
	});

	programForwardRenderer.Init({
		{ GL_VERTEX_SHADER,		"forward.vert" },
		{ GL_FRAGMENT_SHADER,	"forward.frag" }
	});

	programLightRenderer.Init({
		{ GL_VERTEX_SHADER,		"deferredPoint.vert" },
		{ GL_FRAGMENT_SHADER,	"deferredPoint.frag" }
	});

	programLightSpheres.Init({
		{ GL_VERTEX_SHADER,			"sphere.vert" },
		{ GL_TESS_CONTROL_SHADER,	"sphere.tcs" },
		{ GL_TESS_EVALUATION_SHADER,"sphere.tes" },
		{ GL_FRAGMENT_SHADER,		"sphere.frag" }
	});

	programDirectionalLight.Init({
		{ GL_VERTEX_SHADER,		"directionalLight.vert" },
		{ GL_FRAGMENT_SHADER,	"directionalLight.frag" }
	});

	LoadAssets();

	// Create point lights
	for (int i = 0; i < NUM_POINT_LIGHTS; ++i)
	{
		pointLightPositions.push_back(glm::vec3(rand() / (double)RAND_MAX * 700.0f - 350.0f,
												rand() / (double)RAND_MAX * 100.0f + 35.0f,
												rand() / (double)RAND_MAX * 700.0f - 350.0f));
		pointLightNextPositions.push_back(glm::vec3(rand() / (double)RAND_MAX * 700.0f - 350.0f,
													rand() / (double)RAND_MAX * 100.0f + 35.0f,
													rand() / (double)RAND_MAX * 700.0f - 350.0f));
		pointLightColors.push_back(glm::vec3(rand() / (double)RAND_MAX * 0.5f + 0.5f,
											 rand() / (double)RAND_MAX * 0.5f + 0.5f,
											 rand() / (double)RAND_MAX * 0.5f + 0.5f));
		pointLightStrengths.push_back(rand() / (double)RAND_MAX * 1.5f + 0.5f);
	}

	CreateFrameBuffers();

	return true;
}


void CMyApp::Clean()
{
	if (frameBufferCreated)
	{
		glDeleteTextures(1, &colorBuffer);
		glDeleteTextures(1, &normalBuffer);
		glDeleteTextures(1, &positionBuffer);
		glDeleteTextures(1, &materialBuffer);
		glDeleteRenderbuffers(1, &depthBuffer);
		glDeleteFramebuffers(1, &fbo);
		glDeleteTextures(1, &shadow_depth_texture);
		glDeleteFramebuffers(1, &shadow_fbo);
	}
}

void CMyApp::Update()
{
	// static declaration runs only once per application
	static Uint32 last_time = SDL_GetTicks();
	delta_time = (SDL_GetTicks() - last_time) / 1000.0f;

	camera.Update(static_cast<float>(delta_time));

	if (!frozen)
	{
		// Move point lights
		for (int i = 0; i < NUM_POINT_LIGHTS; ++i)
		{
			glm::vec3 toGoal = pointLightNextPositions[i] - pointLightPositions[i];
			if (glm::length(toGoal) < 2.0f)
			{
				pointLightNextPositions[i] = glm::vec3(rand() / (double)RAND_MAX * 700.0f - 350.0f,
					rand() / (double)RAND_MAX * 100.0f + 35.0f,
					rand() / (double)RAND_MAX * 700.0f - 350.0f);
				toGoal = pointLightNextPositions[i] - pointLightPositions[i];
			}
			pointLightPositions[i] += glm::normalize(toGoal);
		}
		// Increment elapsed time
		t += delta_time;
	}

	last_time = SDL_GetTicks();
}

void CMyApp::DrawScene(glm::mat4 waterLevel)
{
	programForwardRenderer.Use();

	programForwardRenderer.SetUniform("world", glm::mat4());
	programForwardRenderer.SetUniform("MVP", camera.GetViewProj());
	programForwardRenderer.SetUniform("worldIT", glm::mat4());
	programForwardRenderer.SetUniform("eye_pos", camera.GetEye());
	programForwardRenderer.SetUniform("Ka", 0.5f);
	programForwardRenderer.SetUniform("Kd", 0.6f);
	programForwardRenderer.SetUniform("Ks", 0.1f);
	programForwardRenderer.SetUniform("specular_power", 50.0f);

	programForwardRenderer.SetTexture("texImage", 0, tex_terrain);
	mesh_terrain->draw();

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

	programForwardRenderer.SetUniform("Kd", 0.8f);
	programForwardRenderer.SetUniform("Ks", 0.8f);
	programForwardRenderer.SetUniform("specular_power", 30.0f);
	programForwardRenderer.SetUniform("world", waterLevel);
	programForwardRenderer.SetUniform("MVP", camera.GetViewProj() * waterLevel);
	programForwardRenderer.SetUniform("worldIT", glm::transpose(glm::inverse(waterLevel)));
	programForwardRenderer.SetTexture("texImage", 0, tex_water);
	mesh_water->draw();

	programForwardRenderer.Unuse();

	// put on lights
	programLightSpheres.Use();
	programLightSpheres.SetUniform("MVP", camera.GetViewProj());
	programLightSpheres.SetUniform("tess_level", 15.0f);
	programLightSpheres.SetUniform("eye_pos", camera.GetEye());
	glUniform3fv(glGetUniformLocation(programLightSpheres, "lightPoints"), NUM_POINT_LIGHTS, glm::value_ptr(pointLightPositions.front()));
	glUniform1fv(glGetUniformLocation(programLightSpheres, "lightRads"), NUM_POINT_LIGHTS, &pointLightStrengths.front());
	glUniform3fv(glGetUniformLocation(programLightSpheres, "lightColors"), NUM_POINT_LIGHTS, glm::value_ptr(pointLightColors.front()));
	glPatchParameteri(GL_PATCH_VERTICES, 1);
	glDrawArrays(GL_PATCHES, 0, NUM_POINT_LIGHTS);
	programLightSpheres.Unuse();
}

void CMyApp::Render()
{
	// Update dynamic parameter of scene
	glm::mat4 waterLevel = glm::translate(glm::vec3(0, 5 * sin(t), 0));
	// "Forward rendering": rendering the geometry into the framebuffer's attachements
	// Bind target
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	// Enable depth test for this
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glDisable(GL_BLEND);
	// Clear it
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// Run shader program
	DrawScene(waterLevel);

	// Create a depth map from the direction of the main light
	// Bind target
	glBindFramebuffer(GL_FRAMEBUFFER, shadow_fbo);
	// This has a custom resolution
	glViewport(0, 0, DIR_SHADOW_MAP_RES, DIR_SHADOW_MAP_RES);
	// Clear the previous frame's shadow depth info
	glClear(GL_DEPTH_BUFFER_BIT);
	// Specify the directional light
	glm::vec3 m_light_dir = glm::normalize(glm::vec3(0, -1, -1));
	glm::mat4 m_light_proj = glm::ortho<float>(-500, 500, -300, 300, 0, 1000);
	glm::mat4 m_light_view = glm::lookAt<float>(glm::vec3(400, 190, 250), m_light_dir, glm::vec3(0, 1, 0));
	glm::mat4 m_light_vp = m_light_proj * m_light_view;
	// Shadow map program
	programShadowMapper.Use();
	programShadowMapper.SetUniform("MVP", m_light_vp * glm::mat4(1));
	mesh_terrain->draw();
	mesh_grass->draw();
	mesh_leaves->draw();
	mesh_stems->draw();
	mesh_plants->draw();
	mesh_rocks->draw();
	programShadowMapper.SetUniform("MVP", m_light_vp * glm::mat4(1) * waterLevel);
	mesh_water->draw();
	programShadowMapper.Unuse();

	// -- Lights
	// Bind back the frontbuffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	// Set resolution back
	glViewport(0, 0, width, height);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// We will add two fullscreen quads
	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);
	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_ONE, GL_ONE);

	// Add the effect of the directional light
	programDirectionalLight.Use();
	programDirectionalLight.SetUniform("eye_pos", camera.GetEye());
	programDirectionalLight.SetUniform("shadowVP", m_light_vp * glm::mat4(1));
	programDirectionalLight.SetTexture("colorTexture", 0, colorBuffer);
	programDirectionalLight.SetTexture("normalTexture", 1, normalBuffer);
	programDirectionalLight.SetTexture("positionTexture", 2, positionBuffer);
	programDirectionalLight.SetTexture("materialTexture", 3, materialBuffer);
	programDirectionalLight.SetTexture("shadowDepthTexture", 4, shadow_depth_texture);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	programDirectionalLight.Unuse();

	// Add the effect of the point lights
	programLightRenderer.Use();
	glUniform3fv(glGetUniformLocation(programLightRenderer, "lightPositions"), NUM_POINT_LIGHTS, glm::value_ptr(pointLightPositions.front()));
	glUniform1fv(glGetUniformLocation(programLightRenderer, "lightStrengths"), NUM_POINT_LIGHTS, &pointLightStrengths.front());
	glUniform3fv(glGetUniformLocation(programLightRenderer, "lightColors"), NUM_POINT_LIGHTS, glm::value_ptr(pointLightColors.front()));
	programLightRenderer.SetUniform("eye_pos", camera.GetEye());
	programLightRenderer.SetTexture("colorTexture", 0, colorBuffer);
	programLightRenderer.SetTexture("normalTexture", 1, normalBuffer);
	programLightRenderer.SetTexture("positionTexture", 2, positionBuffer);
	programLightRenderer.SetTexture("materialTexture", 3, materialBuffer);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	programLightRenderer.Unuse();

	if (ImGui::Begin("Base Color"))
	{
		ImGui::Image((ImTextureID)colorBuffer, ImVec2(256, 256), ImVec2(0, 1), ImVec2(1, 0));
	}
	ImGui::End();

	if (ImGui::Begin("Normal Vectors"))
	{
		ImGui::Image((ImTextureID)normalBuffer, ImVec2(256, 256), ImVec2(0, 1), ImVec2(1, 0));
	}
	ImGui::End();

	if (ImGui::Begin("Depth from dir. light"))
	{
		ImGui::Image((ImTextureID)shadow_depth_texture, ImVec2(256, 256), ImVec2(0, 1), ImVec2(1, 0));
	}
	ImGui::End();
}

void CMyApp::KeyboardDown(SDL_KeyboardEvent& key)
{
	if (key.keysym.sym == SDLK_f)
	{
		frozen = !frozen;
	}
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
	width = _w;
	height = _h;
	CreateFrameBuffers();
	std::cout << "new width = " << _w << " | new height = " << _h << "\n";
}