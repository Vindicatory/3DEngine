#pragma once
#include <concepts>
#include <type_traits>

#include <vector>

#include <fstream>
#include <sstream>
#include <iostream>

#define USE_WTF

template<typename SDL_Struct, typename CreatorFunc, typename DestroyerFunc, CreatorFunc create, DestroyerFunc destroy>
class Wrapper {
protected:
   SDL_Struct* obj;

public:
   template<typename... Args>
   explicit Wrapper(Args&&... args) requires std::is_same_v<SDL_Struct*, decltype(create(std::forward<Args>(args)...))>
   {
      obj = create(std::forward<Args>(args)...);
   }

   ~Wrapper() {
      destroy(obj);
   }

   Wrapper(const Wrapper&) = delete;
   Wrapper& operator=(const Wrapper&) = delete;

   Wrapper(Wrapper&& other) noexcept : obj(other.obj) {
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
   a++; a--; a + b; a - b; a * b; a / b; a *= b; a /= b;
};


template<Numeric number>
struct engPoint2D {
   number x, y;
};


template<Numeric number>
struct engPoint3D {
   number x, y, z;

   engPoint3D<number> operator +(const engPoint3D<number>& other) {
      return { x + other.x, y + other.y , z + other.z };
   }

   engPoint3D<number> operator -(const engPoint3D<number>& other) {
      return { x - other.x, y - other.y , z - other.z };
   }

   engPoint3D<number> operator *(number n) {
      return { x * n, y * n , z * n };
   }

   engPoint3D<number> operator /(number n) {
      return { x / n, y / n , z / n };
   }

   number DotProduct(const engPoint3D<number>& other) {
      return x * other.x + y * other.y + z * other.z;
   }

   engPoint3D<number> CrossProduct(const engPoint3D<number>& other) {
      return {
         y * other.z - z * other.y,
         z * other.x - x * other.z,
         x * other.y - y * other.x
      };
   }

   number Lenght()
   {
      return 1 / q_rsqrt(x*x + y*y + z*z);
   }

   void Normalize()
   {
      *this = *this / Lenght();
   }

private:
   float q_rsqrt(float number)
   {
      long i;
      float x2, y;
      const float threehalfs = 1.5F;

      x2 = number * 0.5F;
      y  = number;
      i  = * ( long * ) &y;                       // evil floating point bit level hacking
      i  = 0x5f3759df - ( i >> 1 );               // what the fuck?
      y  = * ( float * ) &i;
      y  = y * ( threehalfs - ( x2 * y * y ) );   // 1st iteration
      // y  = y * ( threehalfs - ( x2 * y * y ) );   // 2nd iteration, this can be removed

      return y;
   }
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
   float m[4][4] = { 0.f };
};

constexpr const mat4x4 mat4x4Identity =
{
   1.f, 0.f, 0.f, 0.f,
   0.f, 1.f, 0.f, 0.f,
   0.f, 0.f, 1.f, 0.f,
   0.f, 0.f, 0.f, 1.f
};

inline mat4x4 Matrix_MakeRotationX(float fAngleRad)
{
   mat4x4 matrix;
   matrix.m[0][0] = 1.0f;
   matrix.m[1][1] = cosf(fAngleRad);
   matrix.m[1][2] = sinf(fAngleRad);
   matrix.m[2][1] = -sinf(fAngleRad);
   matrix.m[2][2] = cosf(fAngleRad);
   matrix.m[3][3] = 1.0f;
   return matrix;
}

inline mat4x4 Matrix_MakeRotationY(float fAngleRad)
{
   mat4x4 matrix;
   matrix.m[0][0] = cosf(fAngleRad);
   matrix.m[0][2] = sinf(fAngleRad);
   matrix.m[2][0] = -sinf(fAngleRad);
   matrix.m[1][1] = 1.0f;
   matrix.m[2][2] = cosf(fAngleRad);
   matrix.m[3][3] = 1.0f;
   return matrix;
}

inline mat4x4 Matrix_MakeRotationZ(float fAngleRad)
{
   mat4x4 matrix;
   matrix.m[0][0] = cosf(fAngleRad);
   matrix.m[0][1] = sinf(fAngleRad);
   matrix.m[1][0] = -sinf(fAngleRad);
   matrix.m[1][1] = cosf(fAngleRad);
   matrix.m[2][2] = 1.0f;
   matrix.m[3][3] = 1.0f;
   return matrix;
}

inline mat4x4 Matrix_MakeTranslation(float x, float y, float z)
{
   mat4x4 matrix;
   matrix.m[0][0] = 1.0f;
   matrix.m[1][1] = 1.0f;
   matrix.m[2][2] = 1.0f;
   matrix.m[3][3] = 1.0f;
   matrix.m[3][0] = x;
   matrix.m[3][1] = y;
   matrix.m[3][2] = z;
   return matrix;
}

inline mat4x4 Matrix_MakeProjection(float fFovDegrees, float fAspectRatio, float fNear, float fFar)
{
   float fFovRad = 1.0f / tanf(fFovDegrees * 0.5f / 180.0f * 3.14159f);
   mat4x4 matrix;
   matrix.m[0][0] = fAspectRatio * fFovRad;
   matrix.m[1][1] = fFovRad;
   matrix.m[2][2] = fFar / (fFar - fNear);
   matrix.m[3][2] = (-fFar * fNear) / (fFar - fNear);
   matrix.m[2][3] = 1.0f;
   matrix.m[3][3] = 0.0f;
   return matrix;
}

inline mat4x4 Matrix_MultiplyMatrix(mat4x4 &m1, mat4x4 &m2)
{
   mat4x4 matrix;
   for (int c = 0; c < 4; c++)
      for (int r = 0; r < 4; r++)
         matrix.m[r][c] = m1.m[r][0] * m2.m[0][c] + m1.m[r][1] * m2.m[1][c] + m1.m[r][2] * m2.m[2][c] + m1.m[r][3] * m2.m[3][c];
   return matrix;
}

inline engPoint3D<float> Matrix_MultiplyVector(const mat4x4& m, const engPoint3D<float>& i)
{
   engPoint3D<float> o;
   o.x = i.x * m.m[0][0] + i.y * m.m[1][0] + i.z * m.m[2][0] + m.m[3][0];
   o.y = i.x * m.m[0][1] + i.y * m.m[1][1] + i.z * m.m[2][1] + m.m[3][1];
   o.z = i.x * m.m[0][2] + i.y * m.m[1][2] + i.z * m.m[2][2] + m.m[3][2];
   float w = i.x * m.m[0][3] + i.y * m.m[1][3] + i.z * m.m[2][3] + m.m[3][3];

   if (w != 0.0f)
   {
      o = o / w;
   }

   return o;
}
