#include "surfaces/sphere.h"

namespace plu {
	namespace surfaces {
		// sphere-ray intersection code
		// basically spheres are a quadradic equation when you sub in the point-along-ray equation into their equation
		// point-along-ray equation: p = e + d * t
		// sphere equation: (x - x0)^2 + (y - y0)^2 + (z - z0)^2 = r^2 
		//		where (x0,y0,z0) is the center of the sphere and r is it's radius
		// sub in the point-along-ray equation into the sphere equation and simplify and you get a quadradic, trust me
		// basically (x,y,z) in the sphere equation is equal to vector p in the point-along-ray formula
		// imagine the sphere equation like such: sum_components( (p - c)^2 ) = r^2
		//		where p is the point on the sphere, c is the center, r is the radius and sum_components sums up the components of a vector
		// sub in (e+d*t) for p and expand (probably component wise) and it is a quadradic i tell you
		// in a ray intersection test you want to find that t value along the ray, so you just solve the quadradic with the quadradic formula
		bool sphere::hit(const ray& r, hit_record* hr) const {
			vec3 v = r.e - center; //very compact vector form of the initial values for the quadradic formula derived above
			float b = -dot(v, r.d);
			float det = (b*b) - dot(v, v) + radius*radius; //find the determinant in the quadradic formula
			if(det < 0) return false; // if there are only imaginary solutions, the ray doesn't really hit the sphere
			det = sqrt(det);
			float i1 = b-det, i2 = b+det; // find the actual roots; these are the two t values where the sphere intersects the ray
			if(i2 > 0 && i1 > 0) { // are any of them along the portion of the ray we care about?
				if(hr == nullptr) return true; // were we just quering for hit test and don't care about surface normals and such?
				if(hr->t < i1) return false; // did it turn out that this ray has already hit something closer? if so than we didn't really hit this sphere
				hr->t = i1; // store the point along the ray where the sphere got hit
				hr->normal = normalize(r(i1) - center); // calculate the surface normal at this point, simpily by normalizing the vector between the surface point and the center of the sphere
				
				// this code calculates the texture coords on the sphere, basically these tell it where to sample the texture that is "wrapped" around the sphere
				// essetially this code converts the intersection point to polar coords but also does some stuff to make it more continuous or something
				// honestly I copy-pasta'd this from somewhere
				float phi = acosf(-dot(hr->normal, vec3(0,1,0)));
				hr->texture_coords.y = phi * one_over_pi<float>();
				float theta = acosf(dot(vec3(0,0,-1), hr->normal)/sinf(phi)) * two_over_pi<float>();
				if(dot(vec3(1,0,0), hr->normal) >= 0) theta = 1.f - theta;
				hr->texture_coords.x = theta;	
				hr->surf = this;
				return true;
			}
			return false;
		}
	}
}
