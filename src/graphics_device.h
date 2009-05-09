///////////////////////////////////////////////////////////////////////////////////
//
//	graphics_device.h - Graphics device interface
//
///////////////////////////////////////////////////////////////////////////////////
#ifndef __GRAPHICSDEVICE_H__
#define __GRAPHICSDEVICE_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <vectormath/cpp/vectormath_aos.h>
#include "opengl/opengl_device_defines.h"

// primitive type
enum GFXPrimitiveType
{
    GFX_PRIMITIVE_POINTLIST         = GFX_DEVICE_PRIMITIVE_POINTLIST,
    GFX_PRIMITIVE_LINES             = GFX_DEVICE_PRIMITIVE_LINES,
    GFX_PRIMITIVE_LINE_LOOP         = GFX_DEVICE_PRIMITIVE_LINE_LOOP,
    GFX_PRIMITIVE_LINE_STRIP        = GFX_DEVICE_PRIMITIVE_LINE_STRIP,
    GFX_PRIMITIVE_TRIANGLES         = GFX_DEVICE_PRIMITIVE_TRIANGLES,
    GFX_PRIMITIVE_TRIANGLE_STRIP    = GFX_DEVICE_PRIMITIVE_TRIANGLE_STRIP,
    GFX_PRIMITIVE_TRIANGLE_FAN      = GFX_DEVICE_PRIMITIVE_TRIANGLE_FAN
};

// buffer clear types
enum GFXBufferClear
{
    GFX_CLEAR_COLOUR_BUFFER         = GFX_DEVICE_CLEAR_COLOURUFFER,
    GFX_CLEAR_DEPTH_BUFFER          = GFX_DEVICE_CLEAR_DEPTHBUFFER,
    GFX_CLEAR_STENCIL_BUFFER        = GFX_DEVICE_CLEAR_STENCILBUFFER
};

// bool states
enum GFXRenderState
{
    GFX_DEPTH_TEST                  = GFX_DEVICE_STATE_DEPTH_TEST,
    GFX_ALPHA_TEST                  = GFX_DEVICE_STATE_ALPHA_TEST,
    GFX_BLEND                       = GFX_DEVICE_STATE_BLEND,
};

enum GFXMatrixMode
{
    GFX_MATRIX_TYPE_WORLD       = GFX_DEVICE_MATRIX_TYPE_WORLD,
    GFX_MATRIX_TYPE_VIEW        = GFX_DEVICE_MATRIX_TYPE_VIEW,
    GFX_MATRIX_TYPE_PROJECTION  = GFX_DEVICE_MATRIX_TYPE_PROJECTION
};

// Types
enum GFXType
{
    GFX_TYPE_BYTE               = GFX_DEVICE_TYPE_BYTE,
    GFX_TYPE_UNSIGNED_BYTE      = GFX_DEVICE_TYPE_UNSIGNED_BYTE,
    GFX_TYPE_SHORT              = GFX_DEVICE_TYPE_SHORT,
    GFX_TYPE_UNSIGNED_SHORT     = GFX_DEVICE_TYPE_UNSIGNED_SHORT,
    GFX_TYPE_INT                = GFX_DEVICE_TYPE_INT,
    GFX_TYPE_UNSIGNED_INT       = GFX_DEVICE_TYPE_UNSIGNED_INT,
    GFX_TYPE_FLOAT              = GFX_DEVICE_TYPE_FLOAT,
};

// Texture format
enum GFXTextureFormat
{
    GFX_TEXTURE_FORMAT_LUMINANCE  = 0,
    GFX_TEXTURE_FORMAT_RGB        = 1,
    GFX_TEXTURE_FORMAT_RGBA       = 2,
    GFX_TEXTURE_FORMAT_RGB_DXT1   = 3,
    GFX_TEXTURE_FORMAT_RGBA_DXT1  = 4,
    GFX_TEXTURE_FORMAT_RGBA_DXT3  = 5,
    GFX_TEXTURE_FORMAT_RGBA_DXT5  = 6
};

