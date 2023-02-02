#ifndef POINTLIGHT_H
#define POINTLIGHT_H

#include <optix.h>

struct PointLight
{
  optix::float3 position;
  optix::float3 intensity;
};

#endif // POINTLIGHT_H
