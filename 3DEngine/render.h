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
      : WindowBase(title, pos.x, pos.y, size.x, size.y, flags),
   size(size)
   {}

   engPoint2D<int> size;
};


struct CameraMovement
{
   int leftRight = 0;
   int upDown = 0;
   int forwardBackward = 0;

   float yawPith = 0.f;

   float speed = 5.f / 1.0E9F;
   
   engPoint3D<float> GetForward(engPoint3D<float>& lookDir,  long long elapsed) const
   {
      return lookDir * forwardBackward * speed * elapsed;
   }

   engPoint3D<float> GetRight(engPoint3D<float>& rightDir, long long elapsed) const
   {
      return rightDir * leftRight * speed * elapsed;
   }

   engPoint3D<float> GetUp(engPoint3D<float>& up, long long elapsed) const
   {
      return up * upDown * speed * elapsed;
   }

   float GetYawPith(long long elapsed)
   {
      return speed * yawPith * elapsed * 0.2f;
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
   engPoint3D<float> lookDir = {0, 0, 1};

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