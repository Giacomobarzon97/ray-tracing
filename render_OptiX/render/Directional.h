// 02576 OptiX Rendering Framework
// Written by Jeppe Revall Frisvad, 2011
// Copyright (c) DTU Informatics 2011

#ifndef DIRECTIONAL_H
#define DIRECTIONAL_H

#include <optix.h>

struct Directional
{
  optix::float3 direction;
  optix::float3 emission;
};

#endif // DIRECTIONAL_H