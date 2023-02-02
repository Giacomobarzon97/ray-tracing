// 02576 OptiX Rendering Framework
// Written by Jeppe Revall Frisvad, 2011
// Copyright (c) DTU Informatics 2011

#include <optix_world.h>
#include "structs.h"

using namespace optix;

// Standard ray variables
rtDeclareVariable(Ray, ray, rtCurrentRay, );
rtDeclareVariable(float, t_hit, rtIntersectionDistance, );
rtDeclareVariable(PerRayData_radiance, prd_radiance, rtPayload, );
rtDeclareVariable(int, max_depth, , );

// Variables for shading
rtDeclareVariable(float3, shading_normal, attribute shading_normal, );

// Recursive ray tracing variables
rtDeclareVariable(float, scene_epsilon, , );
rtDeclareVariable(rtObject, top_object, , );
rtDeclareVariable(unsigned int, radiance_ray_type, , );

// Closest hit program for drawing shading normals
RT_PROGRAM void mirror_shader()
{
  if(prd_radiance.depth > max_depth)
  {
    prd_radiance.result = make_float3(0.0f);
    return;
  }

  float3 hit_pos = ray.origin + t_hit * ray.direction;
  float3 normal = normalize(rtTransformNormal(RT_OBJECT_TO_WORLD, shading_normal));

  // Implement mirror reflection here.
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
  // Hint: (a) OptiX includes a function reflect(v, n) which returns the reflection
  //       of the ray direction v around the forward facing normal n.
  //       (b) Use prd_radiance.depth to keep track of the number of the number of
  //       surface interactions previously suffered by the ray.
  prd_radiance.result = make_float3(0.0f);
  if(prd_radiance.depth<=max_depth){
	  float3 reflected_dir=reflect(ray.direction,shading_normal);
	  PerRayData_radiance prd;
      float3 hit_pos=ray.direction*t_hit;
	  Ray r=make_Ray(hit_pos,reflected_dir,0,scene_epsilon,RT_DEFAULT_MAX);
	  rtTrace(top_object,r,prd_radiance);
	  prd_radiance.depth+=1;
  }
}
