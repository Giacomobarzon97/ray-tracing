// 02576 OptiX Rendering Framework
// Written by Jeppe Revall Frisvad, 2011
// Copyright (c) DTU Informatics 2011

#include <iostream>
#include <fstream>
#include <string>
#include <climits>
#include <optix_world.h>
#include <sutil.h>
#include <OptiXMesh.h>
#include "../cuda/random.h"
#include "Directional.h"
#include "PointLight.h"
#include "ObjScene.h"
#include "structs.h"

using namespace std;
using namespace optix;
using namespace sutil;

const int ObjScene::WIDTH = 800; // 512;
const int ObjScene::HEIGHT = 450; // 512;

const unsigned int SAMPLES_FRAME = 5000;

enum DefaultLight { DefaultDirectional, DefaultPointLight };
DefaultLight default_light_type = DefaultLight::DefaultDirectional;

const float3 default_light_direction = normalize(make_float3(-1.0f));
const float3 default_light_radiance = make_float3(M_PIf);

const float3 default_pointlight_position = make_float3(0.08f, 0.1f, 0.11f);
const float3 default_pointlight_intensity = make_float3(0.0f);

const float3 EYE = make_float3(0.0f, 0.0f, 2.26f);
const float3 DIR = make_float3(0.0f, 0.0f, -1.0f);
const float3 UP = make_float3(0.0f, 1.0f, 0.0f);
const float VFOV = 53.1301023542f;

const float3 bgcolor = make_float3(0.8f, 0.88f, 0.97f);
const uint max_depth = 10;

bool ObjScene::keyPressed(unsigned char key, int x, int y)
{
  switch(key)
  {
  case 't':
    use_tonemap = !use_tonemap;
    cout << "Tone mapping switched " << (use_tonemap ? "on." : "off.") << endl;
    return true;
  // Use '+' and '-' to increase or decrease the number of
  // jitter samples per pixel in a simple ray tracing
  case '+':
    ++pixel_subdivs;
    compute_jitters();
    cout << "Rays per pixel: " << pixel_subdivs*pixel_subdivs << endl;
    return true;
  case '-':
    if(pixel_subdivs > 1)
    {
      --pixel_subdivs;
      compute_jitters();
    }
    cout << "Rays per pixel: " << pixel_subdivs*pixel_subdivs << endl;
    return true;
  case 'e':
    return export_raw(outfile);
  case 'i':
    return import_raw(outfile);
  }
  return SampleScene::keyPressed(key, x, y);
}

void ObjScene::load_shaders(Program& closest_hit, Program& any_hit)
{
  // Load default shader (if no shader was chosen, use the normal shader)
  if(shadername.empty())
    shadername = "normal_shader";
  closest_hit = m_context->createProgramFromPTXString(getPtxString("render", (shadername + ".cu").c_str()), shadername);
  any_hit = m_context->createProgramFromPTXString(getPtxString("render", (shadername + ".cu").c_str()), "any_hit_shadow");
  set_shader(1, closest_hit);
  set_shadow_shader(1, any_hit);

  // Load shaders attached to other MTL illumination models
  Program mirror = m_context->createProgramFromPTXString(getPtxString("render", "mirror_shader.cu"), "mirror_shader");
  set_shader(3, mirror);
  Program transparent = m_context->createProgramFromPTXString(getPtxString("render", "transparent_shader.cu"), "transparent_shader");
  set_shader(4, transparent);
  Program holdout = m_context->createProgramFromPTXString(getPtxString("render", "holdout_shader.cu"), "holdout_shader");
  set_shader(30, holdout);
}

