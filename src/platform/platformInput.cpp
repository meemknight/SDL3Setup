#include "platformInput.h"
#include "gameLayer.h"
#include <SDL3/SDL.h>

platform::Button keyBoard[platform::Button::BUTTONS_COUNT];
platform::Button leftMouse;
platform::Button rightMouse;

platform::Controller controller = {};
platform::Controller controllers[4] = {};
std::string typedInput;

platform::Button *platform::getAllButtons()
{
	return keyBoard;
}

platform::Button &platform::getLMouseButton()
{
	return leftMouse;
}

platform::Button &platform::getRMouseButton()
{
	return rightMouse;
}

int platform::isButtonHeld(int key)
{
	if (key < Button::A || key >= Button::BUTTONS_COUNT) { return 0; }

	return keyBoard[key].held;
}

int platform::isButtonPressed(int key)
{
	if (key < Button::A || key >= Button::BUTTONS_COUNT) { return 0; }

	return keyBoard[key].pressed;
}

int platform::isButtonReleased(int key)
{
	if (key < Button::A || key >= Button::BUTTONS_COUNT) { return 0; }

	return keyBoard[key].released;
}

int platform::isButtonTyped(int key)
{
	if (key < Button::A || key >= Button::BUTTONS_COUNT) { return 0; }

	return keyBoard[key].typed;
}

int platform::isLMousePressed()
{
	return leftMouse.pressed;
}

int platform::isRMousePressed()
{
	return rightMouse.pressed;
}

int platform::isLMouseReleased()
{
	return leftMouse.released;
}

int platform::isRMouseReleased()
{
	return rightMouse.released;
}


int platform::isLMouseHeld()
{
	return leftMouse.held;
}

int platform::isRMouseHeld()
{
	return rightMouse.held;
}

platform::Controller platform::getControllerButtons()
{
	return platform::hasFocused() ? controller : platform::Controller{};
}

platform::Controller platform::getControllerButtonsAtIndex(int i)
{
	if (i < 0 || i > 3) { return {}; }

	return platform::hasFocused() ? controllers[i] : platform::Controller{};
}

std::string platform::getTypedInput()
{
	return typedInput;
}

void platform::internal::setButtonState(int button, int newState)
{

	processEventButton(keyBoard[button], newState);

}

void platform::internal::setLeftMouseState(int newState)
{
	processEventButton(leftMouse, newState);

}

void platform::internal::setRightMouseState(int newState)
{
	processEventButton(rightMouse, newState);

}

static SDL_GamepadButton ToSDLButton(platform::Controller::Buttons b)
{
	switch (b)
	{
		case platform::Controller::A:       return SDL_GAMEPAD_BUTTON_SOUTH;
		case platform::Controller::B:       return SDL_GAMEPAD_BUTTON_EAST;
		case platform::Controller::X:       return SDL_GAMEPAD_BUTTON_WEST;
		case platform::Controller::Y:       return SDL_GAMEPAD_BUTTON_NORTH;

		case platform::Controller::LBumper: return SDL_GAMEPAD_BUTTON_LEFT_SHOULDER;
		case platform::Controller::RBumper: return SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER;

		case platform::Controller::Back:    return SDL_GAMEPAD_BUTTON_BACK;
		case platform::Controller::Start:   return SDL_GAMEPAD_BUTTON_START;
		case platform::Controller::Guide:   return SDL_GAMEPAD_BUTTON_GUIDE;

		case platform::Controller::LThumb:  return SDL_GAMEPAD_BUTTON_LEFT_STICK;
		case platform::Controller::RThumb:  return SDL_GAMEPAD_BUTTON_RIGHT_STICK;

		case platform::Controller::Up:      return SDL_GAMEPAD_BUTTON_DPAD_UP;
		case platform::Controller::Right:   return SDL_GAMEPAD_BUTTON_DPAD_RIGHT;
		case platform::Controller::Down:    return SDL_GAMEPAD_BUTTON_DPAD_DOWN;
		case platform::Controller::Left:    return SDL_GAMEPAD_BUTTON_DPAD_LEFT;

		default:                  return SDL_GAMEPAD_BUTTON_INVALID;
	}
}

static float NormalizeAxisS16(Sint16 v)
{
	// SDL axes are typically Sint16 [-32768..32767]
	if (v >= 0) return (float)v / 32767.0f;
	return (float)v / 32768.0f;
}

static float ApplyDeadzone(float x, float dz = 0.15f)
{
	if (fabsf(x) <= dz) return 0.f;
	// simple rescale to keep full range after deadzone
	const float s = (fabsf(x) - dz) / (1.f - dz);
	return (x < 0.f) ? -s : s;
}


