// Copyright 2017 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

#include <cstring>

#include "Common/Assert.h"
#include "Common/CommonTypes.h"
#include "VideoCommon/BPMemory.h"
#include "VideoCommon/TextureConverterShaderGen.h"
#include "VideoCommon/VideoCommon.h"
#include "VideoCommon/VideoConfig.h"

TextureConverterShaderUid GetTextureConverterShaderUid(u32 dst_format, bool is_depth_copy,
                                                       bool is_intensity, bool scale_by_half)
{
  TextureConverterShaderUid out;
  convertion_shader_uid_data* uid_data = out.GetUidData<convertion_shader_uid_data>();
  memset(uid_data, 0, sizeof(*uid_data));

  uid_data->efb_has_alpha = bpmem.zcontrol.pixel_format == PEControl::RGBA6_Z24;
  uid_data->dst_format = dst_format;
  uid_data->is_depth_copy = is_depth_copy;
  uid_data->is_intensity = is_intensity;
  uid_data->scale_by_half = scale_by_half;

  return out;
}

ShaderCode GenerateTextureConverterShaderCode(APIType ApiType,
                                              const convertion_shader_uid_data* uid_data)
{
  ShaderCode out;

  float colmat[28] = {0};
  float* const fConstAdd = colmat + 16;
  float* const ColorMask = colmat + 20;
  ColorMask[0] = ColorMask[1] = ColorMask[2] = ColorMask[3] = 255.0f;
  ColorMask[4] = ColorMask[5] = ColorMask[6] = ColorMask[7] = 1.0f / 255.0f;

  if (uid_data->is_depth_copy)
  {
    switch (uid_data->dst_format)
    {
    case 0:  // Z4
      colmat[3] = colmat[7] = colmat[11] = colmat[15] = 1.0f;
      break;
    case 8:  // Z8H
    case 1:  // Z8
      colmat[0] = colmat[4] = colmat[8] = colmat[12] = 1.0f;
      break;

    case 3:  // Z16
      colmat[1] = colmat[5] = colmat[9] = colmat[12] = 1.0f;
      break;

    case 11:  // Z16 (reverse order)
      colmat[0] = colmat[4] = colmat[8] = colmat[13] = 1.0f;
      break;

    case 6:  // Z24X8
      colmat[0] = colmat[5] = colmat[10] = 1.0f;
      break;

    case 9:  // Z8M
      colmat[1] = colmat[5] = colmat[9] = colmat[13] = 1.0f;
      break;

    case 10:  // Z8L
      colmat[2] = colmat[6] = colmat[10] = colmat[14] = 1.0f;
      break;

    case 12:  // Z16L - copy lower 16 depth bits
      colmat[1] = colmat[5] = colmat[9] = colmat[14] = 1.0f;
      break;

    default:
      ERROR_LOG(VIDEO, "Unknown copy zbuf format: 0x%x", uid_data->dst_format);
      colmat[2] = colmat[5] = colmat[8] = 1.0f;
      break;
    }
  }
  else if (uid_data->is_intensity)
  {
    fConstAdd[0] = fConstAdd[1] = fConstAdd[2] = 16.0f / 255.0f;
    switch (uid_data->dst_format)
    {
    case 0:  // I4
    case 1:  // I8
    case 2:  // IA4
    case 3:  // IA8
    case 8:  // I8
      // TODO - verify these coefficients
      colmat[0] = 0.257f;
      colmat[1] = 0.504f;
      colmat[2] = 0.098f;
      colmat[4] = 0.257f;
      colmat[5] = 0.504f;
      colmat[6] = 0.098f;
      colmat[8] = 0.257f;
      colmat[9] = 0.504f;
      colmat[10] = 0.098f;

      if (uid_data->dst_format < 2 || uid_data->dst_format == 8)
      {
        colmat[12] = 0.257f;
        colmat[13] = 0.504f;
        colmat[14] = 0.098f;
        fConstAdd[3] = 16.0f / 255.0f;
        if (uid_data->dst_format == 0)
        {
          ColorMask[0] = ColorMask[1] = ColorMask[2] = 255.0f / 16.0f;
          ColorMask[4] = ColorMask[5] = ColorMask[6] = 1.0f / 15.0f;
        }
      }
      else  // alpha
      {
        colmat[15] = 1;
        if (uid_data->dst_format == 2)
        {
          ColorMask[0] = ColorMask[1] = ColorMask[2] = ColorMask[3] = 255.0f / 16.0f;
          ColorMask[4] = ColorMask[5] = ColorMask[6] = ColorMask[7] = 1.0f / 15.0f;
        }
      }
      break;

    default:
      ERROR_LOG(VIDEO, "Unknown copy intensity format: 0x%x", uid_data->dst_format);
      colmat[0] = colmat[5] = colmat[10] = colmat[15] = 1.0f;
      break;
    }
  }
  else
  {
    switch (uid_data->dst_format)
    {
    case 0:  // R4
      colmat[0] = colmat[4] = colmat[8] = colmat[12] = 1;
      ColorMask[0] = 255.0f / 16.0f;
      ColorMask[4] = 1.0f / 15.0f;
      break;
    case 1:  // R8
    case 8:  // R8
      colmat[0] = colmat[4] = colmat[8] = colmat[12] = 1;
      break;

    case 2:  // RA4
      colmat[0] = colmat[4] = colmat[8] = colmat[15] = 1.0f;
      ColorMask[0] = ColorMask[3] = 255.0f / 16.0f;
      ColorMask[4] = ColorMask[7] = 1.0f / 15.0f;

      if (!uid_data->efb_has_alpha)
      {
        ColorMask[3] = 0.0f;
        fConstAdd[3] = 1.0f;
      }
      break;
    case 3:  // RA8
      colmat[0] = colmat[4] = colmat[8] = colmat[15] = 1.0f;

      if (!uid_data->efb_has_alpha)
      {
        ColorMask[3] = 0.0f;
        fConstAdd[3] = 1.0f;
      }
      break;

    case 7:  // A8
      colmat[3] = colmat[7] = colmat[11] = colmat[15] = 1.0f;

      if (!uid_data->efb_has_alpha)
      {
        ColorMask[3] = 0.0f;
        fConstAdd[0] = 1.0f;
        fConstAdd[1] = 1.0f;
        fConstAdd[2] = 1.0f;
        fConstAdd[3] = 1.0f;
      }
      break;

    case 9:  // G8
      colmat[1] = colmat[5] = colmat[9] = colmat[13] = 1.0f;
      break;
    case 10:  // B8
      colmat[2] = colmat[6] = colmat[10] = colmat[14] = 1.0f;
      break;

    case 11:  // RG8
      colmat[0] = colmat[4] = colmat[8] = colmat[13] = 1.0f;
      break;

    case 12:  // GB8
      colmat[1] = colmat[5] = colmat[9] = colmat[14] = 1.0f;
      break;

    case 4:  // RGB565
      colmat[0] = colmat[5] = colmat[10] = 1.0f;
      ColorMask[0] = ColorMask[2] = 255.0f / 8.0f;
      ColorMask[4] = ColorMask[6] = 1.0f / 31.0f;
      ColorMask[1] = 255.0f / 4.0f;
      ColorMask[5] = 1.0f / 63.0f;
      fConstAdd[3] = 1.0f;  // set alpha to 1
      break;

    case 5:  // RGB5A3
      colmat[0] = colmat[5] = colmat[10] = colmat[15] = 1.0f;
      ColorMask[0] = ColorMask[1] = ColorMask[2] = 255.0f / 8.0f;
      ColorMask[4] = ColorMask[5] = ColorMask[6] = 1.0f / 31.0f;
      ColorMask[3] = 255.0f / 32.0f;
      ColorMask[7] = 1.0f / 7.0f;

      if (!uid_data->efb_has_alpha)
      {
        ColorMask[3] = 0.0f;
        fConstAdd[3] = 1.0f;
      }
      break;
    case 6:  // RGBA8
      colmat[0] = colmat[5] = colmat[10] = colmat[15] = 1.0f;

      if (!uid_data->efb_has_alpha)
      {
        ColorMask[3] = 0.0f;
        fConstAdd[3] = 1.0f;
      }
      break;

    default:
      ERROR_LOG(VIDEO, "Unknown copy color format: 0x%x", uid_data->dst_format);
      colmat[0] = colmat[5] = colmat[10] = colmat[15] = 1.0f;
      break;
    }
  }

  out.Write("SAMPLER_BINDING(9) uniform sampler2DArray samp9;\n"
            "in vec3 f_uv0;\n"
            "out vec4 ocol0;\n"
            "const vec4 colmat[7] = {\n");

  for (int i = 0; i < 7; i++)
  {
    out.Write("	vec4(%f, %f, %f, %f)%s\n", colmat[i * 4 + 0], colmat[i * 4 + 1], colmat[i * 4 + 2],
              colmat[i * 4 + 3], i < 7 ? "," : "");
  }
  out.Write("};\n");

  if (uid_data->is_depth_copy)
  {
    out.Write("void main(){\n"
              "	vec4 texcol = texture(samp9, vec3(f_uv0.xy, %s));\n"
              "	int depth = int(texcol.x * 16777216.0);\n"

              // Convert to Z24 format
              "	ivec4 workspace;\n"
              "	workspace.r = (depth >> 16) & 255;\n"
              "	workspace.g = (depth >> 8) & 255;\n"
              "	workspace.b = depth & 255;\n"

              // Convert to Z4 format
              "	workspace.a = (depth >> 16) & 0xF0;\n"

              // Normalize components to [0.0..1.0]
              "	texcol = vec4(workspace) / 255.0;\n"

              "	ocol0 = texcol * mat4(colmat[0], colmat[1], colmat[2], colmat[3]) + colmat[4];\n"
              "}\n",
              g_ActiveConfig.bStereoEFBMonoDepth ? "0.0" : "f_uv0.z");
  }
  else
  {
    out.Write("void main(){\n"
              "	vec4 texcol = texture(samp9, f_uv0);\n"
              "	texcol = floor(texcol * colmat[5]) * colmat[6];\n"
              "	ocol0 = texcol * mat4(colmat[0], colmat[1], colmat[2], colmat[3]) + colmat[4];\n"
              "}\n");
  }

  return out;
}
