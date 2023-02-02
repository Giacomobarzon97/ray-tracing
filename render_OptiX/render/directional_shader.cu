// 02576 OptiX Rendering Framework
// Written by Jeppe Revall Frisvad, 2011
// Copyright (c) DTU Informatics 2011

#include <optix_world.h>
#include "structs.h"
#include "Directional.h"

using namespace optix;

// Standard ray variables
rtDeclareVariable(optix::Ray, ray, rtCurrentRay, );
rtDeclareVariable(float, t_hit, rtIntersectionDistance, );
rtDeclareVariable(PerRayData_radiance, prd_radiance, rtPayload, );

// Variables for shading
rtBuffer<Directional> lights;
rtDeclareVariable(float3, shading_normal, attribute shading_normal, );
rtDeclareVariable(float3, texcoord, attribute texcoord, ); 

// Material properties (corresponding to OBJ mtl params)
rtDeclareVariable(float3, Kd, , );
rtDeclareVariable(float3, Ka, , );

// Any hit program for shadows
RT_PROGRAM void any_hit_shadow() { rtTerminateRay(); }

// Closest hit program for Lambertian shading using the basic light as a directional source
RT_PROGRAM void directional_shader() 
{ 
  const float3& emission = Ka;
  const float3& rho_d = Kd;
  float3 normal = normalize(rtTransformNormal(RT_OBJECT_TO_WORLD, shading_normal)); 
  float3 ffnormal = faceforward(normal, -ray.direction, normal); 

  // Implement Lambertian reflection here.
  //
  // Input:  
  // ray           (the ray that hit the material)
  //
  // Output: 
  // prd_radiance  (per ray data, in particular prd_radiance.result which is the reflected radiance)
  //
  // Relevant data fields that are available (see above):
  // lights        (vector of pointers to the lights in the scene)
  // ffnormal     (forward facing surface normal where the ray hit the material)
  // rho_d         (difuse reflectance of the material)
  //
  // Hint: (a) Elements in the lights vector have the data type Directional,
  //       which is defined in Directional.h.
  //       (b) OptiX includes a function dot(v, w) to take the dot product
  //       of two vectors v and w.
  float3 color=make_float3(0,0,0);
  for(int i=0;i<lights.size();i++){
	color=color+(rho_d/3.14)*lights[i].emission*dot(lights[i].direction,ffnormal);
  }

  prd_radiance.result = color + emission; 
}
