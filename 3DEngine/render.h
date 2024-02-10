#pragma once

#include "templates.h"
#include <SDL.h>
#include <vector>
#include <memory>


constexpr float maxFps = 60; // 0 obviously means unlimited
constexpr float expectedFrameDur = maxFps <= 0 ? 0 : 1000.f / maxFps;

void StartRenderLoop();


using WindowBase = Wrapper<SDL_Window, decltype(&SDL_CreateWindow), decltype(&SDL_DestroyWindow), &SDL_CreateWindow, &SDL_DestroyWindow>;

class Window : public WindowBase {
public:
	Window(const char* title, engPoint2D<int> pos, engPoint2D<int> size, Uint32 flags)
		: WindowBase(title, pos.x, pos.y, size.x, size.y, flags)
	{}
};


using RendererBase = Wrapper<SDL_Renderer, decltype(&SDL_CreateRenderer), decltype(&SDL_DestroyRenderer), &SDL_CreateRenderer, &SDL_DestroyRenderer>;

class Renderer : public RendererBase {
public:
	static std::vector<Renderer*> activeRenderers;

public:
	Renderer(Window* w, int index, Uint32 flags) :
		RendererBase(w->Get(), index, flags),
		frameCount(0),
		elapsedTime(0.f)
	{
		activeRenderers.push_back(this);
		window = w;
	}

	void DoRender();

private:
	int frameCount;
	float elapsedTime; // milliseconds

	Window* window;
};
