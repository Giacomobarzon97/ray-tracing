#ifndef SAMPLER_H
#define SAMPLER_H

#include <optix_world.h>
#include "../cuda/random.h"

// Given a direction vector v sampled around the z-axis of a
// local coordinate system, this function applies the same
// rotation to v as is needed to rotate the z-axis to the
// actual direction n that v should have been sampled around
// [Frisvad, Journal of Graphics Tools 16, 2012;
//  Duff et al., Journal of Computer Graphics Techniques 6, 2017].
__inline__ __device__ void rotate_to_normal(const optix::float3& normal, optix::float3& v)
{
  float sign = copysignf(1.0f, normal.z);
  const float a = -1.0f/(1.0f + fabsf(normal.z));
  const float b = normal.x*normal.y*a;
  v = optix::make_float3(1.0f + normal.x*normal.x*a, b, -sign*normal.x)*v.x
    + optix::make_float3(sign*b, sign*(1.0f + normal.y*normal.y*a), -normal.y)*v.y
    + normal*v.z;
}

// Given spherical coordinates, where theta is the 
// polar angle and phi is the azimuthal angle, this
// function returns the corresponding direction vector
__inline__ __device__ optix::float3 spherical_direction(float sin_theta, float cos_theta, float phi)
{
  float sin_phi, cos_phi;
  sincosf(phi, &sin_phi, &cos_phi);
  return optix::make_float3(sin_theta*cos_phi, sin_theta*sin_phi, cos_theta);
}

__inline__ __device__ optix::float3 sample_hemisphere(const optix::float3& normal, optix::uint& t)
{
  // Get random numbers

  // Calculate new direction as if the z-axis were the normal

  // Rotate from z-axis to actual normal and return

  return normal;
}

__inline__ __device__ optix::float3 sample_cosine_weighted(const optix::float3& normal, optix::uint& t)
{
  // Get random numbers

  // Calculate new direction as if the z-axis were the normal

  // Rotate from z-axis to actual normal and return

  return normal;
}

__inline__ __device__ float3 sample_isotropic(optix::float3& forward, optix::uint& t)
{

  // Rotate from z-axis to actual normal and return

  return forward;
}

__inline__ __device__ float3 sample_HG(optix::float3& forward, float g, optix::uint& t)
{

  // Calculate new direction as if the z-axis were the forward direction

  // Rotate from z-axis to actual normal and return

  return forward;
}

__inline__ __device__ float3 sample_barycentric(optix::uint& t)
{
  // Get random numbers

  // Calculate Barycentric coordinates

  // Return barycentric coordinates
  
  return make_float3(0.0f);
}

__inline__ __device__ optix::float3 sample_Phong_distribution(const optix::float3& normal, const optix::float3& dir, float shininess, optix::uint& t)
{
  // Get random numbers

  // Calculate sampled direction as if the z-axis were the reflected direction

  // Rotate from z-axis to actual reflected direction

  return normal;
}

__inline__ __device__ optix::float3 sample_Blinn_normal(const optix::float3& normal, float shininess, optix::uint& t)
{
  // Get random numbers

  // Calculate sampled half-angle vector as if the z-axis were the normal

  // Rotate from z-axis to actual normal

  return normal;
}

__inline__ __device__ optix::float3 sample_Beckmann_normal(const optix::float3& normal, float width, optix::uint& t)
{
  // Get random numbers

  // Calculate sampled half-angle vector as if the z-axis were the normal

  // Rotate from z-axis to actual normal

  return normal;
}

__inline__ __device__ optix::float3 sample_GGX_normal(const optix::float3& normal, float roughness, optix::uint& t)
{
  // Get random numbers

  // Calculate sampled half-angle vector as if the z-axis were the normal

  // Rotate from z-axis to actual normal

  return normal;
}

#endif // SAMPLER_H