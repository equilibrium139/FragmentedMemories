#ifndef GAME_H
#define GAME_h

#include "Scene.h"
#include "Renderer.h"

struct GameInput
{
	double cursorX, cursorY;
	double cursorDeltaX, cursorDeltaY;

	int windowWidth, windowHeight;

	bool moveUp;
	bool moveDown;
	bool moveRight;
	bool moveLeft;

	bool start;
};

struct Game
{
	bool UpdateAndRender(GameInput& input, float dt, float time);

	Scene scene;
	Renderer renderer;
};

#endif // !GAME_H
