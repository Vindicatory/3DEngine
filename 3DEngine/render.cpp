#include "render.h"
#include <SDL.h>
#include <string>
#include <cassert>
#include<windows.h>

#include <chrono>
using namespace std::chrono;

std::vector<Renderer*> Renderer::activeRenderers{};

void StartRenderLoop()
{
   assert(Renderer::activeRenderers[0] != nullptr);

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


void Renderer::DoRender()
{
   auto timeFrameStart = high_resolution_clock::now();

   frameCount++;

   float fps = elapsedTime == 0 ? 0.f : 1000.f / elapsedTime;

   SDL_SetWindowTitle(window->Get(), std::to_string(fps).c_str());

   SDL_RenderClear(obj);
   SDL_SetRenderDrawColor(obj, 144, 77, 122, 255);
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