void ObjScene::initScene(InitialCameraData& camera_data)
{
  // Setup m_context
  m_context->setRayTypeCount(2);
  m_context->setEntryPointCount(2);
  m_context->setStackSize(20000);
  m_context->setMaxCallableProgramDepth(max_depth + 3);
  m_context->setMaxTraceDepth(max_depth + 3);

  // m_context->setPrintEnabled(true);
  // m_context->setPrintBufferSize(2048);

  m_context["radiance_ray_type"]->setUint(0u);
  m_context["shadow_ray_type"]->setUint(1u);
  m_context["max_depth"]->setInt(max_depth);
  m_context["max_splits"]->setInt(0);
  m_context["output_buffer"]->set(createOutputBuffer(RT_FORMAT_FLOAT4, WIDTH, HEIGHT));

  // Ray generation program
  compute_jitters();
  const string ptx = getPtxString("render", "pinhole_camera.cu");
  const string camera_name = "pinhole_camera";
  Program ray_gen_program = m_context->createProgramFromPTXString(ptx, camera_name);
  m_context->setRayGenerationProgram(0, ray_gen_program);

  // Exception / miss programs
  m_context->setExceptionProgram(0, m_context->createProgramFromPTXString(ptx, "exception"));
  m_context->setMissProgram(0, m_context->createProgramFromPTXString(getPtxString("render", "constantbg.cu"), "miss"));
  m_context["bad_color"]->setFloat(0.0f, 1.0f, 0.0f);
  m_context["bg_color"]->setFloat(bgcolor.x, bgcolor.y, bgcolor.z);

  // Closest hit and any hit programs
  Program closest_hit, any_hit;
  load_shaders(closest_hit, any_hit);

  // Create group for scene objects and add acceleration structure
  obj_group = m_context->createGeometryGroup();
  obj_group->setChildCount(static_cast<unsigned int>(filenames.size()));
  Acceleration acceleration = m_context->createAcceleration("Trbvh", "Bvh");
  obj_group->setAcceleration(acceleration);
  acceleration->markDirty();

  // We need the scene bounding box for placing the camera
  Aabb bbox;

  // Load geometry from OBJ files into the group of scene objects
  for(unsigned int i = 0; i < filenames.size(); ++i)
  {
    OptiXMesh mesh;
    mesh.context = m_context;
    loadMesh(filenames[i], mesh, get_object_transform(filenames[i]));
    size_t idx = filenames[i].find_last_of("\\/") + 1;
    if(idx < filenames[i].length() && filenames[i].compare(idx, 5, "plane") != 0)
    {
      bbox.include(mesh.bbox_min);
      bbox.include(mesh.bbox_max);
    }

    // Set material shaders
    GeometryInstance& gi = mesh.geom_instance;
    for(unsigned int k = 0; k < gi->getMaterialCount(); ++k)
    {
      Material& m = gi->getMaterial(k);
      int illum = m["illum"]->getInt();
      Program shader = get_shader(illum);
      Program shadow_shader = get_shadow_shader(illum);
      if(!shader.get())
        shader = closest_hit;
      if(!shadow_shader.get())
        shadow_shader = any_hit;
      m->setClosestHitProgram(0u, shader);
      m->setAnyHitProgram(1u, shadow_shader);
    }

    // Add geometry group to the group of scene objects
    obj_group->setChild(i, gi);
  }

  // Add light sources depending on chosen shader
  if(shadername == "arealight_shader")
  {
    if(!extract_area_lights())
    {
      cerr << "Error: no area lights in scene. "
           << "You cannot use the area light shader if there are no emissive objects in the scene. "
           << "Objects are emissive if their ambient color is not zero."
           << endl;
      m_context->destroy();
      exit(0);
    }
  }
  else
    add_default_light();

  // Set top level geometry in acceleration structure. 
  // The default used by the ObjLoader is SBVH.
  m_context["top_object"]->set(obj_group);
  m_context["top_shadower"]->set(obj_group);

  // Set up camera
  float max_dim = bbox.extent(bbox.longestAxis());
  float3 eye = bbox.center();
  eye.z += 1.5f*max_dim;
  //*
  camera_data = InitialCameraData( eye,                             // eye
                                   bbox.center(),                   // lookat
                                   make_float3( 0.0f, 1.0f, 0.0f ), // up
                                   53.1301f );                      // vfov */
  /*
  camera_data = InitialCameraData(EYE,                         // eye
                                  EYE + DIR,                   // lookat
                                  UP,                          // up
                                  VFOV);                      // */

// Declare camera variables.  The values do not matter, they will be overwritten in trace.
  m_context["eye"]->setFloat(make_float3(0.0f, 0.0f, 0.0f));
  m_context["U"]->setFloat(make_float3(0.0f, 0.0f, 0.0f));
  m_context["V"]->setFloat(make_float3(0.0f, 0.0f, 0.0f));
  m_context["W"]->setFloat(make_float3(0.0f, 0.0f, 0.0f));

  // Set ray tracing epsilon for intersection tests
  float scene_epsilon = 1.e-2f; // *max_dim;
  m_context["scene_epsilon"]->setFloat(scene_epsilon);

  // Load environment map
  TextureSampler envmap;
  if(use_envmap)
  {
    envmap = loadTexture(m_context, envfile, bgcolor);
    m_context["envmap"]->setTextureSampler(envmap);
    m_context->setMissProgram(0, m_context->createProgramFromPTXString(getPtxString("render", "envmap_background.cu"), "miss"));
  }

  // Tone mapping pass
  m_context["tonemap_output_buffer"]->set(createOutputBuffer(RT_FORMAT_FLOAT4, WIDTH, HEIGHT));
  {
    std::string ptx = getPtxString("render", "tonemap_camera.cu");
    Program ray_gen_program = m_context->createProgramFromPTXString(ptx, "tonemap_camera");
    m_context->setRayGenerationProgram(1, ray_gen_program);
  }

  // Environment cameras
  unsigned int env_entry_point = 0;
  RTsize env_tex_width, env_tex_height;
  if(use_envmap)
  {
    std::string ptx = getPtxString("render", "env_cameras.cu");
    envmap.get()->getBuffer(0, 0)->getSize(env_tex_width, env_tex_height);
    m_context["env_luminance"]->set(m_context->createBuffer(RT_BUFFER_OUTPUT, RT_FORMAT_FLOAT, env_tex_width, env_tex_height));
    {
      Program ray_gen_program = m_context->createProgramFromPTXString(ptx, "env_luminance_camera");
      env_entry_point = m_context->getEntryPointCount();
      m_context->setEntryPointCount(env_entry_point + 1);
      m_context->setRayGenerationProgram(env_entry_point, ray_gen_program);
    }
    m_context["marginal_f"]->set(m_context->createBuffer(RT_BUFFER_OUTPUT, RT_FORMAT_FLOAT, env_tex_height));
    {
      Program ray_gen_program = m_context->createProgramFromPTXString(ptx, "env_marginal_camera");
      unsigned int entry_point = m_context->getEntryPointCount();
      m_context->setEntryPointCount(entry_point + 1);
      m_context->setRayGenerationProgram(entry_point, ray_gen_program);
    }
    m_context["marginal_pdf"]->set(m_context->createBuffer(RT_BUFFER_OUTPUT, RT_FORMAT_FLOAT, env_tex_height));
    m_context["conditional_pdf"]->set(m_context->createBuffer(RT_BUFFER_OUTPUT, RT_FORMAT_FLOAT, env_tex_width, env_tex_height));
    m_context["marginal_cdf"]->set(m_context->createBuffer(RT_BUFFER_OUTPUT, RT_FORMAT_FLOAT, env_tex_height));
    m_context["conditional_cdf"]->set(m_context->createBuffer(RT_BUFFER_OUTPUT, RT_FORMAT_FLOAT, env_tex_width, env_tex_height));
    {
      Program ray_gen_program = m_context->createProgramFromPTXString(ptx, "env_pdf_camera");
      unsigned int entry_point = m_context->getEntryPointCount();
      m_context->setEntryPointCount(entry_point + 1);
      m_context->setRayGenerationProgram(entry_point, ray_gen_program);
    }
  }

  // Prepare to run 
  m_context->validate();
  m_context->compile();

  // Environment importance sampling pre-pass
  if(env_entry_point)
  {
    m_context->launch(env_entry_point, env_tex_width, env_tex_height);
    m_context->launch(env_entry_point + 1, env_tex_width, env_tex_height);
    m_context->launch(env_entry_point + 2, env_tex_width, env_tex_height);
  }

  double current_time;
  current_time = sutil::currentTime();
  cout << "Start-up time: " << current_time - start_time << endl;
}