enum GFXBlendFactor
{
    GFX_BLEND_FACTOR_ZERO                     = GFX_DEVICE_BLEND_FACTOR_ZERO,
    GFX_BLEND_FACTOR_ONE                      = GFX_DEVICE_BLEND_FACTOR_ONE,
    GFX_BLEND_FACTOR_SRC_COLOR                = GFX_DEVICE_BLEND_FACTOR_SRC_COLOR,
    GFX_BLEND_FACTOR_ONE_MINUS_SRC_COLOR      = GFX_DEVICE_BLEND_FACTOR_ONE_MINUS_SRC_COLOR,
    GFX_BLEND_FACTOR_DST_COLOR                = GFX_DEVICE_BLEND_FACTOR_DST_COLOR,
    GFX_BLEND_FACTOR_ONE_MINUS_DST_COLOR      = GFX_DEVICE_BLEND_FACTOR_ONE_MINUS_DST_COLOR,
    GFX_BLEND_FACTOR_SRC_ALPHA                = GFX_DEVICE_BLEND_FACTOR_SRC_ALPHA,
    GFX_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA      = GFX_DEVICE_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
    GFX_BLEND_FACTOR_DST_ALPHA                = GFX_DEVICE_BLEND_FACTOR_DST_ALPHA,
    GFX_BLEND_FACTOR_ONE_MINUS_DST_ALPHA      = GFX_DEVICE_BLEND_FACTOR_ONE_MINUS_DST_ALPHA,
    GFX_BLEND_FACTOR_SRC_ALPHA_SATURATE       = GFX_DEVICE_BLEND_FACTOR_SRC_ALPHA_SATURATE,
    GFX_BLEND_FACTOR_CONSTANT_COLOR           = GFX_DEVICE_BLEND_FACTOR_CONSTANT_COLOR,
    GFX_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR = GFX_DEVICE_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR,
    GFX_BLEND_FACTOR_CONSTANT_ALPHA           = GFX_DEVICE_BLEND_FACTOR_CONSTANT_ALPHA,
    GFX_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA = GFX_DEVICE_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA,
};

// Parameter structure for CreateDevice
struct GFXSCreateDeviceParams
{
    uint32_t        m_DisplayWidth;
    uint32_t        m_DisplayHeight;
    const char*	    m_AppTitle;
    bool            m_Fullscreen;
    bool            m_PrintDeviceInfo;
};

GFXHContext GFXGetContext();

GFXHDevice GFXCreateDevice(int* argc, char** argv, GFXSCreateDeviceParams *params);
void GFXDestroyDevice();

// clear/draw functions
void GFXFlip();
void GFXClear(GFXHContext context, uint32_t flags, uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha, float depth, uint32_t stencil);
void GFXSetVertexStream(GFXHContext context, uint16_t stream, uint16_t size, GFXType type, uint16_t stride, const void* vertex_buffer);
void GFXDisableVertexStream(GFXHContext context, uint16_t stream);
void GFXDrawElements(GFXHContext context, GFXPrimitiveType prim_type, uint32_t count, GFXType type, const void* index_buffer);
void GFXDraw(GFXHContext context, GFXPrimitiveType prim_type, uint32_t first, uint32_t count);

HGFXVertexProgram GFXCreateVertexProgram(const void* program, uint32_t program_size);
HGFXFragmentProgram GFXCreateFragmentProgram(const void* program, uint32_t program_size);
void GFXDestroyVertexProgram(HGFXVertexProgram prog);
void GFXDestroyFragmentProgram(HGFXFragmentProgram prog);
void GFXSetVertexProgram(GFXHContext context, HGFXVertexProgram program);
void GFXSetFragmentProgram(GFXHContext context, HGFXFragmentProgram program);

void GFXSetFragmentConstant(GFXHContext context, const Vectormath::Aos::Vector4* data, int base_register);
void GFXSetVertexConstantBlock(GFXHContext context, const Vectormath::Aos::Vector4* data, int base_register, int num_vectors);
void GFXSetFragmentConstantBlock(GFXHContext context, const Vectormath::Aos::Vector4* data, int base_register, int num_vectors);


void GFXSetViewport(GFXHContext context, int width, int height);

void GFXEnableState(GFXHContext context, GFXRenderState state);
void GFXDisableState(GFXHContext context, GFXRenderState state);
void GFXSetBlendFunc(GFXHContext context, GFXBlendFactor source_factor, GFXBlendFactor destinaton_factor);

GFXHTexture GFXCreateTexture(uint32_t width, uint32_t height, GFXTextureFormat texture_format);

void GFXSetTextureData(GFXHTexture texture,
                       uint16_t mip_map,
                       uint16_t width, uint16_t height, uint16_t border,
                       GFXTextureFormat texture_format, const void* data, uint32_t data_size);

void GFXDestroyTexture(GFXHTexture t);
void GFXSetTexture(GFXHContext context, GFXHTexture t);

#endif	// __GRAPHICSDEVICE_H__


