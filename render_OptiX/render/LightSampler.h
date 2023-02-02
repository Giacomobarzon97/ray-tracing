/*
 * =====================================================================================
 *
 *       Filename:  LightSampler.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  02/24/2012 09:56:00 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *        Company:  
 *
 * =====================================================================================
 */

#ifndef LIGHTSAMPLER_H
#define LIGHTSAMPLER_H

#include "../cuda/random.h"
#include "Light.h"

rtBuffer<Light> lights;

// Area light variables
rtBuffer<float3> light_verts;     
rtBuffer<float3> light_norms;
rtBuffer<int3> light_vidxs;
rtBuffer<int3> light_nidxs;


__device__ __inline__ void sample_light(const float3& pos, float3& dir, float3& L, float& dist, unsigned int& seed)
{
	// Sample a light source 
	
	int numlights = lights.size() ;
	int n = int( numlights * rnd(seed) );

	const LightType& lt = lights[n].type;
	if (lt == POINT_LIGHT_TYPE)
	{
		dir = lights[n].position - pos;
		dist = length(dir);
		dir /= dist;

		L = lights[n].emission / (dist*dist);


	}
	else if (lt == DIRECTIONAL_LIGHT_TYPE)
	{
		L = lights[n].emission;
		dir = -lights[n].direction;
		dist = RT_DEFAULT_MAX;
	}
	else if (lt == AREA_LIGHT_TYPE)
	{
		int triangle = int( light_vidxs.size() * rnd(seed) );		

		float3 normal;
		float3 light_pos;


		float area = 0.0f;
		for(int i = 0; i < light_vidxs.size(); ++i)
		{
			int3 idx = light_vidxs[i];
			float3 v0 = light_verts[idx.x];
			float3 v1 = light_verts[idx.y];
			float3 v2 = light_verts[idx.z];


			float3 e0 = v1 - v0;
			float3 e1 = v2 - v0;
			float3 n = cross(e0, e1);

			float n_length = length(n);
			area += 0.5f*n_length;	

			if (i == triangle)
			{
				normal = n / n_length;

				// Barycentric sampling

				float sqrt_xi1 = sqrt(rnd(seed));
				float xi2 = rnd(seed);


				// Calculate Barycentric coordinates
				float u = 1.0f - sqrt_xi1;
				float v = (1.0f - xi2)*sqrt_xi1;

				light_pos = v0 + u*e0 + v*e1;
			}
		}



		dir = light_pos - pos;
		float sqr_dist = dot(dir, dir);
		dist = sqrtf(sqr_dist);
		dir /= dist;

		float cos_theta_prime = dot(normal, -dir); // the ray may hit the light source from behind

		L = lights[n].emission*(area*clamp(cos_theta_prime,0.0f, 1.0f)/sqr_dist);

	}

	L *= float(numlights); // Multiply by 1/pdf for the discrete sampling of light source
}


#endif // LIGHTSAMPLER_H
