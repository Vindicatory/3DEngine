#pragma once
#include <concepts>
#include <type_traits>

#include <vector>

template<typename SDL_Struct, typename CreatorFunc, typename DestroyerFunc, CreatorFunc create, DestroyerFunc destroy>
class Wrapper {
protected:
   SDL_Struct* obj;
   DestroyerFunc destroy;

public:
   template<typename... Args>
   explicit Wrapper(Args&&... args) requires std::is_same_v<SDL_Struct*, decltype(create(std::forward<Args>(args)...))>
      : destroy(destroy)
   {
      obj = create(std::forward<Args>(args)...);
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

   SDL_Struct* Get() const {
      return obj;
   }

   SDL_Struct* operator->() const {
      return obj;
   }

   bool isValid() const {
      return obj != nullptr;
   }
};


// maths, vectors, e.t.c

template<typename B>
concept Boolean =
   requires(B x, B y) {
      { x = true };
      { x = false };
      { x = (x == y) };
      { x = (x != y) };
      { x = !x };
      { x = (x = y) };
};

template<typename T>
concept Equitable =
   requires (T a, T b) {
      {a == b} -> Boolean;
};

template<typename T>
concept Comparable = Equitable<T> &&
   requires (T a, T b) {
   a < b; a > b; a == b; a <= b; a >= b;
};

template<typename T>
concept Numeric = Comparable<T> &&
   requires (T a, T b) {
   a++; a--; a + b; a - b; a * b; a / b;
};


template<Numeric number>
struct engPoint2D {
   number x, y;
};

template<Numeric number>
struct engPoint3D {
   number x, y, z;
};

template<Numeric number>
struct engTriangle {
   engPoint3D<number> p[3];
};

template<Numeric number>
struct engMesh {
   std::vector<engTriangle<number>> tris;
};