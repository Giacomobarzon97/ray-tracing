#pragma once
#include <optix_world.h>

// Payload for radiance ray type
struct PerRayData_radiance
{
  optix::float3 result;
  int emit;
  int depth;
  unsigned int seed;
  int colorband;
};

// Payload for shadow ray type
struct PerRayData_shadow
{
  float attenuation;
};