void ObjScene::trace(const RayGenCameraData& camera_data)
{
  m_context["eye"]->setFloat(camera_data.eye);
  m_context["U"]->setFloat(camera_data.U);
  m_context["V"]->setFloat(camera_data.V);
  m_context["W"]->setFloat(camera_data.W);

  if(m_camera_changed)
  {
    frame = 0;
    m_camera_changed = false;
  }
  m_context["frame"]->setUint(frame++);
  if(frame % 10 == 0) cout << "Frame: " << frame << endl;

  if(deforming)
    obj_group->getAcceleration()->markDirty();

  // Launch the ray tracer
  Buffer buffer = m_context["output_buffer"]->getBuffer();
  RTsize buffer_width, buffer_height;
  buffer->getSize(buffer_width, buffer_height);
  m_context->launch(0, static_cast<unsigned int>(buffer_width), static_cast<unsigned int>(buffer_height));

  // Apply tone mapping
  if(use_tonemap)
    m_context->launch(1, static_cast<unsigned int>(buffer_width), static_cast<unsigned int>(buffer_height));
}

Buffer ObjScene::getOutputBuffer()
{
  if(use_tonemap)
    return m_context["tonemap_output_buffer"]->getBuffer();
  return m_context["output_buffer"]->getBuffer();
}

