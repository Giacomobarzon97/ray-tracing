// 02576 OptiX Rendering Framework
// Written by Jeppe Revall Frisvad, 2011
// Copyright (c) DTU Informatics 2011

#include <iostream>
#include <string>
#include <algorithm>
#include <cctype>
#include <sutil.h>
#include <sampleConfig.h>
#include <optix_world.h>
#include "GLUTDisplay.h"
#include "ObjScene.h"

using namespace std;
using namespace optix;

namespace
{
	void lower_case(char& x)
  { 
    x = tolower(x); 
  }

	inline void lower_case_string(std::string& s)
	{
    for_each(s.begin(), s.end(), lower_case);
	}
}

void printUsageAndExit( const string& argv0 ) 
{
  cerr << "Usage  : " << argv0 << " [options] any_object.obj [another.obj ...]" << endl
       << "options: --help           | -h            Print this usage message" << endl
       << "         --shader         | -sh <shader>  specify the closest hit program to be used for shading" << endl
       << "         --env <filename>                 specify the environment map to be loaded in panoramic format" << endl
       << endl;
  GLUTDisplay::printUsage();

  exit(0);
}

int main( int argc, char** argv ) 
{
  vector<string> filenames;
  string filename = "";
  string shadername = "";
  string envname = "";

  for ( int i = 1; i < argc; ++i ) 
  {
    string arg( argv[i] );
    if( arg == "-h" || arg == "--help" ) 
    {
      printUsageAndExit( argv[0] ); 
    }
    else if( arg == "-sh" || arg == "--shader" )
    {
      if( i == argc-1 )
        printUsageAndExit( argv[0] );
      shadername = argv[++i];
      lower_case_string(shadername);
    }
    else if(arg == "-env")
    {
      if(i == argc-1)
        printUsageAndExit(argv[0]);
      
      envname = argv[++i];
      string file_extension;
      size_t idx = envname.find_last_of('.');
      if(idx < envname.length())
      {
        file_extension = envname.substr(idx, envname.length() - idx);
        lower_case_string(file_extension);
      }

      if(file_extension == ".hdr" || file_extension == ".ppm")
        lower_case_string(envname);
      else
      {
        cerr << "Please use environment maps in .hdr or .ppm format. Received: '" << envname << "'" << endl;
        printUsageAndExit(argv[0]);
      }
    }
    else 
    {
      filename = argv[i];
      string file_extension;
      size_t idx = filename.find_last_of('.');
      if(idx < filename.length())
      {
        file_extension = filename.substr(idx, filename.length() - idx);
        lower_case_string(file_extension);
      }

      if(file_extension == ".obj")
      {
        filenames.push_back(filename);
        lower_case_string(filenames.back());
      }
      else
      {
        cerr << "Unknown option or not an obj file: '" << arg << "'" << endl;
        printUsageAndExit( argv[0] );
      }
    }
  }
  if ( filenames.size() == 0 ) 
    filenames.push_back(string(SAMPLES_DIR) + "/models/" + "cow_vn.obj");

  ObjScene scene(filenames, shadername, envname);
  GLUTDisplay::init(argc, argv);
  try
  {
    GLUTDisplay::run( "ObjScene", &scene );
    return 0;
  }
  SUTIL_CATCH(scene.getContext()->get())
  return 0;
}
