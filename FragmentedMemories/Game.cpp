#include "Game.h"

GameOutput Game::UpdateAndRender(GameInput& input, float dt, float time)
{
	GameOutput output;
	output.quit = false;

	if (input.start)
	{
		output.quit = true;
		return output;
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

	return output;
}