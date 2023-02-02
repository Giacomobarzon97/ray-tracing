// 02576 OptiX Rendering Framework
// Written by Jeppe Revall Frisvad, 2011
// Copyright (c) DTU Informatics 2011

#include <optix_world.h>
#include "structs.h"

using namespace optix;

// Standard ray variables
rtDeclareVariable(PerRayData_radiance, prd_radiance, rtPayload, );
rtDeclareVariable(PerRayData_shadow, prd_shadow, rtPayload, );

// Variables for shading
rtDeclareVariable(float3, shading_normal, attribute shading_normal, );

// Any hit program for shadows
RT_PROGRAM void any_hit_shadow()
{
  prd_shadow.attenuation = 0.0f;
  rtTerminateRay();
}

// Closest hit program for drawing shading normals
RT_PROGRAM void normal_shader()
{
  float3 normal = normalize(rtTransformNormal(RT_OBJECT_TO_WORLD, shading_normal));
  prd_radiance.result = normal*0.5f + 0.5f;
}