void ObjScene::add_default_light()
{
  switch(default_light_type)
  {
  case DefaultDirectional:
    {
      Directional light = { normalize(default_light_direction), default_light_radiance };

      Buffer light_buffer = m_context->createBuffer(RT_BUFFER_INPUT);
      light_buffer->setFormat(RT_FORMAT_USER);
      light_buffer->setElementSize(sizeof(Directional));
      light_buffer->setSize(1);
      memcpy(light_buffer->map(), &light, sizeof(Directional));
      light_buffer->unmap();
      m_context[ "lights" ]->set(light_buffer);
    }
    break;
  case DefaultPointLight:
    {
      PointLight light = { default_pointlight_position, default_pointlight_intensity };

      Buffer light_buffer = m_context->createBuffer(RT_BUFFER_INPUT);
      light_buffer->setFormat(RT_FORMAT_USER);
      light_buffer->setElementSize(sizeof(PointLight));
      light_buffer->setSize(1);
      memcpy(light_buffer->map(), &light, sizeof(PointLight));
      light_buffer->unmap();
      m_context[ "lights" ]->set(light_buffer);
    }
    break;
  }
}

unsigned int ObjScene::extract_area_lights()
{
  for(unsigned int i = 0; i < obj_group->getChildCount(); ++i)
  {
    float emission[3];
    GeometryInstance& gi = obj_group->getChild(i);
    for(unsigned int j = 0; j < gi->getMaterialCount(); ++j)
    {
      Material& m = gi->getMaterial(j);

      rtVariableGet3fv(m["Ka"]->get(), emission);
      bool emissive = false;
      for(unsigned int k = 0; k < 3; ++k)
        emissive = emissive || emission[k] > 0.0f;

      if(emissive)
        lights.push_back(make_uint2(i, j));
    }
  }
  unsigned int counter = 0;
  unsigned int offset = 0;
  vector<int3> triangles;
  for(unsigned int j = 0; j < lights.size(); ++j)
  {
    unsigned int found = static_cast<unsigned int>(triangles.size());
    uint2 light = lights[j];
    GeometryInstance& gi = obj_group->getChild(light.x);
    Buffer mbuf = gi["material_buffer"]->getBuffer();
    Buffer ibuf = gi["index_buffer"]->getBuffer();
    GeometryTriangles& g = gi->getGeometryTriangles();
    int32_t* mat_idxs = reinterpret_cast<int32_t*>(mbuf->map());
    int32_t* idxs = reinterpret_cast<int32_t*>(ibuf->map());
    for(unsigned int i = 0; i < g->getPrimitiveCount(); ++i)
      if(mat_idxs[i] == static_cast<int32_t>(light.y))
        triangles.push_back(make_int3(idxs[i*3], idxs[i*3 + 1], idxs[i*3 + 2]));
    mbuf->unmap();
    ibuf->unmap();
    if(triangles.size() > found)
    {
      offset = found;
      Material& m = gi->getMaterial(light.y);
      float emission[3];
      rtVariableGet3fv(m["Ka"]->get(), emission);
      m_context["light_emission"]->set3fv(emission);
      m_context["light_verts"]->setBuffer(gi["vertex_buffer"]->getBuffer());
      m_context["light_norms"]->setBuffer(gi["normal_buffer"]->getBuffer());
      ++counter;
    }
  }
  if(counter > 1)
    cerr << "Warning: Multiple area lights currently not supported" << endl;

  Buffer light_buf = m_context->createBuffer(RT_BUFFER_INPUT, RT_FORMAT_INT3, triangles.size() - offset);
  int32_t* light_idxs = reinterpret_cast<int32_t*>(light_buf->map());
  for(unsigned int i = offset; i < triangles.size(); ++i)
  {
    unsigned int entry = (i - offset)*3;
    light_idxs[entry] = triangles[i].x;
    light_idxs[entry + 1] = triangles[i].y;
    light_idxs[entry + 2] = triangles[i].z;
  }
  light_buf->unmap();
  m_context["light_idxs"]->setBuffer(light_buf);

  return counter;
}

