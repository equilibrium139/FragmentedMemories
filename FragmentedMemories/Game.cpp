#include "Game.h"

bool Game::UpdateAndRender(GameInput& input, float dt, float time)
{
	if (input.start)
	{
		return false;
	}
	if (input.moveUp) 
	{
		scene.camera.ProcessKeyboard(CAM_FORWARD, dt);
	}
	if (input.moveDown) 
	{
		scene.camera.ProcessKeyboard(CAM_BACKWARD, dt);
	}
	if (input.moveLeft) 
	{
		scene.camera.ProcessKeyboard(CAM_LEFT, dt);
	}
	if (input.moveRight) 
	{
		scene.camera.ProcessKeyboard(CAM_RIGHT, dt);
	}
	scene.camera.ProcessMouseMovement(input.cursorDeltaX, -input.cursorDeltaY);

	renderer.Draw(scene, dt, input.windowWidth, input.windowHeight);

	return true;
}