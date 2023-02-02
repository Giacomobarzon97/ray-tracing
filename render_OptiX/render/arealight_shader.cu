// 02576 OptiX Rendering Framework
// Written by Jeppe Revall Frisvad, 2011
// Copyright (c) DTU Informatics 2011

#include <optix_world.h>
#include "AreaLight.h"
#include "structs.h"
#include "sampler.h"

using namespace optix;

#define INDIRECT

// Standard ray variables
rtDeclareVariable(Ray, ray, rtCurrentRay, );
rtDeclareVariable(float, t_hit, rtIntersectionDistance, );
rtDeclareVariable(PerRayData_radiance, prd_radiance, rtPayload, );
rtDeclareVariable(PerRayData_shadow,   prd_shadow,   rtPayload, );

// Variables for shading
rtDeclareVariable(float3, shading_normal, attribute shading_normal, );
rtDeclareVariable(float3, texcoord, attribute texcoord, ); 

// Material properties (corresponding to OBJ mtl params)
rtDeclareVariable(float3, Kd, , );
rtDeclareVariable(float3, Ka, , );

// Shadow variables
rtDeclareVariable(float, scene_epsilon, , );
rtDeclareVariable(rtObject, top_shadower, , );
rtDeclareVariable(unsigned int, shadow_ray_type, , );

#ifdef INDIRECT
// Recursive ray tracing variables
rtDeclareVariable(rtObject, top_object, , );
rtDeclareVariable(unsigned int, radiance_ray_type, , );
rtDeclareVariable(int, max_depth, , );
#endif

// Any hit program for shadows
RT_PROGRAM void any_hit_shadow()
{
  // this material is opaque, so it fully attenuates all shadow rays
  prd_shadow.attenuation = 0.0f;
  rtTerminateRay();
}

// Closest hit program for Lambertian shading using a triangle mesh as an area source.
// This one includes shadows.
RT_PROGRAM void arealight_shader() 
{
#ifdef INDIRECT
  if(prd_radiance.depth > max_depth)
  {
    prd_radiance.result = make_float3(0.0f);
    return;
  }
#endif
  float3 hit_pos = ray.origin + t_hit * ray.direction;
  float3 normal = normalize(rtTransformNormal(RT_OBJECT_TO_WORLD, shading_normal)); 
  float3 ffnormal = faceforward(normal, -ray.direction, normal); 
  const float3& rho_d = Kd;
  uint& t = prd_radiance.seed;
  float3 color = prd_radiance.emit ? Ka : make_float3(0.0f);

  // Direct illumination
  float3 w_i;
  float3 L_e;
  float dist;
  sample_center(hit_pos, w_i, L_e, dist);

  // Implement Lambertian reflection here, include shadow rays.
  //
  // Input:  
  // ray           (the ray that hit the material)
  // t_hit         (distance along ray to hit position)
  //
  // Output: 
  // prd_radiance  (per ray data, in particular prd_radiance.result which is the reflected radiance)
  //
  // Relevant data fields that are available (see above):
  // w_i           (sampled direction toward the light)
  // L_e           (emitted radiance received from the direction w_i)
  // dist          (distance to the sampled position on the light source)
  // hit_pos       (position where the ray hit the material)
  // ff_normal     (forward facing surface normal where the ray hit the material)
  // rho_d         (difuse reflectance of the material)
  // scene_epsilon (user defined epsilon for ray tracing)
  // top_shadower  (topmost object in hierarchy of shadow casting objects)
  //
  // Hint: Implement the function sample_center(...) in AreaLight.h first.

  Ray r=make_Ray(hit_pos,w_i,0,scene_epsilon,t_hit-scene_epsilon);
  PerRayData_radiance prd;
  rtTrace(top_shadower,r,prd);
  color=color+(rho_d/3.14)*prd.result*dot(w_i,ffnormal);
  
  //color+=rho_d;
#ifdef INDIRECT
  // Indirect illumination
#endif

  prd_radiance.result = color; 
}