Matrix4x4 ObjScene::get_object_transform(string filename)
{
  size_t idx = filename.find_last_of("\\/") + 1;
  if(idx < filename.length())
  {
    if (filename.compare(idx, 7, "cornell") == 0)
      return Matrix4x4::scale(make_float3(0.025f))*Matrix4x4::rotate(M_PIf, make_float3(0.0f, 1.0f, 0.0f));
    else if (filename.compare(idx, 6, "dragon") == 0)
      return Matrix4x4::rotate(-M_PI_2f, make_float3(1.0, 0.0, 0.0));
    else if(filename.compare(idx, 5, "bunny") == 0)
      return optix::Matrix4x4::translate(make_float3(-3.0f, -0.85f, -8.0f))*optix::Matrix4x4::scale(make_float3(25.0f));
    else if(filename.compare(idx, 12, "justelephant") == 0)
      return Matrix4x4::translate(make_float3(-10.0f, 3.0f, -2.0f))*Matrix4x4::rotate(0.5f, make_float3(0.0f, 1.0f, 0.0f));
    else if(filename.compare(idx, 5, "plane") == 0)
      return Matrix4x4::translate(make_float3(0.0f, 0.33f, 0.0f));
  }
  return optix::Matrix4x4::identity();
}

void ObjScene::set_shader(int illum, Program closest_hit_program)
{
  if(illum < 0)
  {
    cout << "Negative identification numbers are not supported for illumination models." << endl;
    return;    
  }
  while(illum > static_cast<int>(shaders.size()))
    shaders.push_back(Program());
  shaders.push_back(closest_hit_program);
}

Program ObjScene::get_shader(int illum)
{
  if(illum > 0 && illum < static_cast<int>(shaders.size()))
    return shaders[illum];
  return Program();
}

void ObjScene::set_shadow_shader(int illum, Program any_hit_program)
{
  if(illum < 0)
  {
    cout << "Negative identification numbers are not supported for illumination models." << endl;
    return;    
  }
  while(illum > static_cast<int>(shadow_shaders.size()))
    shadow_shaders.push_back(Program());
  shadow_shaders.push_back(any_hit_program);
}

Program ObjScene::get_shadow_shader(int illum)
{
  if(illum > 0 && illum < static_cast<int>(shadow_shaders.size()))
    return shadow_shaders[illum];
  return Program();
}

