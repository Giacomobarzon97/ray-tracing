#include <optix_world.h>
#include "InvSphereMap.h"

using namespace optix;

void InvSphereMap::project_direction(const float3& d, float& u, float& v) const
{
	// Implement an inverse sphere map here.
	float3 ve = normalize(d);
	float teta = acos(ve.x);
	float fi = atan2(ve.y, ve.x);
	u = teta / M_PIf;
	v = fi / 2 * M_PIf;
}