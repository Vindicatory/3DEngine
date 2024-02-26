#include "render.h"
#include <SDL.h>
#include <string>
#include <cassert>
#include <windows.h>

#include <algorithm>

#include <chrono>
#include <list>
using namespace std::chrono;

std::vector<Renderer*> Renderer::activeRenderers{};
engPoint2D<int> engScreenSize{ 1280, 720 };

void StartRenderLoop()
{
   assert(Renderer::activeRenderers[0] != nullptr);

   for (auto& renderer : Renderer::activeRenderers) {
      renderer->OnStart();
   }

   SDL_Event event;

   while (true)
   {
      CameraMovement movement;
      
      if (SDL_PollEvent(&event))
      {
         if (event.type == SDL_QUIT)
         {
            return;
         }
      }

      SDL_PumpEvents();
      const Uint8* keyboardState = SDL_GetKeyboardState(NULL);
      if (keyboardState[SDL_SCANCODE_A]) {
         movement.leftRight -= 1;
      }
      if (keyboardState[SDL_SCANCODE_W]) {
         movement.forwardBackward += 1;
      }
      if (keyboardState[SDL_SCANCODE_D]) {
         movement.leftRight += 1;
      }
      if (keyboardState[SDL_SCANCODE_S]) {
         movement.forwardBackward -= 1;
      }
      if (keyboardState[SDL_SCANCODE_Q]) {
         movement.upDown -= 1;
      }
      if (keyboardState[SDL_SCANCODE_E]) {
         movement.upDown += 1;
      }

      // pitch & yaw

      if (keyboardState[SDL_SCANCODE_LEFT]) {
         movement.yawPith += -1.f;
      }
      if (keyboardState[SDL_SCANCODE_RIGHT]) {
         movement.yawPith += 1.f;
      }
      if (keyboardState[SDL_SCANCODE_UP]) {
      }
      if (keyboardState[SDL_SCANCODE_DOWN]) {
      }

      for (auto& renderer : Renderer::activeRenderers) {
         renderer->camMovement = movement;
         renderer->DoRender();
      }
   }
}


