// 02576 OptiX Rendering Framework
// Written by Jeppe Revall Frisvad, 2011
// Copyright (c) DTU Informatics 2011

#ifndef OBJSCENE_H
#define OBJSCENE_H

#include <string>
#include <vector>
#include <optixu/optixu_matrix_namespace.h>
#include <optixu/optixpp_namespace.h>
#include "SampleScene.h"

class ObjScene : public SampleScene
{
public:
  ObjScene(const std::vector<std::string>& obj_filenames, const std::string& shader_name, const std::string& env_filename)
    : filenames(obj_filenames),
      shadername(shader_name),
      envfile(env_filename), pixel_subdivs(1),
      frame(0u), deforming(true), use_tonemap(false)
  {
    setUseVBOBuffer(false);
    use_envmap = !envfile.empty();
    std::string modelname = filenames.back();
    std::string delimiter = ".";
    std::string name = modelname.substr(0, modelname.find_last_of(delimiter));
    setOutputFilename(name);
    start_time = sutil::currentTime();
  }

  virtual void initScene(InitialCameraData& camera_data);
  virtual void trace(const RayGenCameraData& camera_data);
  virtual bool keyPressed(unsigned char key, int x, int y);
  virtual optix::Buffer getOutputBuffer();

private:
  // Light sources
  void add_default_light();
  unsigned int extract_area_lights();

  // Geometry and transformation getters
  optix::Matrix4x4 get_object_transform(std::string filename);

  // Shaders
  void set_shader(int illum, optix::Program closest_hit_program);
  optix::Program get_shader(int illum);
  void set_shadow_shader(int illum, optix::Program any_hit_program);
  optix::Program get_shadow_shader(int illum);
  void load_shaders(optix::Program& closest_hit, optix::Program& any_hit);
  bool export_raw(std::string& name);
  bool import_raw(std::string& name);

  // Jitter samples for anti-aliasing
  void compute_jitters();

  std::vector<std::string> filenames;
  std::string shadername;
  std::string envfile;
  optix::GeometryGroup obj_group;
  std::vector<optix::uint2> lights;
  std::vector<optix::Program> shaders;
  std::vector<optix::Program> shadow_shaders;
  unsigned int pixel_subdivs;
  unsigned int frame;
  bool deforming;
  bool use_tonemap;
  bool use_envmap;

  const static int WIDTH;
  const static int HEIGHT;
  double start_time;
};

#endif // OBJSCENE_H