// 02576 OptiX Rendering Framework
// Written by Jeppe Revall Frisvad, 2011
// Copyright (c) DTU Informatics 2011


#include <optix_world.h>
#include "structs.h"
#include "envmap.h"

using namespace optix;

// Standard ray variables
rtDeclareVariable(PerRayData_radiance, prd_radiance, rtPayload, );

// Variables for shading
rtDeclareVariable(Ray, ray, rtCurrentRay, );

// Miss program returning background color
RT_PROGRAM void miss()
{
  if(prd_radiance.emit)
    prd_radiance.result = env_lookup(ray.direction);
  else
    prd_radiance.result = make_float3(0.0f);
}
