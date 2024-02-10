#pragma once

#include "templates.h"
#include <SDL.h>


class Window : public Wrapper<SDL_Window, decltype(&SDL_CreateWindow), decltype(&SDL_DestroyWindow), &SDL_CreateWindow, &SDL_DestroyWindow> {
public:
	Window(const char* title, engPoint2D<int> pos, engPoint2D<int> size, uint16_t flags)
		: Wrapper<SDL_Window, decltype(&SDL_CreateWindow), decltype(&SDL_DestroyWindow), &SDL_CreateWindow, &SDL_DestroyWindow>(title, pos.x, pos.y, size.x, size.y, flags)
	{}
};

using Renderer = Wrapper<SDL_Renderer, decltype(&SDL_CreateRenderer), decltype(&SDL_DestroyRenderer), &SDL_CreateRenderer, &SDL_DestroyRenderer>;