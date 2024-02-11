#pragma once

#include "templates.h"
#include <SDL.h>
#include <vector>
#include <memory>


constexpr float maxFps = 55; // 0 obviously means unlimited
constexpr float expectedFrameDur = maxFps <= 0 ? 0 : 1000.f / maxFps;
extern engPoint2D<int> engScreenSize;

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
      elapsedTime(0.f),
      fTheta(0)
   {
      activeRenderers.push_back(this);
      window = w;
   }

   void DoRender();
   void OnStart();

   engMesh meshCube;
   mat4x4 matProj;
   float fTheta;

   engPoint3D<float> camPos;

private:

   void MultiplyMatrixVector(const engPoint3D<float>& i, engPoint3D<float>& o, mat4x4& m);
   void DrawTriangle(const engTriangle<float> t, Uint8 r, Uint8 g, Uint8 b, Uint8 a);

   // ENSURE PLZ THAT packed is [0, 1] IM TOO LAZY TO CHECK IT
   Uint8 Unpack(float packed) {
      return 255.f * packed;
   }

private:
   int frameCount;
   float elapsedTime; // milliseconds

   Window* window;
};
