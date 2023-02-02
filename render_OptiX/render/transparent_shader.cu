// 02576 OptiX Rendering Framework
// Written by Jeppe Revall Frisvad, 2011
// Copyright (c) DTU Informatics 2011

#include <optix_world.h>
#include "structs.h"
#include "fresnel.h"
#include "sampler.h"

using namespace optix;

// Standard ray variables
rtDeclareVariable(optix::Ray, ray, rtCurrentRay, );
rtDeclareVariable(float, t_hit, rtIntersectionDistance, );
rtDeclareVariable(PerRayData_radiance, prd_radiance, rtPayload, );
rtDeclareVariable(int, max_depth, , );

// Variables for shading
rtDeclareVariable(float3, shading_normal, attribute shading_normal, );

// Material properties (corresponding to OBJ mtl params)
rtDeclareVariable(float, ior, , );

// Recursive ray tracing variables
rtDeclareVariable(float, scene_epsilon, , );
rtDeclareVariable(rtObject, top_object, , );
rtDeclareVariable(unsigned int, radiance_ray_type, , );

// Closest hit program for drawing shading normals
RT_PROGRAM void transparent_shader()
{
  if(prd_radiance.depth > max_depth)
  {
    prd_radiance.result = make_float3(0.0f);
    return;
  }

  float3 hit_pos = ray.origin + t_hit * ray.direction;
  float3 normal = normalize(rtTransformNormal(RT_OBJECT_TO_WORLD, shading_normal));
  float3 result = make_float3(0.0f);

  // Implement reflection and refraction using splitting.
  //
  // Input:  
  // ray            (the ray that hit the material)
  // t_hit          (distance along ray to hit position)
  // prd_radiance   (per ray data)
  //
  // Output: 
  // prd_radiance   (per ray data)
  //
  // Relevant data fields that are available (see above):
  // shading_normal (surface normal where the ray hit the material)
  // max_depth      (maximum trace depth)
  // scene_epsilon  (user defined epsilon for ray tracing)
  // top_object     (topmost object in hierarchy of objects)
  //
  // Hint: (a) OptiX includes functions reflect(...) and refract(...) for computing
  //       the reflected and refracted directions, respectively.
  //       (b) Make sure that you handle total internal reflection.

  // Trace reflected ray

  // Compute cosine to angle of incidence
  
  // Compute relative index of refraction

  // Compute Fresnel reflectance (R) and trace refracted ray if necessary
  result=make_float3(0.0f);
  PerRayData_radiance out_prd;
  out_prd.depth=prd_radiance.depth-1;
  if(prd_radiance.depth<=max_depth){
	  float3 reflected_dir=reflect(ray.direction,shading_normal);
	  PerRayData_radiance prd;
      float3 hit_pos=ray.direction*t_hit;
	  Ray reflect_r=make_Ray(hit_pos,reflected_dir,0,scene_epsilon,RT_DEFAULT_MAX);
	  rtTrace(top_object,reflect_r,prd_radiance);
	  prd_radiance.depth+=1;
     
      float3 refract_dir;
	  bool refraction=refract(refract_dir,ray.direction,shading_normal,ior);
	  Ray refract_r=make_Ray(hit_pos,refract_dir,0,scene_epsilon,RT_DEFAULT_MAX);
	  rtTrace(top_object,reflect_r,prd_radiance);

	  if(refraction){
		rtTrace(top_object,reflect_r,out_prd);
		out_prd.depth+=1;
	  }	  


	  float cos_theta=dot(ray.direction,shading_normal);
  }
  result=prd_radiance.result+out_prd.result;
  prd_radiance.result = result;
}
