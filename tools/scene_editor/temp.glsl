#version 450 core
#extension GL_ARB_shading_language_420pack : enable
#extension GL_ARB_separate_shader_objects : enable

#define DEFERRED
#define MAT_FILE "D:/flame/assets/shaders/std_mat.glsl"

#define SET 0
#include "../math.glsl"


#include "D:\flame\assets\shaders\scene.dsl"
#undef SET
#define SET 1
#include "D:\flame\assets\shaders\instance.dsl"
#undef SET
#define SET 2
#include "D:\flame\assets\shaders\material.dsl"
#undef SET
#define SET 3



layout(location = 0) in flat uint i_id;
layout(location = 1) in flat uint i_matid;
layout(location = 2) in		 vec2 i_uv;
layout(location = 3) in		 vec3 i_normal;
layout(location = 4) in		 vec3 i_tangent;

layout(location = 0) out vec4 o_res_col_met;
layout(location = 1) out vec4 o_res_nor_rou;

void main()
{
#include "../editor_mat.glsl"
}

