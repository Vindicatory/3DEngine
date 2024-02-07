#include "SDL.h"
#include <string>

int main(int argc, char* argv[])
{
   (void)argc; (void)argv;

   SDL_Init(SDL_INIT_EVERYTHING);
   SDL_Window* w = SDL_CreateWindow("quick", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_RESIZABLE);
   SDL_Renderer* r = SDL_CreateRenderer(w, -1, 0);

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
      SDL_SetWindowTitle(w, std::to_string(counter).c_str());

      SDL_RenderClear(r);
      SDL_SetRenderDrawColor(r, 144, 77, 122, 255);
      SDL_RenderPresent(r);
      SDL_RenderFlush(r);
   }

   SDL_DestroyRenderer(r);
   SDL_DestroyWindow(w);
   SDL_Quit();

   return 0;
}