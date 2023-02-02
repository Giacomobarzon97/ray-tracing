#include <optix.h>
#include <optix_math.h>

using namespace optix;

// Window variables
rtBuffer<float4, 2> output_buffer;
rtBuffer<float4, 2> tonemap_output_buffer;
rtDeclareVariable(uint2, launch_index, rtLaunchIndex, );

RT_PROGRAM void tonemap_camera()
{
  float4 c = output_buffer[launch_index];
  tonemap_output_buffer[launch_index] = make_float4(powf(c.x, 1.0f/1.8f),
                                                    powf(c.y, 1.0f/1.8f),
                                                    powf(c.z, 1.0f/1.8f), c.w);
}
