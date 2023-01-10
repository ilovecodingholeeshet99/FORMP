#include "Scene1.h"
#include "GL\glew.h"
#include "shader.hpp"
#include "Application.h"
#include "MeshBuilder.h"
#include "Mtx44.h"

using namespace std;

#define time_scale 5
#define x_resistant 1

Scene1::Scene1()
{
	m_obj = NULL;
}

Scene1::~Scene1()
{
}

void Scene1::Init()
{
	//Calculating aspect ratio
	m_worldHeight = 500.f;
	m_worldWidth = m_worldHeight * (float)Application::GetWindowWidth() / Application::GetWindowHeight();

	// Set background color to dark blue
	glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

	//Enable depth buffer and depth testing
	glEnable(GL_DEPTH_TEST);

	//Enable back face culling
	glEnable(GL_CULL_FACE);

	//Default to fill mode
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	// Generate a default VAO for now
	glGenVertexArrays(1, &m_vertexArrayID);
	glBindVertexArray(m_vertexArrayID);

	// Load the shader programs
	m_programID = LoadShaders("Shader//TransformVertexShader.vertexshader",
		"Shader//SimpleFragmentShader.fragmentshader");
	glUseProgram(m_programID);

	// Get a handle for our "MVP" uniform
	m_parameters[U_MVP] = glGetUniformLocation(m_programID, "MVP");

	camera.Init(Vector3(0, 0, 1), Vector3(0, 0, 0), Vector3(0, 1, 0));

	// Init VBO here
	for (int i = 0; i < NUM_GEOMETRY; ++i)
	{
		meshList[i] = nullptr;
	}

	meshList[GEO_AXES] = MeshBuilder::GenerateAxes("Axes", 10000.f, 10000.f, 10000.f);
	meshList[GEO_BALL] = MeshBuilder::GenerateSphere("Sphere", Color(1.0f, 1.0f, 1.0f), 10, 10, 1.f);

	// alien (spheres)
	meshList[GEO_ALIEN] = MeshBuilder::GenerateSphere("Alien", Color(1.0f, 1.0f, 0.0f), 10, 10, 1.f);

	m_obj = new GameObject(GameObject::GO_BALL);

	// position set to (half width and 10% of the screen height
	m_obj->pos = Vector3(m_worldWidth / 2.0f, m_worldHeight / 10.0f, 0.f);
	m_obj->scale = Vector3(10.f, 10.f, 1.f);
	m_obj->vel = Vector3(0.f, 0.f, 0.f);
	m_obj->active = false;
	m_obj->mass = 1;

	// loop through the array for the aliens
	for (int i = 0; i < MAX_ALIEN; ++i)
	{
		// allocate memory for each alien
		m_alien[i] = new GameObject(GameObject::GO_ALIEN);

		// randomise the position. only appear on the top half of the screen
		m_alien[i]->pos = Vector3( (float)(rand() % (int) m_worldWidth),
									(float)(rand() % (int) (m_worldHeight / 2) + (m_worldHeight / 2)),
									0.f);
		// randomise the size of the alien 
		m_alien[i]->scale = Vector3(20.f, 20.f, 1.f);
		// set active. 
		m_alien[i]->active = true;
		m_alien[i]->mass = 2;
	}
	// shifted out multiplier as it's a fixed value. can just calculate during init.
	multiplier = 1.0f / m_obj->mass;
}

void Scene1::Update(double dt)
{
	// Check for key press, you can add more for interaction
	HandleKeyPress();

	//Calculate all the forces acting on this frame
	force += left;
	force += right;

	// check if button press
	slowdown = force.IsZero();

	// check if the player is not pressing any button and the object is still moving
	if (slowdown && !m_obj->vel.IsZero())
		// add in resistance force to slowly, to simplify the whole equation, you could use -(obj->vel.x)
		// since mass is 1
		if (m_obj->vel.x > 0)	
			force.x = -x_resistant;
		else 
			force.x = x_resistant;
	//Calculate the resulting acceleration
	// F = ma.  Hence a = F/M = F * 1/M
	acc = force * multiplier;

	// set initial velocity (U)
	temp = m_obj->vel;

	//commpute the resulting velocity.
	// v = u + at
	m_obj->vel += (float)dt * time_scale * acc;
	
	// need to check if it's meant to stop the object or not
	// aka check if the player is pressing
	if (slowdown && 
		( m_obj->vel.x > 0 && temp.x < 0 ) || (m_obj->vel.x < 0 && temp.x > 0))
		m_obj->vel.SetZero();

	// s = 1/2 (u+v)t
	m_obj->pos += 0.5f * (float)dt * time_scale * (temp + m_obj->vel);
	
	// need all the forces to reset
	force.SetZero();

	// loop aliens
	for (int j = 0; j < MAX_ALIEN; ++j) {
		// check active (if inactive, it's already been shot)
		if (m_alien[j]->active) {
			if (CheckSSCollision(m_obj, m_alien[j])) {
				CollisionResponse(m_obj, m_alien[j]);
			}
			m_alien[j]->pos += (float)dt * time_scale * (m_alien[j]->vel);
		}
	}
}

