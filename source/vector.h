
#ifndef __VECTOR_HPP
#define __VECTOR_HPP

#include <gccore.h>
#include <math.h>

class vec3 : public guVector
{
public:
	vec3(void)
	{
		x = 0.f;
		y = 0.f;
		z = 0.f;
	}

	vec3(const guVector &v)
	{
		x = v.x;
		y = v.y;
		z = v.z;
	}

	vec3(float px, float py, float pz)
	{
		x = px;
		y = py;
		z = pz;
	}

	vec3(float px, float py)
	{
		x = px;
		y = py;
		z = 0.f;
	}

	float sqNorm(void) const
	{
		return x * x + y * y + z * z;
	}

	float norm(void) const
	{
		return sqrt(sqNorm());
	}

	vec3 operator-(const vec3 &v) const
	{
		return vec3(x - v.x, y - v.y, z - v.z);
	}

	vec3 operator+(const vec3 &v) const
	{
		return vec3(x + v.x, y + v.y, z + v.z);
	}

	bool operator!=(const vec3 &v) const
	{
		return fabs(x - v.x) > 0.f || fabs(y - v.y) > 0.f || fabs(z - v.z) > 0.f;
	}

	bool operator==(const vec3 &v) const
	{
		return fabs(x - v.x) == 0.f && fabs(y - v.y) == 0.f && fabs(z - v.z) == 0.f;
	}

	vec3 &operator-=(const vec3 &v)
	{
		x -= v.x;
		y -= v.y;
		z -= v.z;
		return *this;
	}

	vec3 &operator+=(const vec3 &v)
	{
		x += v.x;
		y += v.y;
		z += v.z;
		return *this;
	}

	vec3 &operator*=(const vec3 &v)
	{
		x *= v.x;
		y *= v.y;
		z *= v.z;
		return *this;
	}

	vec3 operator/(float f) const
	{
		return f == 0.f ? *this : vec3(x / f, y / f, z / f);
	}

	vec3 operator*(const vec3 &v) const
	{
		return vec3(x * v.x, y * v.y, z * v.z);
	}

	vec3 operator*(float f) const
	{
		return vec3(x * f, y * f, z * f);
	}

	vec3 unit(void) const
	{
		return operator/(norm());
	}

	vec3 operator-(void) const
	{
		return vec3(-x, -y, -z);
	}
	
	vec3 rotateX(float angle) const
	{
		angle *= 0.01745329251994329577;
		float c = cos(angle);
		float s = sin(angle);
		return vec3(x, y * c - z * s, z * c + y * s);
	}

	vec3 rotateY(float angle) const
	{
		angle *= 0.01745329251994329577;
		float c = cos(angle);
		float s = sin(angle);
		return vec3(x * c + z * s, y, z * c - x * s);
	}

	vec3 rotateZ(float angle) const
	{
		angle *= 0.01745329251994329577;
		float c = cos(angle);
		float s = sin(angle);
		return vec3(x * c - y * s, y * c + x * s, z);
	}

	vec3 rotateX(float c, float s) const
	{
		return vec3(x, y * c - z * s, z * c + y * s);
	}

	vec3 rotateY(float c, float s) const
	{
		return vec3(x * c + z * s, y, z * c - x * s);
	}

	vec3 rotateZ(float c, float s) const
	{
		return vec3(x * c - y * s, y * c + x * s, z);
	}

	float dot(const vec3 &v) const
	{
		return x * v.x + y * v.y + z * v.z;
	}

	vec3 cross(const vec3 &v) const
	{
		return vec3(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x);
	}
};

#endif // !defined(__VECTOR_HPP)
