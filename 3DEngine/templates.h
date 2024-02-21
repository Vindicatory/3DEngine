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

inline mat4x4 Matrix_PointAt(engPoint3D<float> &pos, engPoint3D<float> &target, engPoint3D<float> &up)
{
   // Calculate new forward direction
   engPoint3D<float> newForward = target - pos;
   newForward.Normalize();

   // Calculate new Up direction
   engPoint3D<float> a = newForward * up.DotProduct(newForward);
   engPoint3D<float> newUp = up - a;
   newUp.Normalize();

   // New Right direction is easy, its just cross product
   engPoint3D<float> newRight = newUp.CrossProduct(newForward);

   // Construct Dimensioning and Translation Matrix
   mat4x4 matrix;
   matrix.m[0][0] = newRight.x; matrix.m[0][1] = newRight.y; matrix.m[0][2] = newRight.z; matrix.m[0][3] = 0.0f;
   matrix.m[1][0] = newUp.x; matrix.m[1][1] = newUp.y; matrix.m[1][2] = newUp.z; matrix.m[1][3] = 0.0f;
   matrix.m[2][0] = newForward.x; matrix.m[2][1] = newForward.y; matrix.m[2][2] = newForward.z;   matrix.m[2][3] = 0.0f;
   matrix.m[3][0] = pos.x;  matrix.m[3][1] = pos.y;  matrix.m[3][2] = pos.z;  matrix.m[3][3] = 1.0f;
   return matrix;
}


inline mat4x4 Matrix_QuickInverse(mat4x4 &m) // Only for Rotation/Translation Matrices
{
   mat4x4 matrix;
   matrix.m[0][0] = m.m[0][0]; matrix.m[0][1] = m.m[1][0]; matrix.m[0][2] = m.m[2][0]; matrix.m[0][3] = 0.0f;
   matrix.m[1][0] = m.m[0][1]; matrix.m[1][1] = m.m[1][1]; matrix.m[1][2] = m.m[2][1]; matrix.m[1][3] = 0.0f;
   matrix.m[2][0] = m.m[0][2]; matrix.m[2][1] = m.m[1][2]; matrix.m[2][2] = m.m[2][2]; matrix.m[2][3] = 0.0f;
   matrix.m[3][0] = -(m.m[3][0] * matrix.m[0][0] + m.m[3][1] * matrix.m[1][0] + m.m[3][2] * matrix.m[2][0]);
   matrix.m[3][1] = -(m.m[3][0] * matrix.m[0][1] + m.m[3][1] * matrix.m[1][1] + m.m[3][2] * matrix.m[2][1]);
   matrix.m[3][2] = -(m.m[3][0] * matrix.m[0][2] + m.m[3][1] * matrix.m[1][2] + m.m[3][2] * matrix.m[2][2]);
   matrix.m[3][3] = 1.0f;
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


inline engPoint3D<float> Vector_IntersectPlane(engPoint3D<float> &plane_p, engPoint3D<float> &plane_n, engPoint3D<float> &lineStart, engPoint3D<float> &lineEnd)
{
   plane_n.Normalize();
   float plane_d = -plane_n.DotProduct(plane_p);
   float ad = lineStart.DotProduct(plane_n);
   float bd = lineEnd.DotProduct(plane_n);
   float t = (-plane_d - ad) / (bd - ad);
   engPoint3D<float> lineStartToEnd = lineEnd - lineStart;
   engPoint3D<float> lineToIntersect = lineStartToEnd * t;
   return lineStart + lineToIntersect;
}

   inline int Triangle_ClipAgainstPlane(engPoint3D<float> plane_p, engPoint3D<float> plane_n, engTriangle<float> &in_tri, engTriangle<float> &out_tri1, engTriangle<float> &out_tri2)
   {
      // Make sure plane normal is indeed normal
      plane_n.Normalize();

      // Return signed shortest distance from point to plane, plane normal must be normalised
      auto dist = [&](engPoint3D<float> &p)
      {
         p.Normalize();
         return plane_n.x * p.x + plane_n.y * p.y + plane_n.z * p.z - plane_n.DotProduct(plane_p);
      };

      // Create two temporary storage arrays to classify points either side of plane
      // If distance sign is positive, point lies on "inside" of plane
      engPoint3D<float>* inside_points[3];  int nInsidePointCount = 0;
      engPoint3D<float>* outside_points[3]; int nOutsidePointCount = 0;

      // Get signed distance of each point in triangle to plane
      float d0 = dist(in_tri.p[0]);
      float d1 = dist(in_tri.p[1]);
      float d2 = dist(in_tri.p[2]);

      if (d0 >= 0) { inside_points[nInsidePointCount++] = &in_tri.p[0]; }
      else { outside_points[nOutsidePointCount++] = &in_tri.p[0]; }
      if (d1 >= 0) { inside_points[nInsidePointCount++] = &in_tri.p[1]; }
      else { outside_points[nOutsidePointCount++] = &in_tri.p[1]; }
      if (d2 >= 0) { inside_points[nInsidePointCount++] = &in_tri.p[2]; }
      else { outside_points[nOutsidePointCount++] = &in_tri.p[2]; }

      // Now classify triangle points, and break the input triangle into 
      // smaller output triangles if required. There are four possible
      // outcomes...

      if (nInsidePointCount == 0)
      {
         // All points lie on the outside of plane, so clip whole triangle
         // It ceases to exist

         return 0; // No returned triangles are valid
      }

      if (nInsidePointCount == 3)
      {
         // All points lie on the inside of plane, so do nothing
         // and allow the triangle to simply pass through
         out_tri1 = in_tri;

         return 1; // Just the one returned original triangle is valid
      }

      if (nInsidePointCount == 1 && nOutsidePointCount == 2)
      {
         // Triangle should be clipped. As two points lie outside
         // the plane, the triangle simply becomes a smaller triangle

         // Copy appearance info to new triangle
         out_tri1.dp =  in_tri.dp;

         // The inside point is valid, so keep that...
         out_tri1.p[0] = *inside_points[0];

         // but the two new points are at the locations where the 
         // original sides of the triangle (lines) intersect with the plane
         out_tri1.p[1] = Vector_IntersectPlane(plane_p, plane_n, *inside_points[0], *outside_points[0]);
         out_tri1.p[2] = Vector_IntersectPlane(plane_p, plane_n, *inside_points[0], *outside_points[1]);

         return 1; // Return the newly formed single triangle
      }

      if (nInsidePointCount == 2 && nOutsidePointCount == 1)
      {
         // Triangle should be clipped. As two points lie inside the plane,
         // the clipped triangle becomes a "quad". Fortunately, we can
         // represent a quad with two new triangles

         // Copy appearance info to new triangles
         out_tri1.dp =  in_tri.dp;

         out_tri2.dp =  in_tri.dp;

         // The first triangle consists of the two inside points and a new
         // point determined by the location where one side of the triangle
         // intersects with the plane
         out_tri1.p[0] = *inside_points[0];
         out_tri1.p[1] = *inside_points[1];
         out_tri1.p[2] = Vector_IntersectPlane(plane_p, plane_n, *inside_points[0], *outside_points[0]);

         // The second triangle is composed of one of he inside points, a
         // new point determined by the intersection of the other side of the 
         // triangle and the plane, and the newly created point above
         out_tri2.p[0] = *inside_points[1];
         out_tri2.p[1] = out_tri1.p[2];
         out_tri2.p[2] = Vector_IntersectPlane(plane_p, plane_n, *inside_points[1], *outside_points[0]);

         return 2; // Return two newly formed triangles which form a quad
      }
   }