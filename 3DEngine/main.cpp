#include "SDL.h"
#include <string>
#include "render.h"
#include <memory>

int main(int argc, char** args)
{
   SDL_Init(SDL_INIT_EVERYTHING);

   Window window = Window("", {SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED}, engScreenSize, SDL_WINDOW_RESIZABLE);

   Renderer renderer = Renderer(&window, -1, 0);

   StartRenderLoop();

   SDL_Quit();

   return 0;
}