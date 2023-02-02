// 02576 OptiX Rendering Framework
// Written by Jeppe Revall Frisvad, 2011
// Copyright (c) DTU Informatics 2011

#include <optix_world.h>
#include "structs.h"

// Standard ray variables
rtDeclareVariable(PerRayData_radiance, prd_radiance, rtPayload, );

// Variables for shading
rtDeclareVariable(float3, bg_color, , );

// Miss program returning background color
RT_PROGRAM void miss()
{
  prd_radiance.result = bg_color;
}
