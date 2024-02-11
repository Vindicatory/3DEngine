#pragma once
#include <concepts>
#include <type_traits>

#include <vector>


#include <fstream>
#include <sstream>
#include <iostream>

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
      destroy(obj);
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
   float dp;
};


struct engMesh {
   std::vector<engTriangle<float>> tris;

   bool LoadFromObjFile(const char* filePath, bool loadMultiple = false)
   {
      if (!loadMultiple) {
         tris.clear();
      }

      std::ifstream file(filePath);
      if (!file.is_open()) {
         return false;
      }

      std::vector<engPoint3D<float>> verts;

      while (!file.eof()) {
         
         char line[128];
         file.getline(line, 128);

         std::stringstream s;
         s << line;

         char junk;

         if (line[0] == 'v') {
            engPoint3D<float> v;
            s >> junk >> v.x >> v.y >> v.z;
            verts.push_back(v);
         }

         if (line[0] == 'f')
         {
            int f[3];
            s >> junk >> f[0] >> f[1] >> f[2];
            tris.push_back({verts[f[0] - 1], verts[f[1] - 1], verts[f[2] - 1] });
         }
      }

      return true;
   }
};


struct mat4x4 {
   float m[4][4] = { 0 };
};
