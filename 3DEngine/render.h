#pragma once

#include "templates.h"
#include <SDL.h>
#include <vector>
#include <memory>

constexpr double howMuchNsInAMs = 1.0E6F;
constexpr double howMuchNsInASec = 1.0E9F;
constexpr float maxFps = 60.f; // 0 obviously means unlimited
constexpr double expectedFrameDurNs = maxFps <= 0 ? 0 : howMuchNsInASec / maxFps;
extern engPoint2D<int> engScreenSize;

void StartRenderLoop();


using WindowBase = Wrapper<SDL_Window, decltype(&SDL_CreateWindow), decltype(&SDL_DestroyWindow), &SDL_CreateWindow, &SDL_DestroyWindow>;

class Window : public WindowBase {
public:
   Window(const char* title, engPoint2D<int> pos, engPoint2D<int> size, Uint32 flags)
      : WindowBase(title, pos.x, pos.y, size.x, size.y, flags)
   {}
};


struct CameraMovement
{
   bool left = false;
   bool up = false;
   bool right = false;
   bool down = false;
   bool forward = false;
   bool backward = false;

   float speed = 5.f / 1.0E9F;
   
   engPoint3D<float> GetOffset(long long elapsed) const
   {
      return {
         speed * left * elapsed - speed * right * elapsed, // x
         -speed * down * elapsed + speed * up * elapsed, // y
         -speed * backward * elapsed + speed * forward * elapsed, // z
      };
   }
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

   CameraMovement camMovement;
   engPoint3D<float> camPos;
   engPoint3D<float> lookDir;

private:

   void DrawTriangle(const engTriangle<float> t);

   // ENSURE PLZ THAT packed is [0, 1] IM TOO LAZY TO CHECK IT
   Uint8 Unpack(float packed) {
      return 255.f * packed;
   }

private:
   int frameCount;
   long long elapsedTime; // nanoseconds (YES)

   Window* window;
};