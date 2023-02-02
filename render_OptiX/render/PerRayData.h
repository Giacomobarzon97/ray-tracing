#ifndef PERRAYDATA_H
#define PERRAYDATA_H

#include <optix.h>

// Payload for radiance ray type
struct PerRayData_radiance
{
	optix::float3 result;
	float importance;
	int depth;
	unsigned int seed;
	bool specular_bounce;
	int colorband;
};

// Payload for shadow ray type
struct PerRayData_shadow
{
  float attenuation;
  float dist;
  optix::float3 shading_normal;
};

#endif // PERRAYDATA_H