void Renderer::OnStart()
{
   meshCube.LoadFromObjFile("axis.obj");
   meshCube.LoadFromObjFile("mountains.obj", true);

   float fNear = 0.1f;
   float fFar = 1000.0f;
   float fFov = 90.0f;
   float fAspectRatio = (float)engScreenSize.y / (float)engScreenSize.x;
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
   //fTheta += 1.f / howMuchNsInASec * elapsedTime;

   mat4x4 matRotY = Matrix_MakeRotationY(fTheta);

   mat4x4 mTrans = Matrix_MakeTranslation(0.f, 0.f, 5.f);
   mat4x4 mWorld = mat4x4Identity;
   mWorld = Matrix_MultiplyMatrix(mWorld, matRotY);
   mWorld = Matrix_MultiplyMatrix(mWorld, mTrans);

   // Handle camera movement
   engPoint3D<float> up = {0.f, 1.f, 0.f};

   mat4x4 mCameraRot = Matrix_MakeRotationY(camMovement.GetYawPith(elapsedTime));
   lookDir = Matrix_MultiplyVector(mCameraRot, lookDir);
   
   camPos = camPos + camMovement.GetForward(lookDir, elapsedTime);
   camPos = camPos + camMovement.GetUp(up, elapsedTime);

   engPoint3D<float> right = lookDir.CrossProduct(up);
   camPos = camPos + camMovement.GetRight(right, elapsedTime);
   
   engPoint3D<float> target = camPos + lookDir;

   mat4x4 mCamera = Matrix_PointAt(camPos, target, up);
   mat4x4 matView = Matrix_QuickInverse(mCamera);
   
   // Store triagles for rastering later
   std::vector<engTriangle<float>> vecTrianglesToRaster;

   float width = (float)window->size.x;
   float height = (float)window->size.y;
   
   // draw Triangles
   for (auto tri : meshCube.tris)
   {
      engTriangle<float> triProjected, triTransformed, triViewed;
      
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

         // Convert World Space --> View Space
         triViewed.p[0] = Matrix_MultiplyVector(matView, triTransformed.p[0]);
         triViewed.p[1] = Matrix_MultiplyVector(matView, triTransformed.p[1]);
         triViewed.p[2] = Matrix_MultiplyVector(matView, triTransformed.p[2]);
         triViewed.dp = dp;
         
         engTriangle<float> clipped[2];
         int nClippedTriangles = Triangle_ClipAgainstPlane({ 0.0f, 0.0f, 0.1f }, { 0.0f, 0.0f, 1.0f }, triViewed, clipped[0], clipped[1]);

         // We may end up with multiple triangles form the clip, so project as
         // required
         for (int n = 0; n < nClippedTriangles; n++)
         {
            // Project triangles from 3D --> 2D
            triProjected.p[0] = Matrix_MultiplyVector(matProj, clipped[n].p[0]);
            triProjected.p[1] = Matrix_MultiplyVector(matProj, clipped[n].p[1]);
            triProjected.p[2] = Matrix_MultiplyVector(matProj, clipped[n].p[2]);
            triProjected.dp = clipped[n].dp;

            // Scale into view, we moved the normalising into cartesian space
            // out of the matrix.vector function from the previous videos, so
            // do this manually
            
            triProjected.p[0] = triProjected.p[0] / triProjected.p[0].w;
            triProjected.p[1] = triProjected.p[1] / triProjected.p[1].w;
            triProjected.p[2] = triProjected.p[2] / triProjected.p[2].w;

            // X/Y are inverted so put them back
            triProjected.p[0].x *= -1.0f;
            triProjected.p[1].x *= -1.0f;
            triProjected.p[2].x *= -1.0f;
            triProjected.p[0].y *= -1.0f;
            triProjected.p[1].y *= -1.0f;
            triProjected.p[2].y *= -1.0f;

            // Offset verts into visible normalised space
            engPoint3D<float> vOffsetView = { 1,1,0 };
            triProjected.p[0] = triProjected.p[0] + vOffsetView;
            triProjected.p[1] = triProjected.p[1] + vOffsetView;
            triProjected.p[2] = triProjected.p[2] + vOffsetView;
            
            triProjected.p[0].x *= 0.5f * width;
            triProjected.p[0].y *= 0.5f * height;
            triProjected.p[1].x *= 0.5f * width;
            triProjected.p[1].y *= 0.5f * height;
            triProjected.p[2].x *= 0.5f * width;
            triProjected.p[2].y *= 0.5f * height;

            // Store triangle for sorting
            vecTrianglesToRaster.push_back(triProjected);
         }
      }
   }
   
   sort(vecTrianglesToRaster.begin(), vecTrianglesToRaster.end(), [](engTriangle<float>& t1, engTriangle<float>& t2)
      {
         float z1 = (t1.p[0].z + t1.p[1].z + t1.p[2].z) / 3.0f;
         float z2 = (t2.p[0].z + t2.p[1].z + t2.p[2].z) / 3.0f;
         return z1 > z2;
      });

   for (auto &triToRaster : vecTrianglesToRaster)
   {
      // Clip triangles against all four screen edges, this could yield
      // a bunch of triangles, so create a queue that we traverse to 
      //  ensure we only test new triangles generated against planes
      engTriangle<float> clipped[2];
      std::list<engTriangle<float>> listTriangles;

      // Add initial triangle
      listTriangles.push_back(triToRaster);
      int nNewTriangles = 1;

      for (int p = 0; p < 4; p++)
      {
         int nTrisToAdd = 0;
         while (nNewTriangles > 0)
         {
            // Take triangle from front of queue
            engTriangle<float> test = listTriangles.front();
            listTriangles.pop_front();
            nNewTriangles--;

            // Clip it against a plane. We only need to test each 
            // subsequent plane, against subsequent new triangles
            // as all triangles after a plane clip are guaranteed
            // to lie on the inside of the plane. I like how this
            // comment is almost completely and utterly justified
            switch (p)
            {
            case 0:	nTrisToAdd = Triangle_ClipAgainstPlane({ 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, test, clipped[0], clipped[1]); break;
            case 1:	nTrisToAdd = Triangle_ClipAgainstPlane({ 0.0f, height - 1, 0.0f }, { 0.0f, -1.0f, 0.0f }, test, clipped[0], clipped[1]); break;
            case 2:	nTrisToAdd = Triangle_ClipAgainstPlane({ 0.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }, test, clipped[0], clipped[1]); break;
            case 3:	nTrisToAdd = Triangle_ClipAgainstPlane({ width - 1, 0.0f, 0.0f }, { -1.0f, 0.0f, 0.0f }, test, clipped[0], clipped[1]); break;
            }

            // Clipping may yield a variable number of triangles, so
            // add these new ones to the back of the queue for subsequent
            // clipping against next planes
            for (int w = 0; w < nTrisToAdd; w++)
               listTriangles.push_back(clipped[w]);
         }
         nNewTriangles = listTriangles.size();
      }
   }

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
   
   SDL_Color col = {r, g, b, a};

   SDL_Vertex verts[3] =
   {
       { SDL_FPoint{ t.p[0].x, t.p[0].y }, col, SDL_FPoint{ 0 }, },
       { SDL_FPoint{ t.p[1].x, t.p[1].y }, col, SDL_FPoint{ 0 }, },
       { SDL_FPoint{ t.p[2].x, t.p[2].y }, col, SDL_FPoint{ 0 }, },
   };

   SDL_RenderGeometry(obj, nullptr, verts, 3, nullptr, 0);
}