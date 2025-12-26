#define GLM_ENABLE_EXPERIMENTAL
#include "gameLayer.h"
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include "platformInput.h"
#include <iostream>
#include <sstream>
#include <platformTools.h>
#include <logs.h>
#include <SDL3/SDL.h>
#include <gl2d/gl2d.h>

#include <imguiTools.h>


struct GameData
{
	glm::vec2 rectPos = {0,0};

}gameData;


gl2d::Renderer2D renderer;
gl2d::Camera camera;
gl2d::Texture texture;
gl2d::Font font;

bool initGame(SDL_Renderer *sdlRenderer)
{

	gl2d::init();


	renderer.create(sdlRenderer);

	texture.loadFromFile(RESOURCES_PATH "test.jpg");

	font.createFromFile(RESOURCES_PATH "roboto_black.ttf");

	std::cout << platform::getControllerName(0) << "\n";
	std::cout << platform::getControllerType(0) << "\n";


	return true;
}


//IMPORTANT NOTICE, IF YOU WANT TO SHIP THE GAME TO ANOTHER PC READ THE README.MD IN THE GITHUB
//https://github.com/meemknight/SDL3Setup
//OR THE INSTRUCTION IN THE CMAKE FILE.
//YOU HAVE TO CHANGE A FLAG IN THE CMAKE SO THAT RESOURCES_PATH POINTS TO RELATIVE PATHS
//BECAUSE OF SOME CMAKE PROGBLMS, RESOURCES_PATH IS SET TO BE ABSOLUTE DURING PRODUCTION FOR MAKING IT EASIER.

bool gameLogic(float deltaTime, platform::Input &input, SDL_Renderer *sdlRenderer)
{
#pragma region init stuff
	int w = 0; int h = 0;
	w = platform::getFrameBufferSizeX(); //window w
	h = platform::getFrameBufferSizeY(); //window h
	
	renderer.updateWindowMetrics(w, h);

	renderer.clearScreen(Colors_Orange);

#pragma endregion

	glm::vec2 dir = {};
	dir = input.controller.LStick;
	dir.y *= -1;

	//you can also do platform::isButtonHeld(platform::Button::Left)
	if (input.isButtonHeld(platform::Button::Left))
	{
		dir.x -= 1;
	}
	if (input.isButtonHeld(platform::Button::Right))
	{
		dir.x += 1;
	}
	if (input.isButtonHeld(platform::Button::Up))
	{
		dir.y -= 1;
	}
	if (input.isButtonHeld(platform::Button::Down))
	{
		dir.y += 1;
	}

	if (glm::length(dir) != 0)
	{
		dir = glm::normalize(dir);
		dir *= deltaTime * 100;
		gameData.rectPos += dir;
	}



#if REMOVE_IMGUI == 0
	//ImGui::ShowDemoWindow();
	ImGui::PushMakeWindowNotTransparent();
	ImGui::Begin("Test Imgui");

	ImGui::DragFloat2("Positions", &gameData.rectPos[0]);


	ImGui::helpMarker("test");

	ImGui::addErrorSymbol();
	ImGui::addWarningSymbol();

	ImGui::PopMakeWindowNotTransparent();
	ImGui::End();
#endif

	renderer.renderRectangle({gameData.rectPos, 100, 100}, texture);

	renderer.renderText({200,200}, "Test", font, Colors_White);

	renderer.flush();



	return true;
#pragma endregion

}

//This function might not be be called if the program is forced closed
void closeGame()
{



}
