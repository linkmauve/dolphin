// Copyright 2017 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

#pragma once

#include <string>
#include <utility>

#include "Common/CommonTypes.h"
#include "VideoCommon/ShaderGenCommon.h"
#include "VideoCommon/TextureDecoder.h"

enum class APIType;

#pragma pack(1)
struct convertion_shader_uid_data
{
  u32 NumValues() const { return sizeof(convertion_shader_uid_data); }
  u32 dst_format : 8;

  u32 efb_has_alpha : 1;
  u32 is_depth_copy : 1;
  u32 is_intensity : 1;
  u32 scale_by_half : 1;
  // u32 is_xfb_copy : 1;
  // u32 xfb_gamma : 2;
};
#pragma pack()

typedef ShaderUid<convertion_shader_uid_data> TextureConverterShaderUid;

ShaderCode GenerateTextureConverterShaderCode(APIType ApiType,
                                              const convertion_shader_uid_data* uid_data);

TextureConverterShaderUid GetTextureConverterShaderUid(u32 dst_format, bool is_depth_copy,
                                                       bool is_intensity, bool scale_by_half);
