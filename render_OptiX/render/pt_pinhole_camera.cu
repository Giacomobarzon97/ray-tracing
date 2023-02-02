// 02576 OptiX Rendering Framework
// Written by Jeppe Revall Frisvad, 2011
// Copyright (c) DTU Informatics 2011

#include <optix_world.h>
#include "../cuda/helpers.h"
#include "../cuda/random.h"
#include "structs.h"

using namespace optix;

// Camera variables
rtDeclareVariable(float3,   eye, , );
rtDeclareVariable(float3,   U, , );
rtDeclareVariable(float3,   V, , );
rtDeclareVariable(float3,   W, , );

// Ray generation variables
rtDeclareVariable(float,    scene_epsilon, , );
rtDeclareVariable(rtObject, top_object, , );
rtDeclareVariable(uint,     radiance_ray_type, , );
rtDeclareVariable(uint,     frame, , );

// Window variables
rtBuffer<float4, 2> output_buffer;
rtDeclareVariable(uint2, launch_index, rtLaunchIndex, );
rtDeclareVariable(uint2, launch_dim,   rtLaunchDim, );

// Exception and debugging variables
rtDeclareVariable(float3, bad_color, , );
rtDeclareVariable(float, time_view_scale, , ) = 1e-6f;

//#define TIME_VIEW

RT_PROGRAM void pt_pinhole_camera()
{
#ifdef TIME_VIEW
  clock_t t0 = clock(); 
#endif
  PerRayData_radiance prd;
   prd.emit = 1;
  prd.depth = 0;
  prd.seed = tea<16>(launch_dim.x*launch_index.y+launch_index.x, frame);
  prd.colorband = -1;
  
  float2 jitter = make_float2(rnd(prd.seed), rnd(prd.seed));
  float2 ip_coords = (make_float2(launch_index) + jitter) / make_float2(launch_dim) * 2.0f - 1.0f;
  float3 origin = eye;
  float3 direction = normalize(ip_coords.x*U + ip_coords.y*V + W);
  Ray ray(origin, direction, radiance_ray_type, scene_epsilon, RT_DEFAULT_MAX);

  rtTrace(top_object, ray, prd);

#ifdef TIME_VIEW
  clock_t t1 = clock(); 
 
  float expected_fps   = 1.0f;
  float pixel_time     = ( t1 - t0 ) * time_view_scale * expected_fps;
  output_buffer[launch_index] = make_color( make_float3(  pixel_time ) ); 
#else
  float4 curr_sum = (frame != 0) ? output_buffer[launch_index] * ((float)frame) : make_float4(0.0f);
  output_buffer[launch_index] = (make_float4(prd.result, 0.0f) + curr_sum) / ((float)(frame + 1)) ;
#endif
}

RT_PROGRAM void exception()
{
  const unsigned int code = rtGetExceptionCode();
  rtPrintf( "Caught exception 0x%X at launch index (%d,%d)\n", code, launch_index.x, launch_index.y );
  output_buffer[launch_index] = make_float4(bad_color,1.0f);
}
