// 02562 Rendering Framework
// Written by Jeppe Revall Frisvad, 2011
// Copyright (c) DTU Informatics 2011

#include <optix_world.h>
#include "HitInfo.h"
#include "int_pow.h"
#include "GlossyVolume.h"

using namespace optix;

#ifndef M_1_PIf
#define M_1_PIf 0.31830988618379067154
#endif

float3 GlossyVolume::shade(const Ray& r, HitInfo& hit, bool emit) const
{
	// Compute the specular part of the glossy shader and attenuate it
	// by the transmittance of the material if the ray is inside (as in
	// the volume shader).
	//return 0.9*Transparent::shade(r, hit, emit)+0.1*Mirror::shade(r, hit, emit)+Phong::shade(r, hit, emit);

	float3 rho_d = get_diffuse(hit);
	float3 rho_s = get_specular(hit);
	float s = get_shininess(hit);

	float3 phong_radiance = make_float3(0.0f);

	float3 wo = -r.direction;
	for (int i = 0; i < lights.size(); i++) {
		float3 wi;
		float3 L;
		bool shade = lights[i]->sample(hit.position, wi, L);
		float3 wr = reflect(wi, hit.shading_normal);
		if (dot(wi, hit.shading_normal) > 0) {
			float3 b = rho_s * (s + 2) / (2 * M_PIf);
			float c = pow(dot(wo, wr), s);
			float3 d = b * c;
			float3 e = L * dot(wi, hit.shading_normal);
			phong_radiance = phong_radiance + d * e;
		}
	}

	return 0.9 * Transparent::shade(r, hit, emit)+0.1*Mirror::shade(r, hit, emit)+ phong_radiance +Volume::shade(r, hit, emit);
}
