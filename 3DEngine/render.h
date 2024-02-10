#pragma once
#include <concepts>
#include <type_traits>
#include <SDL.h>


template<typename SDL_Struct, typename CreatorFunc, typename DestroyerFunc, CreatorFunc create, DestroyerFunc destroy>
class Wrapper {
private:
   SDL_Struct* obj;
public:
   template<typename... Args>
   explicit Wrapper(Args&&... args)
      : obj(std::forward<Args>(args)...)) {
         if (!obj) {
            throw std::runtime_error("Failed to create SDL obj.");
         }
    }

    ~Wrapper() {
       if (obj) {
          destroy(obj);
       }
    }

    Wrapper(const Wrapper&) = delete;
    Wrapper& operator=(const Wrapper&) = delete;

    Wrapper(Wrapper&& other) : obj(other.obj) {
       other.obj = nullptr;
    }

    Wrapper& operator=(Wrapper&& other) noexcept {
       if (this != &other) {
          if (obj) {
             destroy(obj);
          }

          obj = other.obj;
          other.obj = nullptr;
       }
       return *this;
    }

    SDL_Struct* get() const {
       return obj;
    }

    SDL_Struct* operator->() const {
       return obj;
    }

    bool isValid() const {
       return obj != nullptr;
    }
};

// Helper functions for the actual SDL resource creation
auto createWindowWrapper(const char* title, int x, int y, int w, int h, Uint32 flags) {
   return Wrapper<SDL_Window, decltype(&SDL_CreateWindow), decltype(&SDL_DestroyWindow), SDL_CreateWindow, SDL_DestroyWindow>(title, x, y, w, h, flags);
}

auto createRendererWrapper(SDL_Window* window, int index, Uint32 flags) {
   return Wrapper<SDL_Renderer, decltype(&SDL_CreateRenderer), decltype(&SDL_DestroyRenderer), SDL_CreateRenderer, SDL_DestroyRenderer>(window, index, flags);
}