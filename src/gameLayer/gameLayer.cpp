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

#include <algorithm> // std::clamp

static void DrawStickGrid(const char *label, float x, float y, float size = 140.0f)
{
	// clamp to expected range
	x = std::clamp(x, -1.0f, 1.0f);
	y = std::clamp(y, -1.0f, 1.0f);

	ImGui::TextUnformatted(label);

	ImVec2 p0 = ImGui::GetCursorScreenPos();
	ImVec2 p1 = ImVec2(p0.x + size, p0.y + size);

	ImGui::InvisibleButton(label, ImVec2(size, size));

	ImDrawList *dl = ImGui::GetWindowDrawList();

	// background + border
	dl->AddRectFilled(p0, p1, IM_COL32(20, 20, 20, 255));
	dl->AddRect(p0, p1, IM_COL32(120, 120, 120, 255));

	// grid lines
	for (int i = 1; i < 4; i++)
	{
		float t = i / 4.0f;
		float gx = p0.x + t * size;
		float gy = p0.y + t * size;
		dl->AddLine(ImVec2(gx, p0.y), ImVec2(gx, p1.y), IM_COL32(60, 60, 60, 255));
		dl->AddLine(ImVec2(p0.x, gy), ImVec2(p1.x, gy), IM_COL32(60, 60, 60, 255));
	}

	// axes (center cross)
	ImVec2 c((p0.x + p1.x) * 0.5f, (p0.y + p1.y) * 0.5f);
	dl->AddLine(ImVec2(c.x, p0.y), ImVec2(c.x, p1.y), IM_COL32(90, 90, 90, 255));
	dl->AddLine(ImVec2(p0.x, c.y), ImVec2(p1.x, c.y), IM_COL32(90, 90, 90, 255));

	// map stick space to widget space
	// x: -1..1 -> left..right
	// y: -1..1 -> up..down  (invert if you want up=+1)
	float px = c.x + x * (size * 0.5f);
	float py = c.y + y * (size * 0.5f);

	dl->AddCircleFilled(ImVec2(px, py), 5.0f, IM_COL32(255, 255, 255, 255));

	ImGui::Text("x: %.3f  y: %.3f", x, y);
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

	DrawStickGrid("LStick", input.controller.LStick.x, input.controller.LStick.y);
	DrawStickGrid("RStick", input.controller.RStick.x, input.controller.RStick.y);

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
