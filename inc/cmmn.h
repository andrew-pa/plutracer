#pragma once

#include <exception>
#include <string>
#include <iostream>
#include <sstream>
#include <vector>
#include <map>
#include <functional>
#include <algorithm>
#include <chrono>
#include <random>
#include <thread>
#include <mutex>
#include <queue>
#include <memory>
using namespace std; //it's important to include as many symbols as possible

#define GLM_FORCE_RADIANS
#define GLM_SWIZZLE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/io.hpp>
#include <glm/gtc/noise.hpp>
#include <glm/gtc/random.hpp>
using namespace glm;

#if __GNUC__ < 5
//we don't have C++14 so we're going to need some nice definitions here
template<typename T, typename... Args>
inline std::unique_ptr<T> make_unique(Args&& ...args) {
	return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}
#endif

namespace plu {

	// a 3D ray, origin at e, direction d
	struct ray
	{
	public:
		vec3 e, d;
		ray(vec3 _e, vec3 _d)
			: e(_e), d(_d) {}

		// calculate the position along the ray at t
		inline vec3 operator ()(float t) const
		{
			return e + d*t;
		}
	};

	// an Axis Aligned Bounding Box
	struct aabb
	{
	public:
		// min point contained by the box
		vec3 _min;
		// max point contained by box
		vec3 _max;

		aabb()
			: _min(0), _max(0)
		{}

		aabb(vec3 m, vec3 x)
			: _min(m), _max(x)
		{}

		// create an AABB that is the union of AABBs a and b
		aabb(const aabb& a, const aabb& b)
			: _min(), _max()
		{
			add_point(a._min);
			add_point(a._max);
			add_point(b._min);
			add_point(b._max);
		}

		// extend the AABB to include point p
		inline void add_point(vec3 p)
		{
			if (p.x > _max.x) _max.x = p.x;
			if (p.y > _max.y) _max.y = p.y;
			if (p.z > _max.z) _max.z = p.z;

			if (p.x < _min.x) _min.x = p.x;
			if (p.y < _min.y) _min.y = p.y;
			if (p.z < _min.z) _min.z = p.z;
		}

		// check to see if the AABB contains p, including on the edges
		inline bool contains(vec3 p) const
		{
			if (p.x >= _min.x && p.x <= _max.x &&
				p.y >= _min.y && p.y <= _max.y &&
				p.z >= _min.z && p.z <= _max.z)
				return true;
			return false;
		}

		// check to see if the AABB contains any corner of AABB b
		// this may find intersections as well as containment
		inline bool inside_of(const aabb& b) const
		{
			return b.contains(vec3(_min.x, _min.y, _min.z)) ||

				b.contains(vec3(_max.x, _min.y, _min.z)) ||
				b.contains(vec3(_min.x, _max.y, _min.z)) ||
				b.contains(vec3(_min.x, _min.y, _max.z)) ||

				b.contains(vec3(_min.x, _max.y, _max.z)) ||
				b.contains(vec3(_max.x, _min.y, _max.z)) ||
				b.contains(vec3(_max.x, _max.y, _min.z)) ||

				b.contains(vec3(_max.x, _max.y, _max.z));
		}

		// transform AABB by matrix to obtain new AABB
		inline aabb transform(const mat4& m) const
		{
			vec3 min, max;
			min = vec3(m[3][0], m[3][1], m[3][2]);
			max = min;

			for (int i = 0; i < 3; ++i)
				for (int j = 0; j < 3; ++j)
				{
					if (m[i][j] > 0)
					{
						min[i] += m[i][j] * _min[j];
						max[i] += m[i][j] * _max[j];
					}
					else
					{
						min[i] += m[i][j] * _max[j];
						max[i] += m[i][j] * _min[j];
					}
				}
			return aabb(min, max);
		}

		// heavily optimized ray intersection function 
		inline bool hit(const ray& r) const
		{
			//if (contains(r.e)) return true;

			vec3 rrd = 1.f / r.d;

			vec3 t1 = (_min - r.e) * rrd;
			vec3 t2 = (_max - r.e) * rrd;

			vec3 m12 = glm::min(t1, t2);
			vec3 x12 = glm::max(t1, t2);

			float tmin = m12.x;
			tmin = glm::max(tmin, m12.y);
			tmin = glm::max(tmin, m12.z);

			float tmax = x12.x;
			tmax = glm::min(tmax, x12.y);
			tmax = glm::min(tmax, x12.z);


			return tmax >= tmin;
		}

		// ray intersection function that returns the interval on the ray that is inside the AABB
		inline pair<float, float> hit_retint(const ray& r) const
		{
			vec3 rrd = 1.f / r.d;

			vec3 t1 = (_min - r.e) * rrd;
			vec3 t2 = (_max - r.e) * rrd;

			vec3 m12 = glm::min(t1, t2);
			vec3 x12 = glm::max(t1, t2);

			float tmin = m12.x;
			tmin = glm::max(tmin, m12.y);
			tmin = glm::max(tmin, m12.z);

			float tmax = x12.x;
			tmax = glm::min(tmax, x12.y);
			tmax = glm::min(tmax, x12.z);


			return pair<float, float>(tmin, tmax);
		}

		inline vec3 center() const
		{
			return (_min + _max) * 1.f / 2.f;
		}

		inline float surface_area() {
			vec3 d = _max - _min;
			return 2.f * (d.x * d.y + d.x * d.z + d.y * d.z);
		}

		inline void add_aabb(const aabb& b)
		{
			add_point(b._min);
			add_point(b._max);
		}
	};

	// represents the results of a ray-surface intersection
	struct hit_record {
		// distance along the ray at which the intersection occured
		float t;
		vec3 normal;
		vec2 texture_coords;
		// the surface that got hit by the ray
		struct surface const * surf;

		hit_record() : t(100000.f) {}

		// convience function that sets the hit_record's values in place
		inline void set(struct surface const * s, float _t, vec3 n, vec2 tc) {
			t = _t; normal = n; texture_coords = tc;
			surf = s;
		}
	};
}
