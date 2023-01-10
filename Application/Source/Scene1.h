#ifndef SCENE_1_H
#define SCENE_1_H

#include "GameObject.h"
#include "Vector3.h"
#include "Scene.h"
#include "Mesh.h"
#include "MatrixStack.h"
#include "Camera.h"

#define MAX_ALIEN 5		// maximum aliens to have in the game

class Scene1 : public Scene
{
public:
	enum GEOMETRY_TYPE
	{
		GEO_AXES,
		GEO_TEXT,
		GEO_BALL,
		GEO_CUBE,
		GEO_BULLET,
		GEO_ALIEN, // to create alien mesh
		NUM_GEOMETRY,
	};

	enum UNIFORM_TYPE
	{
		U_MVP = 0,
		U_TOTAL,
	};

	Scene1();
	~Scene1();

	virtual void Init();
	virtual void Update(double dt);
	virtual void Render();
	virtual void Exit();

private:
	void HandleKeyPress();
	void RenderMesh(Mesh* mesh, bool enableLight);
	bool CheckSSCollision(const GameObject* A,const GameObject* B);
	void CollisionResponse(GameObject* go1, GameObject* go2);

	unsigned m_vertexArrayID;
	Mesh* meshList[NUM_GEOMETRY];

	unsigned m_programID;
	unsigned m_parameters[U_TOTAL];

	float m_worldWidth;
	float m_worldHeight;
	GameObject* m_obj;
	GameObject* m_alien[MAX_ALIEN];    // array to store the aliens
	bool bRButtonState;
	Camera camera;

	MS modelStack;
	MS viewStack;
	MS projectionStack;

	//Vector3 gravity;
	Vector3 temp, acc, force;
	Vector3 up, down, left, right;
	Vector3 diff;
	bool slowdown;
	bool UPButton;
	float multiplier, distX, distY;

	Vector3 A, B, AB, BE, AE;
	float dist;
};

#endif