bool ObjScene::export_raw(string& name)
{
  if(m_use_vbo_buffer)
  {
    cerr << "Export not implemented for VBO buffer mode." << endl;
    return false;
  }

  // export render data
  ofstream ofs_data(name + ".txt");
  if(ofs_data.bad())
    return false;
  ofs_data << frame << endl << WIDTH << " " << HEIGHT;
  ofs_data.close();

  Buffer out = m_context["output_buffer"]->getBuffer();
  int size_buffer = WIDTH*HEIGHT*4;
  float* mapped = new float[size_buffer];
  memcpy(mapped, out->map(), size_buffer*sizeof(float));
  out->unmap();
  ofstream ofs_image;
  ofs_image.open(name + ".raw", ios::binary);
  if(ofs_image.bad())
  {
    cerr << "Error in exporting file" << endl;
    return false;
  }

  int size_image = WIDTH*HEIGHT*3;
  float* converted = new float[size_image];
  float average = 0.0f;
  for(int i = 0; i < size_image/3; ++i)
  {
    for(int j = 0; j < 3; ++j)
    {
      converted[i*3 + j] = mapped[i*4 + j];
      average += mapped[i*4 + j];
    }
  }
  average /= size_image*3;
  delete[] mapped;
  ofs_image.write(reinterpret_cast<const char*>(converted), size_image*sizeof(float));
  ofs_image.close();
  delete[] converted;
  cout << "Exported buffer to " << name << ".raw (avg: " << average << ")" << endl;
  return true;
}

bool ObjScene::import_raw(string& name)
{
  if(m_use_vbo_buffer)
  {
    cerr << "Import not implemented for VBO buffer mode." << endl;
    return false;
  }

  // import render data
  unsigned int frame_number, old_width, old_height;
  ifstream ifs_data(name + ".txt");
  if(ifs_data.bad())
    return false;
  ifs_data >> frame_number >> old_width >> old_height;
  ifs_data.close();

  if(old_width != WIDTH || old_height != HEIGHT)
  {
    cout << "Resolution mismatch between current render resolution and that of the render result to be imported." << endl;
    return false;
  }

  // import image
  int size_image = WIDTH*HEIGHT*3;
  float* image = new float[size_image];
  ifstream ifs_image(name + ".raw", ifstream::binary);
  if(ifs_image.bad())
    return false;
  ifs_image.read(reinterpret_cast<char*>(image), size_image*sizeof(float));
  ifs_image.close();

  int size_buffer = WIDTH*HEIGHT*4;
  float* converted = new float[size_buffer];
  for(int i = 0; i < size_buffer/4; ++i)
  {
    for(int j = 0; j < 3; ++j)
      converted[i*4 + j] = image[i*3 + j];
    converted[i*4 + 3] = 1.0f;
  }
  delete[] image;

  Buffer out = m_context["output_buffer"]->getBuffer();
  memcpy(out->map(), converted, size_buffer*sizeof(float));
  frame = frame_number;
  out->unmap();
  out->markDirty();
  delete[] converted;
  cout << "Imported rendering from " << name << ".raw" << endl;
  return true;
}

void ObjScene::compute_jitters()
{
  Buffer jitter_buf = m_context->createBuffer(RT_BUFFER_INPUT, RT_FORMAT_FLOAT2, pixel_subdivs*pixel_subdivs);
  unsigned int t = tea<16>(19, 23);

  // Precompute jitter samples
  vector<float2> jitter;
  float step = 1.0f/static_cast<float>(pixel_subdivs);
  jitter.resize(pixel_subdivs*pixel_subdivs);
  for(unsigned int i = 0; i < pixel_subdivs; ++i)
    for(unsigned int j = 0; j < pixel_subdivs; ++j)
    {
      float2& jitter_sample = jitter[i*pixel_subdivs + j];
      jitter_sample.x = (rnd(t) + j)*step;
      jitter_sample.y = (rnd(t) + i)*step;
    }

  memcpy(jitter_buf->map(), &jitter[0], jitter.size()*sizeof(float2));
  jitter_buf->unmap();
  m_context["jitter"]->setBuffer(jitter_buf);
}