static void MergeIntoAggregate(platform::Controller &agg, const platform::Controller &src)
{
	// Digital: pressed if ANY controller pressed
	for (int i = 0; i < platform::Controller::ButtonCount; i++)
	{
		if (src.buttons[i].held) agg.buttons[i].held = true;
		if (src.buttons[i].pressed) agg.buttons[i].pressed = true;
		if (src.buttons[i].released) agg.buttons[i].released = true;
	}

	// Analog: take max magnitude (common & feels right)
	agg.LT = (src.LT > agg.LT) ? src.LT : agg.LT;
	agg.RT = (src.RT > agg.RT) ? src.RT : agg.RT;

	auto pickMaxMag = [](float a, float b) { return (fabsf(b) > fabsf(a)) ? b : a; };

	agg.LStick.x = pickMaxMag(agg.LStick.x, src.LStick.x);
	agg.LStick.y = pickMaxMag(agg.LStick.y, src.LStick.y);
	agg.RStick.x = pickMaxMag(agg.RStick.x, src.RStick.x);
	agg.RStick.y = pickMaxMag(agg.RStick.y, src.RStick.y);

}

void platform::internal::UpdateControllersSDL3(float deltaTime)
{
	// reset outputs
	controller.setAllToZero();
	for (int i = 0; i < 4; i++) controllers[i].setAllToZero();

	int count = 0;
	SDL_JoystickID *ids = SDL_GetGamepads(&count);  // free with SDL_free :contentReference[oaicite:4]{index=4}
	if (!ids || count <= 0)
	{
		if (ids) SDL_free(ids);
		return;
	}

	const int used = (count < 4) ? count : 4;

	for (int padIndex = 0; padIndex < used; padIndex++)
	{
		SDL_Gamepad *pad = SDL_OpenGamepad(ids[padIndex]);  // :contentReference[oaicite:5]{index=5}
		if (!pad) continue;

		Controller &c = controllers[padIndex];

		// Buttons -> your event pipeline
		for (int b = 0; b < Controller::ButtonCount; b++)
		{
			const SDL_GamepadButton sb = ToSDLButton((Controller::Buttons)b);
			const bool down = (sb != SDL_GAMEPAD_BUTTON_INVALID) && SDL_GetGamepadButton(pad, sb);

			// mirror your old logic:
			processEventButton(c.buttons[b], down ? 1 : 0);
			updateButton(c.buttons[b], deltaTime);
		}

		// Axes
		const float lx = ApplyDeadzone(NormalizeAxisS16(SDL_GetGamepadAxis(pad, SDL_GAMEPAD_AXIS_LEFTX)));
		const float ly = ApplyDeadzone(NormalizeAxisS16(SDL_GetGamepadAxis(pad, SDL_GAMEPAD_AXIS_LEFTY)));
		const float rx = ApplyDeadzone(NormalizeAxisS16(SDL_GetGamepadAxis(pad, SDL_GAMEPAD_AXIS_RIGHTX)));
		const float ry = ApplyDeadzone(NormalizeAxisS16(SDL_GetGamepadAxis(pad, SDL_GAMEPAD_AXIS_RIGHTY)));

		// Triggers are usually [0..1] (some drivers still give [-1..1], normalize if needed)
		float lt = NormalizeAxisS16(SDL_GetGamepadAxis(pad, SDL_GAMEPAD_AXIS_LEFT_TRIGGER));
		float rt = NormalizeAxisS16(SDL_GetGamepadAxis(pad, SDL_GAMEPAD_AXIS_RIGHT_TRIGGER));
		lt = (lt + 1.f) * 0.5f;
		rt = (rt + 1.f) * 0.5f;

		c.LStick = {lx, ly};
		c.RStick = {rx, ry};
		c.LT = (lt < 0.f) ? 0.f : ((lt > 1.f) ? 1.f : lt);
		c.RT = (rt < 0.f) ? 0.f : ((rt > 1.f) ? 1.f : rt);

		// merge into amalgamation
		MergeIntoAggregate(controller, c);

		SDL_CloseGamepad(pad); // :contentReference[oaicite:6]{index=6}
	}

	SDL_free(ids);
}

void platform::internal::updateAllButtons(float deltaTime)
{
	for (int i = 0; i < platform::Button::BUTTONS_COUNT; i++)
	{
		updateButton(keyBoard[i], deltaTime);
	}

	updateButton(leftMouse, deltaTime);
	updateButton(rightMouse, deltaTime);
	
	UpdateControllersSDL3(deltaTime);

}

void platform::internal::resetInputsToZero()
{
	resetTypedInput();

	for (int i = 0; i < platform::Button::BUTTONS_COUNT; i++)
	{
		resetButtonToZero(keyBoard[i]);
	}

	resetButtonToZero(leftMouse);
	resetButtonToZero(rightMouse);
	
	//TODO SDL3
	//controllerButtons.setAllToZero();
}

void platform::internal::addToTypedInput(char c)
{
	typedInput += c;
}

void platform::internal::resetTypedInput()
{
	typedInput.clear();
}
