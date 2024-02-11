#include "render.h"
#include <SDL.h>
#include <string>
#include <cassert>
#include<windows.h>

#include <chrono>
using namespace std::chrono;

std::vector<Renderer*> Renderer::activeRenderers{};
extern engPoint2D<int> engScreenSize{ 1280, 720 };

void StartRenderLoop()
{
   assert(Renderer::activeRenderers[0] != nullptr);

   for (auto& renderer : Renderer::activeRenderers) {
      renderer->OnStart();
   }

   SDL_Event event;

   while (true)
   {
      if (SDL_PollEvent(&event))
      {
         if (event.type == SDL_QUIT)
         {
            // Break out of the loop on quit
            break;
         }
      }

      for (auto& renderer : Renderer::activeRenderers) {
         renderer->DoRender();
      }
   }
}


void Renderer::OnStart()
{
   meshCube.tris = {

      // SOUTH
      { 0.0f, 0.0f, 0.0f,    0.0f, 1.0f, 0.0f,    1.0f, 1.0f, 0.0f },
      { 0.0f, 0.0f, 0.0f,    1.0f, 1.0f, 0.0f,    1.0f, 0.0f, 0.0f },

      // EAST                                                      
      { 1.0f, 0.0f, 0.0f,    1.0f, 1.0f, 0.0f,    1.0f, 1.0f, 1.0f },
      { 1.0f, 0.0f, 0.0f,    1.0f, 1.0f, 1.0f,    1.0f, 0.0f, 1.0f },

      // NORTH                                                     
      { 1.0f, 0.0f, 1.0f,    1.0f, 1.0f, 1.0f,    0.0f, 1.0f, 1.0f },
      { 1.0f, 0.0f, 1.0f,    0.0f, 1.0f, 1.0f,    0.0f, 0.0f, 1.0f },

      // WEST                                                      
      { 0.0f, 0.0f, 1.0f,    0.0f, 1.0f, 1.0f,    0.0f, 1.0f, 0.0f },
      { 0.0f, 0.0f, 1.0f,    0.0f, 1.0f, 0.0f,    0.0f, 0.0f, 0.0f },

      // TOP                                                       
      { 0.0f, 1.0f, 0.0f,    0.0f, 1.0f, 1.0f,    1.0f, 1.0f, 1.0f },
      { 0.0f, 1.0f, 0.0f,    1.0f, 1.0f, 1.0f,    1.0f, 1.0f, 0.0f },

      // BOTTOM                                                    
      { 1.0f, 0.0f, 1.0f,    0.0f, 0.0f, 1.0f,    0.0f, 0.0f, 0.0f },
      { 1.0f, 0.0f, 1.0f,    0.0f, 0.0f, 0.0f,    1.0f, 0.0f, 0.0f },

   };

   float fNear = 0.1f;
   float fFar = 1000.0f;
   float fFov = 90.0f;
   float fAspectRatio = (float)engScreenSize.x / (float)engScreenSize.y;
   float fFovRad = 1.0f / tanf(fFov * 0.5f / 180.0f * 3.14159f);

   matProj.m[0][0] = fAspectRatio * fFovRad;
   matProj.m[1][1] = fFovRad;
   matProj.m[2][2] = fFar / (fFar - fNear);
   matProj.m[3][2] = (-fFar * fNear) / (fFar - fNear);
   matProj.m[2][3] = 1.0f;
   matProj.m[3][3] = 0.0f;
}


