#include "render.h"
#include <SDL.h>
#include <string>
#include <cassert>
#include <windows.h>

#include <algorithm>

#include <chrono>
using namespace std::chrono;

std::vector<Renderer*> Renderer::activeRenderers{};
engPoint2D<int> engScreenSize{ 1280, 720 };

// #define DEBUG_RENDER

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
            return;
         }
      }

      for (auto& renderer : Renderer::activeRenderers) {
         renderer->DoRender();
      }
   }
}


void Renderer::OnStart()
{
   meshCube.LoadFromObjFile("monkey.obj");

   float fNear = 0.1f;
   float fFar = 1000.0f;
   float fFov = 90.0f;
   float fAspectRatio = (float)engScreenSize.x / (float)engScreenSize.y;
   float fFovRad = 1.0f / tanf(fFov * 0.5f / 180.0f * 3.14159f);

   matProj = Matrix_MakeProjection(fFov, fAspectRatio, fNear, fFar);
}


void Renderer::DoRender()
{
   auto timeFrameStart = high_resolution_clock::now();

   frameCount++;

   float fps = elapsedTime == 0 ? 0.f : howMuchNsInASec / elapsedTime;

   SDL_SetWindowTitle(window->Get(), std::format("fps = {}", std::to_string(fps)).c_str());

   SDL_SetRenderDrawColor(obj, 0, 0, 0, 255); // background
   SDL_RenderClear(obj);

   // setup rotation matrices
   fTheta += 1.f / howMuchNsInASec * elapsedTime;

   mat4x4 matRotY = Matrix_MakeRotationY(fTheta);

   mat4x4 mTrans = Matrix_MakeTranslation(0.f, 0.f, 5.f);
   mat4x4 mWorld = mat4x4Identity;
   mWorld = Matrix_MultiplyMatrix(mWorld, matRotY);
   mWorld = Matrix_MultiplyMatrix(mWorld, mTrans);
   
   // Store triagles for rastering later
   std::vector<engTriangle<float>> vecTrianglesToRaster;

   // draw Triangles
   for (auto tri : meshCube.tris)
   {
      engTriangle<float> triProjected, triTransformed;
      
      // offset into the screen
      triTransformed.p[0] = Matrix_MultiplyVector(mWorld, tri.p[0]);
      triTransformed.p[1] = Matrix_MultiplyVector(mWorld, tri.p[1]);
      triTransformed.p[2] = Matrix_MultiplyVector(mWorld, tri.p[2]);

      // Cross-Product to get surface normals
      
      engPoint3D<float> line1 = triTransformed.p[1]- triTransformed.p[0];
      engPoint3D<float> line2 = triTransformed.p[2] - triTransformed.p[0];

      engPoint3D<float> normal = line1.CrossProduct(line2);
      normal.Normalize();

      engPoint3D<float> camRay = triTransformed.p[0] - camPos;
      
      if (normal.DotProduct(camRay) < 0.0f) {
         // shading
         engPoint3D<float> lightDirection = { 0.0f, 1.0f, -1.0f };
         // normalize light vec
         lightDirection.Normalize();

         // dot product between light direction and each triangle normal
         float dp = max(0.1f, lightDirection.DotProduct(normal));

         // project triangles from 3D --> 2D
         triProjected.p[0] = Matrix_MultiplyVector(matProj, triTransformed.p[0]);
         triProjected.p[1] = Matrix_MultiplyVector(matProj, triTransformed.p[1]);
         triProjected.p[2] = Matrix_MultiplyVector(matProj, triTransformed.p[2]);

         // scale into view
         triProjected.p[0].x += 1.0f; triProjected.p[0].y += 1.0f;
         triProjected.p[1].x += 1.0f; triProjected.p[1].y += 1.0f;
         triProjected.p[2].x += 1.0f; triProjected.p[2].y += 1.0f;
         triProjected.p[0].x *= 0.5f * (float)engScreenSize.x;
         triProjected.p[0].y *= 0.5f * (float)engScreenSize.y;
         triProjected.p[1].x *= 0.5f * (float)engScreenSize.x;
         triProjected.p[1].y *= 0.5f * (float)engScreenSize.y;
         triProjected.p[2].x *= 0.5f * (float)engScreenSize.x;
         triProjected.p[2].y *= 0.5f * (float)engScreenSize.y;


         triProjected.dp = dp;
         vecTrianglesToRaster.push_back(triProjected); 
      }
   }
   //
   
   sort(vecTrianglesToRaster.begin(), vecTrianglesToRaster.end(), [](engTriangle<float>& t1, engTriangle<float>& t2)
      {
         float z1 = (t1.p[0].z + t1.p[1].z + t1.p[2].z) / 3.0f;
         float z2 = (t2.p[0].z + t2.p[1].z + t2.p[2].z) / 3.0f;
         return z1 > z2;
      });

   for (auto& tri : vecTrianglesToRaster) {
      DrawTriangle(tri);
   }

   SDL_RenderPresent(obj);
   SDL_RenderFlush(obj);

   auto now = high_resolution_clock::now();
   elapsedTime = duration_cast<nanoseconds>(now - timeFrameStart).count();

   // fps limitation
   if (maxFps > 0) {
      if (elapsedTime >= expectedFrameDurNs) {
         return;
      }

      Sleep((expectedFrameDurNs - elapsedTime) / howMuchNsInAMs);
      elapsedTime = duration_cast<nanoseconds>(high_resolution_clock::now() - timeFrameStart).count();
   }
}

void Renderer::DrawTriangle(const engTriangle<float> t)
{
#ifdef DEBUG_RENDER
   SDL_SetRenderDrawColor(obj, 0, 0, 0, 0); // debug black lines

   SDL_RenderDrawLineF(obj, t.p[0].x, t.p[0].y, t.p[1].x, t.p[1].y);
   SDL_RenderDrawLineF(obj, t.p[1].x, t.p[1].y, t.p[2].x, t.p[2].y);
   SDL_RenderDrawLineF(obj, t.p[0].x, t.p[0].y, t.p[2].x, t.p[2].y);
#endif

   Uint8 a = Unpack(t.dp);
   Uint8 r = Unpack(t.dp);
   Uint8 g = Unpack(t.dp);
   Uint8 b = Unpack(t.dp);

   SDL_SetRenderDrawColor(obj, r, g, b, a); // triangle color

   SDL_Vertex verts[3] =
   {
       { SDL_FPoint{ t.p[0].x, t.p[0].y }, SDL_Color{ r, g, b, a }, SDL_FPoint{ 0 }, },
       { SDL_FPoint{ t.p[1].x, t.p[1].y }, SDL_Color{ r, g, b, a }, SDL_FPoint{ 0 }, },
       { SDL_FPoint{ t.p[2].x, t.p[2].y }, SDL_Color{ r, g, b, a }, SDL_FPoint{ 0 }, },
   };

   SDL_RenderGeometry(obj, nullptr, verts, 3, nullptr, 0);
}