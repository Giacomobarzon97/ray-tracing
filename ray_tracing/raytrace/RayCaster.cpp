// 02562 Rendering Framework
// Written by Jeppe Revall Frisvad, 2011
// Copyright (c) DTU Informatics 2011

#include <iostream>
#include <optix_world.h>
#include "mt_random.h"
#include "Shader.h"
#include "HitInfo.h"
#include "RayCaster.h"

using namespace std;
using namespace optix;

float3 RayCaster::compute_pixel(unsigned int x, unsigned int y) const
{
  // Use the scene and its camera
  // to cast a ray that computes the color of the pixel at index (x, y).
  //
  // Input:  x, y        (pixel index)
  //
  // Return: Result of tracing a ray through the pixel at index (x, y).
  //
  // Relevant data fields that are available (see RayCaster.h and Scene.h)
  // win_to_ip           (pixel size (width, height) in the image plane)
  // lower_left          (lower left corner of the film in the image plane)
  // scene               (scene with access to the closest_hit function)
  // scene->get_camera() (camera in the scene)
  //
  // Hints: (a) Use the function get_shader(...) to get the shader of the
  //            intersected material after the ray has been traced.
  //        (b) Use get_background(...) if the ray does not hit anything.

  //return (ray.direction + 1) / 2;
  Camera* camera = scene->get_camera();
  float2 coords = make_float2(lower_left.x + win_to_ip.x * x, lower_left.y + win_to_ip.y * y);
  float3 color = make_float3(0.0f);
  
  for (int i = 0; i < jitter.size(); i++) {
      Ray ray = camera->get_ray(coords+jitter[i]);

      HitInfo hit = HitInfo();
      bool has_hit = scene->closest_hit(ray, hit);
      if (has_hit) {
          color=color+ get_shader(hit)->shade(ray, hit);
      }
      else {
          color = color + get_background(ray.direction);
      }
  }
  return color / jitter.size();
}

float3 RayCaster::get_background(const float3& dir) const
{ 
  if(!sphere_tex)
    return background;
  return make_float3(sphere_tex->sample_linear(dir));
}

void RayCaster::increment_pixel_subdivs()
{
  ++subdivs;
  compute_jitters();
  cout << "Rays per pixel: " << subdivs*subdivs << endl;
}

void RayCaster::decrement_pixel_subdivs()
{
  if(subdivs > 1)
  {
    --subdivs;
    compute_jitters();
  }
  cout << "Rays per pixel: " << subdivs*subdivs << endl;
}

void RayCaster::compute_jitters()
{
  float aspect = width/static_cast<float>(height);
  win_to_ip.x = win_to_ip.y = 1.0f/static_cast<float>(height);
  lower_left = (win_to_ip - make_float2(aspect, 1.0f))*0.5f;
  step = win_to_ip/static_cast<float>(subdivs);

  jitter.resize(subdivs*subdivs);
  for(unsigned int i = 0; i < subdivs; ++i)
    for(unsigned int j = 0; j < subdivs; ++j)
      jitter[i*subdivs + j] = make_float2(safe_mt_random() + j, safe_mt_random() + i)*step - win_to_ip*0.5f; 
}
