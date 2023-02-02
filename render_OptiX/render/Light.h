/*
 * =====================================================================================
 *
 *       Filename:  LightTypes.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  02/24/2012 10:18:30 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *        Company:  
 *
 * =====================================================================================
 */

#ifndef LIGHT_H
#define LIGHT_H

enum LightType
{
	DIRECTIONAL_LIGHT_TYPE = 0,
	POINT_LIGHT_TYPE,
	AREA_LIGHT_TYPE
};

struct Light
{
	LightType type;
	optix::float3 emission;
	optix::float3 position; // only applicable for point lights
	optix::float3 direction; // only applicable for directional lights
	bool enabled;
};

#endif
