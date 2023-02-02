// 02562 Rendering Framework
// Written by Jeppe Revall Frisvad, 2019
// Copyright (c) DTU Compute 2019

#include <optix_world.h>
#include "HitInfo.h"
#include "sampler.h"
#include "Holdout.h"

using namespace optix;

float3 Holdout::shade(const Ray& r, HitInfo& hit, bool emit) const
{
  float ambient = 0.0f;

  // Implement ambient occlusion here.
  //
  // Input:  r    (the ray that hit the material)
  //         hit  (info about the ray-surface intersection)
  //         emit (passed on to Emission::shade)
  //
  // Return: radiance reflected to where the ray was coming from
  //
  // Relevant data fields that are available (see Holdout.h):
  // samples      (number of times to trace a sample ray)
  // tracer       (pointer to ray tracer)
  //
  // Hint: Use the function tracer->trace_to_closest(...) to trace
  //       a new ray in a direction sampled on the hemisphere around the
  //       surface normal according to the function sample_cosine_weighted(...).
  for (int i = 0; i < samples; i++) {
	  float3 dir = sample_cosine_weighted(hit.shading_normal);
	  Ray new_ray = Ray(hit.position, dir, 0, 1e-4, RT_DEFAULT_MAX);
	  HitInfo new_hit;
	  tracer->trace_to_closest(new_ray,new_hit);
	  if (!new_hit.has_hit)
		  ambient = ambient + 1;
  }
  
  return ambient*tracer->get_background(r.direction);
}