bool Scene1::CheckSSCollision(const GameObject* A, const GameObject* B) // Sphere to sphere
{
	// Code the collision check here
	// Find radius of sphere A
	float radA = A->scale.x; 
	float radB = B->scale.x;
	float dist = A->scale.x - B->scale.x; // Distance including outside sphere
	return dist < radA + radB; // If true, then collided else not collided
}

void Scene1::CollisionResponse(GameObject* go1, GameObject* go2)
{
	// code the collision response here
	Vector3 u1 = go1->vel;
	Vector3 u2 = go2->vel;

	go1->vel = u2;
	go2->vel = u1;
}

void Scene1::Render()
{
	// Clear color buffer every frame
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Projection matrix : Orthographic Projection
	Mtx44 projection;
	projection.SetToOrtho(0, m_worldWidth, 0, m_worldHeight, -10, 10);
	projectionStack.LoadMatrix(projection);

	// Camera matrix
	viewStack.LoadIdentity();
	viewStack.LookAt(
		camera.position.x, camera.position.y, camera.position.z,
		camera.target.x, camera.target.y, camera.target.z,
		camera.up.x, camera.up.y, camera.up.z
	);
	// Model matrix : an identity matrix (model will be at the origin)
	modelStack.LoadIdentity();

	RenderMesh(meshList[GEO_AXES], false);

	// Render objects
	if (m_obj->active)
	{
		modelStack.PushMatrix();
		modelStack.Translate(m_obj->pos.x, m_obj->pos.y, m_obj->pos.z);
		modelStack.Scale(m_obj->scale.x, m_obj->scale.y, m_obj->scale.z);
		RenderMesh(meshList[GEO_BALL], false);
		modelStack.PopMatrix();
	}

	// loop all the aliens
	for (int i = 0; i < MAX_ALIEN; ++i)
	{
		// check if active
		if (m_alien[i]->active) {

			modelStack.PushMatrix();
			modelStack.Translate(m_alien[i]->pos.x, m_alien[i]->pos.y, m_alien[i]->pos.z);
			modelStack.Scale(m_alien[i]->scale.x, m_alien[i]->scale.y, m_alien[i]->scale.z);
			// render
			RenderMesh(meshList[GEO_ALIEN], false);
			modelStack.PopMatrix();
		}
	}

}

void Scene1::RenderMesh(Mesh* mesh, bool enableLight)
{
	Mtx44 MVP;

	MVP = projectionStack.Top() * viewStack.Top() * modelStack.Top();
	glUniformMatrix4fv(m_parameters[U_MVP], 1, GL_FALSE, &MVP.a[0]);
	mesh->Render();
}

void Scene1::Exit()
{
	// Cleanup VBO here
	for (int i = 0; i < NUM_GEOMETRY; ++i)
	{
		if (meshList[i])
		{
			delete meshList[i];
		}
	}
	glDeleteVertexArrays(1, &m_vertexArrayID);
	glDeleteProgram(m_programID);
}

void Scene1::HandleKeyPress() 
{
	double x, y, ww, wh; 
	Application::GetCursorPos(&x, &y); // Get cursor location through pass by reference
	wh = Application::GetWindowHeight(); // Store height and width in double variable
	ww = Application::GetWindowWidth();
	Vector3 mousePos(x * (m_worldWidth / ww), (wh - y) * (m_worldHeight / wh), 0);
	// Mouse click to spawn
	if (!bRButtonState && Application::IsMousePressed(1))
	{
		bRButtonState = true;
		m_obj->active = true;
		m_obj->pos.Set(x * (m_worldWidth / ww), (wh - y) * (m_worldHeight / wh), 0);
	}
	else if (bRButtonState && !Application::IsMousePressed(1))
	{
		bRButtonState = false;
		m_obj->active = false;
		m_obj->vel = m_obj->pos - mousePos;
	}
	if (Application::IsKeyPressed(0x31))
	{
		// Key press to enable culling
		glEnable(GL_CULL_FACE);
	}
	if (Application::IsKeyPressed(0x32))
	{
		// Key press to disable culling
		glDisable(GL_CULL_FACE);
	}
	if (Application::IsKeyPressed(0x33))
	{
		// Key press to enable fill mode for the polygon
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); //default fill mode
	}
	if (Application::IsKeyPressed(0x34))
	{
		// Key press to enable wireframe mode for the polygon
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); //wireframe mode
	}

	// Spacebar button
	if (Application::IsKeyPressed(0x20))
	{
		if (!UPButton) {
		}
		UPButton = true;
	}
	else UPButton = false;

	// Left button
	if (Application::IsKeyPressed(0x25))
		left.x = -1.f;
	else
		left.x = 0.f;

	// Right button
	if (Application::IsKeyPressed(0x27))
		right.x = 1.f;
	else
		right.x = 0.f;
}
