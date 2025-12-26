#define GLM_ENABLE_EXPERIMENTAL
#include "gameLayer.h"
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include "platformInput.h"
#include "imgui.h"
#include <iostream>
#include <sstream>
#include <platformTools.h>
#include <imguiTools.h>
#include <logs.h>
#include <SDL3/SDL.h>
#include <gl2d/gl2d.h>


struct GameData
{
	glm::vec2 rectPos = {100,100};

}gameData;


gl2d::Renderer2D renderer;
gl2d::Camera camera;
gl2d::Texture texture;

bool initGame(SDL_Renderer *sdlRenderer)
{

	gl2d::init();


	renderer.create(sdlRenderer);

	texture.loadFromFile(RESOURCES_PATH "panda.png");


	return true;
}


//IMPORTANT NOTICE, IF YOU WANT TO SHIP THE GAME TO ANOTHER PC READ THE README.MD IN THE GITHUB
//https://github.com/meemknight/cmakeSetup
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

	//you can also do platform::isButtonHeld(platform::Button::Left)

	if (input.isButtonHeld(platform::Button::Left))
	{
		gameData.rectPos.x -= deltaTime * 100;
	}
	if (input.isButtonHeld(platform::Button::Right))
	{
		gameData.rectPos.x += deltaTime * 100;
	}
	if (input.isButtonHeld(platform::Button::Up))
	{
		gameData.rectPos.y -= deltaTime * 100;
	}
	if (input.isButtonHeld(platform::Button::Down))
	{
		gameData.rectPos.y += deltaTime * 100;
	}


	//ImGui::ShowDemoWindow();
	ImGui::PushMakeWindowNotTransparent();
	ImGui::Begin("Test Imgui");

	ImGui::DragFloat2("Positions", &gameData.rectPos[0]);


	ImGui::helpMarker("test");

	ImGui::addErrorSymbol();
	ImGui::addWarningSymbol();

	ImGui::PopMakeWindowNotTransparent();
	ImGui::End();




	renderer.renderRectangle({gameData.rectPos, 100, 100}, texture);



	renderer.flush();


	//SDL_FRect square = {};
	//square.x = gameData.rectPos.x;
	//square.y = gameData.rectPos.y;
	//square.w = 100;
	//square.h = 100;
	//SDL_SetRenderDrawColor(sdlRenderer, 255, 0, 0, 255);
	//SDL_RenderFillRect(sdlRenderer, &square);


	return true;
#pragma endregion

}

//This function might not be be called if the program is forced closed
void closeGame()
{



}
