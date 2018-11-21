#if defined(UBO_CONSTANT_OLD)
layout(set = UBO_CONSTANT_OLD_SET, binding = UBO_CONSTANT_OLD_BINDING) uniform U_comstant
{
	float near;
	float far;
	float cx;
	float cy;
	float aspect;
	float fovy;
	float tanHfFovy;
	float envrCx;
	float envrCy;
}u_constant;
#endif

#if defined(UBO_MATRIX_OLD)
layout(set = UBO_MATRIX_OLD_SET, binding = UBO_MATRIX_OLD_BINDING) uniform U_matrix
{
	mat4 proj;
	mat4 projInv;
	mat4 view;
	mat4 viewInv;
	mat4 projView;
	mat4 projViewRotate;
	vec4 frustumPlanes[6];
	vec2 viewportDim;
}u_matrix;
#endif

#if defined(UBO_MATRIX)
layout(set = UBO_MATRIX_SET, binding = UBO_MATRIX_BINDING) uniform U_matrix
{
	mat4 proj;
	mat4 view;
	vec4 camera_pos;
}u_matrix;
#endif

#if defined(UBO_AMBIENT_OLD)
layout(set = UBO_AMBIENT_OLD_SET, binding = UBO_AMBIENT_OLD_BINDING) uniform U_ambient
{
	vec3 color;
	uint envr_max_mipmap;
	vec4 fog_color;
}u_ambient;
#endif

#if defined(UBO_MATERIAL_OLD)
struct Material
{
	uint albedoAlphaCompress;
	uint specRoughnessCompress;

	uint mapIndex;
	
	uint dummy;
};

layout(set = UBO_MATERIAL_OLD_SET, binding = UBO_MATERIAL_OLD_BINDING) uniform U_material
{
	Material material[256];
}u_material;
#endif

#if defined(IMG_MATERIAL_OLD)
layout(set = IMG_MATERIAL_OLD_SET, binding = IMG_MATERIAL_OLD_BINGIND) uniform sampler2D i_material[256];
#endif

#if defined(UBO_LIGHT_OLD)
struct Light
{
	vec4 coord;
	vec4 color;
	vec4 spotData;
};

layout(set = UBO_AMBIENT_OLD_SET, binding = UBO_AMBIENT_OLD_BINDING) uniform U_light
{
	uint count;
	Light lights[256];
}u_light;
#endif

#if defined(UBO_SHADOW_OLD)
layout(set = UBO_SHADOW_OLD_SET, binding = UBO_SHADOW_OLD_BINDING) uniform U_shadow
{
	mat4 matrix[8];
}u_shadow;
#endif

#if defined(IMG_SHADOW_OLD)
layoutset = IMG_SHADOW_OLD_SET, binding = IMG_SHADOW_OLD_BINGIND) uniform sampler2DArray i_shadow;
#endif