void Renderer::DoRender()
{
   auto timeFrameStart = high_resolution_clock::now();

   frameCount++;

   float fps = elapsedTime == 0 ? 0.f : 1000.f / elapsedTime;

   SDL_SetWindowTitle(window->Get(), std::format("{} {}", "fps=", std::to_string(fps)).c_str());

   SDL_SetRenderDrawColor(obj, 0, 0, 0, 255); // background
   SDL_RenderClear(obj);

   SDL_SetRenderDrawColor(obj, 255, 255, 255, 255); // triangle color
   
   // setup rotation matrices
   mat4x4 matRotZ, matRotX;
   fTheta += 0.001f * elapsedTime;

   // Rotation Z
   matRotZ.m[0][0] = cosf(fTheta);
   matRotZ.m[0][1] = sinf(fTheta);
   matRotZ.m[1][0] = -sinf(fTheta);
   matRotZ.m[1][1] = cosf(fTheta);
   matRotZ.m[2][2] = 1;
   matRotZ.m[3][3] = 1;

   // Rotation X
   matRotX.m[0][0] = 1;
   matRotX.m[1][1] = cosf(fTheta * 0.5f);
   matRotX.m[1][2] = sinf(fTheta * 0.5f);
   matRotX.m[2][1] = -sinf(fTheta * 0.5f);
   matRotX.m[2][2] = cosf(fTheta * 0.5f);
   matRotX.m[3][3] = 1;

   // Draw Triangles
   for (auto tri : meshCube.tris)
   {
      engTriangle<float> triProjected, triTranslated, triRotatedZ, triRotatedZX;

      // Rotate in Z-Axis
      MultiplyMatrixVector(tri.p[0], triRotatedZ.p[0], matRotZ);
      MultiplyMatrixVector(tri.p[1], triRotatedZ.p[1], matRotZ);
      MultiplyMatrixVector(tri.p[2], triRotatedZ.p[2], matRotZ);

      // Rotate in X-Axis
      MultiplyMatrixVector(triRotatedZ.p[0], triRotatedZX.p[0], matRotX);
      MultiplyMatrixVector(triRotatedZ.p[1], triRotatedZX.p[1], matRotX);
      MultiplyMatrixVector(triRotatedZ.p[2], triRotatedZX.p[2], matRotX);

      // Offset into the screen
      triTranslated = triRotatedZX;
      triTranslated.p[0].z = triRotatedZX.p[0].z + 3.0f;
      triTranslated.p[1].z = triRotatedZX.p[1].z + 3.0f;
      triTranslated.p[2].z = triRotatedZX.p[2].z + 3.0f;

      // Project triangles from 3D --> 2D
      MultiplyMatrixVector(triTranslated.p[0], triProjected.p[0], matProj);
      MultiplyMatrixVector(triTranslated.p[1], triProjected.p[1], matProj);
      MultiplyMatrixVector(triTranslated.p[2], triProjected.p[2], matProj);

      // Scale into view
      triProjected.p[0].x += 1.0f; triProjected.p[0].y += 1.0f;
      triProjected.p[1].x += 1.0f; triProjected.p[1].y += 1.0f;
      triProjected.p[2].x += 1.0f; triProjected.p[2].y += 1.0f;
      triProjected.p[0].x *= 0.5f * (float)engScreenSize.x;
      triProjected.p[0].y *= 0.5f * (float)engScreenSize.y;
      triProjected.p[1].x *= 0.5f * (float)engScreenSize.x;
      triProjected.p[1].y *= 0.5f * (float)engScreenSize.y;
      triProjected.p[2].x *= 0.5f * (float)engScreenSize.x;
      triProjected.p[2].y *= 0.5f * (float)engScreenSize.y;

      DrawTriangle(triProjected);
   }
   //

   SDL_RenderPresent(obj);
   SDL_RenderFlush(obj);

   elapsedTime = duration_cast<milliseconds>(high_resolution_clock::now() - timeFrameStart).count();

   // fps limitation
   if (maxFps > 0) {
      if (elapsedTime >= expectedFrameDur) {
         return;
      }

      Sleep(expectedFrameDur - elapsedTime);
      elapsedTime = duration_cast<milliseconds>(high_resolution_clock::now() - timeFrameStart).count();
   }
}


void Renderer::MultiplyMatrixVector(const engPoint3D<float>& i, engPoint3D<float>& o, mat4x4& m)
{
   o.x = i.x * m.m[0][0] + i.y * m.m[1][0] + i.z * m.m[2][0] + m.m[3][0];
   o.y = i.x * m.m[0][1] + i.y * m.m[1][1] + i.z * m.m[2][1] + m.m[3][1];
   o.z = i.x * m.m[0][2] + i.y * m.m[1][2] + i.z * m.m[2][2] + m.m[3][2];
   float w = i.x * m.m[0][3] + i.y * m.m[1][3] + i.z * m.m[2][3] + m.m[3][3];

   if (w != 0.0f)
   {
      o.x /= w; o.y /= w; o.z /= w;
   }
}


void Renderer::DrawTriangle(const engTriangle<float> t)
{
   SDL_RenderDrawLineF(obj, t.p[0].x, t.p[0].y, t.p[1].x, t.p[1].y);
   SDL_RenderDrawLineF(obj, t.p[1].x, t.p[1].y, t.p[2].x, t.p[2].y);
   SDL_RenderDrawLineF(obj, t.p[0].x, t.p[0].y, t.p[2].x, t.p[2].y);
}