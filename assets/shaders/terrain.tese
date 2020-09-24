#version 450 core
#extension GL_ARB_shading_language_420pack : enable
#extension GL_ARB_separate_shader_objects : enable

#define MATERIAL_SET 0
#define LIGHT_SET 1
#define RENDER_DATA_SET 2
#define TERRAIN_SET 3

#include "material_dsl.glsl"
#include "render_data_dsl.glsl"
#include "terrain_dsl.glsl"

layout(quads, equal_spacing, ccw) in;

layout (location = 0) in vec2 i_uvs[];
 
layout (location = 0) out vec2 o_uv;

void main()
{
	o_uv = 
		mix(
			mix(i_uvs[0], i_uvs[1], gl_TessCoord.x), 
			mix(i_uvs[3], i_uvs[2], gl_TessCoord.x), 
			gl_TessCoord.y
		);

	vec4 pos = 
		mix(
			mix(gl_in[0].gl_Position, gl_in[1].gl_Position, gl_TessCoord.x), 
			mix(gl_in[3].gl_Position, gl_in[2].gl_Position, gl_TessCoord.x), 
			gl_TessCoord.y
		);
		
	MaterialInfo material = material_infos[99];
	pos.y -= texture(maps[material.normal_height_map_index], o_uv).a * terrain_info.extent.y;

	gl_Position = render_data.proj_view * pos;
}
