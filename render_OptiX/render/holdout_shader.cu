#include <optix.h>
#include <optix_math.h>
#include "Directional.h"
#include "structs.h"
#include "sampler.h"

using namespace optix;

//#define DIRECT
#define INDIRECT

// Standard ray variables
rtDeclareVariable(Ray, ray, rtCurrentRay, );
rtDeclareVariable(float, t_hit, rtIntersectionDistance, );
rtDeclareVariable(PerRayData_radiance, prd_radiance, rtPayload, );
rtDeclareVariable(PerRayData_shadow,   prd_shadow,   rtPayload, );

// Variables for shading
rtBuffer<Directional> lights;
rtDeclareVariable(float3, shading_normal, attribute shading_normal, );
rtDeclareVariable(float3, texcoord, attribute texcoord, ); 

// Material properties (corresponding to OBJ mtl params)
rtDeclareVariable(float3, Ka, , );

// Shadow variables
rtDeclareVariable(float, scene_epsilon, , );
rtDeclareVariable(rtObject, top_shadower, , );
rtDeclareVariable(unsigned int, shadow_ray_type, , );

// Recursive ray tracing variables
rtDeclareVariable(rtObject, top_object, , );
rtDeclareVariable(unsigned int, radiance_ray_type, , );
rtDeclareVariable(int, max_depth, , );

// Any hit program for shadows
RT_PROGRAM void any_hit_shadow()
{
  // this material is opaque, so it fully attenuates all shadow rays
  prd_shadow.attenuation = 0.0f;
  rtTerminateRay();
}

// Closest hit program for Lambertian shading using the basic light as a directional source.
// This one includes shadows.
RT_PROGRAM void holdout_shader() 
{ 
  if(prd_radiance.depth > max_depth)
  {
    prd_radiance.result = make_float3(0.0f);
    return;
  }

  float3 hit_pos = ray.origin + t_hit*ray.direction;
  float3 normal = normalize(rtTransformNormal(RT_OBJECT_TO_WORLD, shading_normal)); 
  float3 ffnormal = faceforward(normal, -ray.direction, normal); 

  Ray new_ray(hit_pos, ray.direction, radiance_ray_type, scene_epsilon, RT_DEFAULT_MAX);
  PerRayData_radiance prd_env;
  prd_env.emit = 1;
  prd_env.depth = prd_radiance.depth + 1;
  rtTrace(top_object, new_ray, prd_env);
  float3 rho_d = prd_env.result;

  float3 color = prd_radiance.emit ? Ka : make_float3(0.0f);
#ifdef DIRECT
  // Cast shadows due to direct illumination
  color += rho_d;
#endif
#ifdef INDIRECT
  // Ambient occlusion due to indirect illumination
  uint& t = prd_radiance.seed;

  color += rho_d;
#endif
  prd_radiance.result = color;
#if defined(DIRECT) && defined(INDIRECT)
  prd_radiance.result *= 0.5f;
#endif
}
