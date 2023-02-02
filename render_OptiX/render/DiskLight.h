#ifndef DISKLIGHT_H
#define DISKLIGHT_H

#include <optix.h>
#include <optix_math.h>

struct DiskLight
{
	optix::float3 emission;
	optix::float3 center;
	optix::float3 normal;
	optix::float3 tangent;
	optix::float3 bitangent;
	float radius;
	float area;
	/*
	__device__ __inline__ void sample(const optix::float3& pos, optix::float3& dir, optix::float3& L, float& dist, unsigned int& seed)
	{	
		
	
		L = emission*area;
	}
	*/

};



#endif // DISKLIGHT_H