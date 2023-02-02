
#
#  Copyright (c) 2008 - 2009 NVIDIA Corporation.  All rights reserved.
#
#  NVIDIA Corporation and its licensors retain all intellectual property and proprietary
#  rights in and to this software, related documentation and any modifications thereto.
#  Any use, reproduction, disclosure or distribution of this software and related
#  documentation without an express license agreement from NVIDIA Corporation is strictly
#  prohibited.
#
#  TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, THIS SOFTWARE IS PROVIDED *AS IS*
#  AND NVIDIA AND ITS SUPPLIERS DISCLAIM ALL WARRANTIES, EITHER EXPRESS OR IMPLIED,
#  INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
#  PARTICULAR PURPOSE.  IN NO EVENT SHALL NVIDIA OR ITS SUPPLIERS BE LIABLE FOR ANY
#  SPECIAL, INCIDENTAL, INDIRECT, OR CONSEQUENTIAL DAMAGES WHATSOEVER (INCLUDING, WITHOUT
#  LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS, BUSINESS INTERRUPTION, LOSS OF
#  BUSINESS INFORMATION, OR ANY OTHER PECUNIARY LOSS) ARISING OUT OF THE USE OF OR
#  INABILITY TO USE THIS SOFTWARE, EVEN IF NVIDIA HAS BEEN ADVISED OF THE POSSIBILITY OF
#  SUCH DAMAGES
#

if(GLUT_FOUND AND OPENGL_FOUND)
  include_directories(${GLUT_INCLUDE_DIR})
  add_definitions(-DGLUT_FOUND -DGLUT_NO_LIB_PRAGMA)

  # See top level CMakeLists.txt file for documentation of OPTIX_add_sample_executable.
  OPTIX_add_sample_executable( render
    ${SAMPLES_CUDA_DIR}/helpers.h
    ${SAMPLES_CUDA_DIR}/random.h
    render.cpp
    GLUTDisplay.h
    GLUTDisplay.cpp
    SampleScene.h
    SampleScene.cpp
    Mouse.h
    Mouse.cpp
    DeviceMemoryLogger.h
    DeviceMemoryLogger.cpp
    stb_image_write.c
    stb_image_write.h
    ImageDisplay.h
    ImageDisplay.cpp
    ObjScene.h
    ObjScene.cpp
    Directional.h
    PointLight.h
    AreaLight.h
    structs.h
    constantbg.cu
    pinhole_camera.cu
    pt_pinhole_camera.cu
    normal_shader.cu
    directional_shader.cu
    shadow_shader.cu
    arealight_shader.cu
    mirror_shader.cu
    transparent_shader.cu
    tonemap_camera.cu
    sampler.h
    fresnel.h
    envmap.h
    env_cameras.cu
    envmap_background.cu
    envmap_shader.cu
    holdout_shader.cu
    )
else()
  # GLUT or OpenGL not found
  message("Disabling render, which requires glut and opengl.")
endif()



