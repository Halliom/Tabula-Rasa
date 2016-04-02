#pragma once

#include <stdint.h>
#include <math.h>
#include <xmmintrin.h>

#define SHUFFLE3(V, X, Y, Z) Vector3(_mm_shuffle_ps((V).V, (V).V, _MM_SHUFFLE(Z, Z, Y, X)))

#define MINLINE __forceinline
#define VECCALL __vectorcall

struct Vector2
{
	MINLINE Vector2() { V = _mm_set_ps(0.0f, 0.0f, 0.0f, 0.0f); }
	MINLINE Vector2(float Value) { V = _mm_set_ps(Value, Value, Value, Value); }
	MINLINE Vector2(float X, float Y) { V = _mm_set_ps(Y, Y, Y, X); }

	MINLINE float VECCALL X() const { return _mm_cvtss_f32(V); }
	MINLINE float VECCALL Y() const { return _mm_cvtss_f32(_mm_shuffle_ps(V, V, _MM_SHUFFLE(1, 1, 1, 1))); } // or just _MM_SHUFFLE(1, 0, 0, 0) or whatever

	MINLINE void VECCALL Data(float* OutData) { OutData[0] = X(); OutData[1] = Y(); }

	__m128 V;
};

struct Vector3
{
	MINLINE Vector3() { V = _mm_set_ps(0.0f, 0.0f, 0.0f, 0.0f); }
	MINLINE Vector3(float Value) { V = _mm_set_ps(Value, Value, Value, Value); }
	MINLINE Vector3(float X, float Y, float Z) { V = _mm_set_ps(Z, Z, Y, X); }

	MINLINE float VECCALL X() const { return _mm_cvtss_f32(V); }
	MINLINE float VECCALL Y() const { return _mm_cvtss_f32(_mm_shuffle_ps(V, V, _MM_SHUFFLE(1, 1, 1, 1))); } // or just _MM_SHUFFLE(1, 0, 0, 0) or whatever
	MINLINE float VECCALL Z() const { return _mm_cvtss_f32(_mm_shuffle_ps(V, V, _MM_SHUFFLE(2, 2, 2, 2))); }

	MINLINE void VECCALL Data(float* OutData) { OutData[0] = X(); OutData[1] = Y(); OutData[2] = Z(); }

	MINLINE float VECCALL operator[] (size_t i) const { return V.m128_f32[i]; };
	MINLINE float& VECCALL operator[] (size_t i) { return V.m128_f32[i]; };

	__m128 V;
};

MINLINE Vector3 VECCALL operator+ (Vector3& a, Vector3& b) { Vector3 Result; Result.V = _mm_add_ps(a.V, b.V); return Result; }
MINLINE Vector3 VECCALL operator- (Vector3& a, Vector3& b) { Vector3 Result; Result.V = _mm_sub_ps(a.V, b.V); return Result; }
MINLINE Vector3 VECCALL operator* (Vector3& a, Vector3& b) { Vector3 Result; Result.V = _mm_mul_ps(a.V, b.V); return Result; }
MINLINE Vector3 VECCALL operator/ (Vector3& a, Vector3& b) { Vector3 Result; Result.V = _mm_div_ps(a.V, b.V); return Result; }
MINLINE Vector3 VECCALL operator* (Vector3& a, float b) { Vector3 Result; Result.V = _mm_mul_ps(a.V, _mm_set1_ps(b)); return Result; }
MINLINE Vector3 VECCALL operator/ (Vector3& a, float b) { Vector3 Result; Result.V = _mm_div_ps(a.V, _mm_set1_ps(b)); return Result; }
MINLINE Vector3 VECCALL operator* (float a, Vector3& b) { Vector3 Result; Result.V = _mm_mul_ps(_mm_set1_ps(a), b.V); return Result; }
MINLINE Vector3 VECCALL operator/ (float a, Vector3& b) { Vector3 Result; Result.V = _mm_div_ps(_mm_set1_ps(a), b.V); return Result; }

MINLINE Vector3& VECCALL operator+= (Vector3 &a, Vector3& b) { a = a + b; return a; }
MINLINE Vector3& VECCALL operator-= (Vector3 &a, Vector3& b) { a = a - b; return a; }
MINLINE Vector3& VECCALL operator*= (Vector3 &a, Vector3& b) { a = a * b; return a; }
MINLINE Vector3& VECCALL operator/= (Vector3 &a, Vector3& b) { a = a / b; return a; }
MINLINE Vector3& VECCALL operator*= (Vector3 &a, float b) { a = a * b; return a; }
MINLINE Vector3& VECCALL operator/= (Vector3 &a, float b) { a = a / b; return a; }

MINLINE Vector3 VECCALL min(Vector3& a, Vector3& b) { a.V = _mm_min_ps(a.V, b.V); return a; }
MINLINE Vector3 VECCALL max(Vector3& a, Vector3& b) { a.V = _mm_max_ps(a.V, b.V); return a; }

//MINLINE bool3 operator==(Vector3 a, Vector3 b) { a.m = _mm_cmpeq_ps(a.m, b.m); return a; }
//MINLINE bool3 operator!=(Vector3 a, Vector3 b) { a.m = _mm_cmpneq_ps(a.m, b.m); return a; }
//MINLINE bool3 operator< (Vector3 a, Vector3 b) { a.m = _mm_cmplt_ps(a.m, b.m); return a; }
//MINLINE bool3 operator> (Vector3 a, Vector3 b) { a.m = _mm_cmpgt_ps(a.m, b.m); return a; }
//MINLINE bool3 operator<=(Vector3 a, Vector3 b) { a.m = _mm_cmple_ps(a.m, b.m); return a; }
//MINLINE bool3 operator>=(Vector3 a, Vector3 b) { a.m = _mm_cmpge_ps(a.m, b.m); return a; }

struct Vector4
{
	MINLINE Vector4() { V = _mm_set_ps(0.0f, 0.0f, 0.0f, 0.0f); }
	MINLINE Vector4(float Value) { V = _mm_set_ps(Value, Value, Value, Value); }
	MINLINE Vector4(float X, float Y, float Z) { V = _mm_set_ps(1.0f, Z, Y, X); }
	MINLINE Vector4(float X, float Y, float Z, float W) { V = _mm_set_ps(W, Z, Y, X); }

	MINLINE float VECCALL X() const { return _mm_cvtss_f32(V); }
	MINLINE float VECCALL Y() const { return _mm_cvtss_f32(_mm_shuffle_ps(V, V, _MM_SHUFFLE(1, 1, 1, 1))); }
	MINLINE float VECCALL Z() const { return _mm_cvtss_f32(_mm_shuffle_ps(V, V, _MM_SHUFFLE(2, 2, 2, 2))); }
	MINLINE float VECCALL W() const { return _mm_cvtss_f32(_mm_shuffle_ps(V, V, _MM_SHUFFLE(3, 3, 3, 3))); }

	MINLINE void VECCALL Data(float* OutData) { OutData[0] = X(); OutData[1] = Y(); OutData[2] = Z(); OutData[3] = W(); }

	__m128 V;
};