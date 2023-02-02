
#include <optix_world.h>
#include "PerRayData.h"

// Standard ray variables
rtDeclareVariable(PerRayData_radiance, prd_radiance, rtPayload, );
rtDeclareVariable(optix::Ray, ray, rtCurrentRay, );

// Variables for shading
rtDeclareVariable(float3, bg_color, , );
rtTextureSampler<float4, 2> hdr_map; 

// Miss program returning background color
RT_PROGRAM void miss()
{
  float theta = atan2f( ray.direction.x, ray.direction.z );
  float phi   = M_PIf * 0.5f -  acosf( ray.direction.y );
  float u     = (theta + M_PIf) * (0.5f * M_1_PIf);
  float v     = 0.5f * ( 1.0f + sin(phi) );
  prd_radiance.result = make_float3( tex2D(hdr_map, u, v) );
}
