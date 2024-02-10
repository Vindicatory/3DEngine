#include "SDL.h"
#include <string>
#include "render.h"

int main(int argc, char** args)
{
   SDL_Init(SDL_INIT_EVERYTHING);

   Window window = Window("quick", {SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED}, { 1280, 720 }, SDL_WINDOW_RESIZABLE);
   Renderer renderer = Renderer(window.Get(), -1, 0);

   int counter = 0;

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

      counter++;
      SDL_SetWindowTitle(window.Get(), std::to_string(counter).c_str());

      SDL_RenderClear(renderer.Get());
      SDL_SetRenderDrawColor(renderer.Get(), 144, 77, 122, 255);
      SDL_RenderPresent(renderer.Get());
      SDL_RenderFlush(renderer.Get());
   }

   SDL_DestroyRenderer(renderer.Get());
   SDL_DestroyWindow(window.Get());
   SDL_Quit();

   return 0;
}