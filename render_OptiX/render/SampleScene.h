
/*
 * Copyright (c) 2008 - 2009 NVIDIA Corporation.  All rights reserved.
 *
 * NVIDIA Corporation and its licensors retain all intellectual property and proprietary
 * rights in and to this software, related documentation and any modifications thereto.
 * Any use, reproduction, disclosure or distribution of this software and related
 * documentation without an express license agreement from NVIDIA Corporation is strictly
 * prohibited.
 *
 * TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, THIS SOFTWARE IS PROVIDED *AS IS*
 * AND NVIDIA AND ITS SUPPLIERS DISCLAIM ALL WARRANTIES, EITHER EXPRESS OR IMPLIED,
 * INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE.  IN NO EVENT SHALL NVIDIA OR ITS SUPPLIERS BE LIABLE FOR ANY
 * SPECIAL, INCIDENTAL, INDIRECT, OR CONSEQUENTIAL DAMAGES WHATSOEVER (INCLUDING, WITHOUT
 * LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS, BUSINESS INTERRUPTION, LOSS OF
 * BUSINESS INFORMATION, OR ANY OTHER PECUNIARY LOSS) ARISING OUT OF THE USE OF OR
 * INABILITY TO USE THIS SOFTWARE, EVEN IF NVIDIA HAS BEEN ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGES
 */

#pragma once


#include <optixu/optixpp_namespace.h>
#include <sutil.h>

//-----------------------------------------------------------------------------
// 
// SampleScene virtual class
//
//-----------------------------------------------------------------------------

class SampleScene 
{
protected:
  typedef optix::float3 float3;
public:

  // Used to pass current camera info to the ray gen program at render time.
  // eye - Camera position
  // W   - Viewing direction.             length(W) -> focal distance
  // U   - Horizontal axis of view plane. length(U) -> view plane width at focal distance 
  // V   - Vertical axis of view plane.   length(V) -> view plane height at focal distance 
  struct RayGenCameraData 
  { 
    RayGenCameraData() {}
    RayGenCameraData( const float3& m_eye, const float3& m_U, const float3& m_V, const float3& m_W )
      : eye(m_eye), U(m_U), V(m_V), W(m_W) {}
    float3 eye;
    float3 U;
    float3 V;
    float3 W;
  };
    
  // Used to specify initial viewing parameters 
  struct InitialCameraData
  {
    InitialCameraData() {}
    InitialCameraData( const std::string& camera_string );
    InitialCameraData( float3 m_eye, float3 m_lookat, float3 m_up, float  m_vfov )
      : eye(m_eye), lookat(m_lookat), up(m_up), vfov(m_vfov) {}

    float3 eye;
    float3 lookat;
    float3 up;
    float  vfov;
  };

  SampleScene();
  virtual ~SampleScene() {}

  void createContext();

  void  signalCameraChanged() { m_camera_changed = true; }

  void  setNumDevices( int ndev );
  void  enableCPURendering(bool enable);
  void  incrementCPUThreads(int delta); // can pass in negative values

  void  setUseVBOBuffer( bool onoff ) { m_use_vbo_buffer = onoff; }
  bool  usesVBOBuffer() { return m_use_vbo_buffer; }

  void setOutputFilename(const std::string& ofile) { outfile = ofile; }

  // Create path to the SDK's ptx file for given base filename and target (.ptx
  // extension is added).  The returned pointer is valid until the next
  // invocation of ptxpath.
  static const char* const ptxpath( const std::string& target, const std::string& base );

  //----------------------------------------------------------------------------
  // Pure virtual interface to be implemented
  //----------------------------------------------------------------------------

  // Create the optix scene and return initial viewing parameters
  virtual void   initScene( InitialCameraData& camera_data )=0;
  
  // Update camera shader with new viewing params and then trace
  virtual void   trace( const RayGenCameraData& camera_data )=0;
 
  // Return the output buffer to be displayed
  virtual optix::Buffer getOutputBuffer()=0;
 
  //----------------------------------------------------------------------------
  // Optional virtual interface
  //----------------------------------------------------------------------------
  
  // This cleans up the Context.  If you override it, you should call
  // SampleScene::cleanUp() explicitly.
  virtual void   cleanUp();

  // Will resize the output buffer (which might use a VBO) then call doResize.
  // Override this if you want to handle ALL buffer resizes yourself.
  virtual void   resize(unsigned int width, unsigned int height);

  // Where derived classes should handle resizing all buffers except outputBuffer.
  virtual void   doResize(unsigned int width, unsigned int height) {}

  // Use this to add additional keys. Some are already handled but
  // can be overridden.  Should return true if key was handled, false otherwise.
  virtual bool   keyPressed(unsigned char key, int x, int y);

  // Accessor
  optix::Context& getContext() { return m_context; }

protected:
  optix::Buffer createOutputBuffer(RTformat format, unsigned int width, unsigned int height);

  optix::Context m_context;

  bool   m_camera_changed;
  bool   m_use_vbo_buffer;
  int    m_num_devices;
  bool   m_cpu_rendering_enabled;

  std::string outfile;

private:
  // Checks to see if CPU mode has been enabled and sets the appropriate flags.
  void updateCPUMode();
};
