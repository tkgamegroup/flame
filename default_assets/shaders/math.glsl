const float PI = 3.14159265359;
const float PI_INV = 1.0 / PI;

vec2 panorama(vec3 v)
{
	return vec2(0.5 + 0.5 * atan(v.x, v.z) * PI_INV, acos(v.y) * PI_INV);
}

vec4 pack_uint_to_v4(uint id)
{
	vec4 ret;
	ret[0] = (id & 0xff) / 255.0;
	id >>= 8;
	ret[1] = (id & 0xff) / 255.0;
	id >>= 8;
	ret[2] = (id & 0xff) / 255.0;
	id >>= 8;
	ret[3] = (id & 0xff) / 255.0;
	return ret;
}

float linear_depth(float near, float far, float d /* -1, +1 */)
{
	return 2.0 * near * far / (far + near - d * (far - near));
}

vec3 cube_coord(vec3 v)
{
	return vec3(v.x, v.y, v.z);
}

vec3 rgb2xyz(vec3 _rgb)
{
	vec3 xyz;
	xyz.x = dot(vec3(0.4124564, 0.3575761, 0.1804375), _rgb);
	xyz.y = dot(vec3(0.2126729, 0.7151522, 0.0721750), _rgb);
	xyz.z = dot(vec3(0.0193339, 0.1191920, 0.9503041), _rgb);
	return xyz;
}

vec3 xyz2rgb(vec3 _xyz)
{
	vec3 rgb;
	rgb.x = dot(vec3( 3.2404542, -1.5371385, -0.4985314), _xyz);
	rgb.y = dot(vec3(-0.9692660,  1.8760108,  0.0415560), _xyz);
	rgb.z = dot(vec3( 0.0556434, -0.2040259,  1.0572252), _xyz);
	return rgb;
}

vec3 xyz2Yxy(vec3 _xyz)
{
	float inv = 1.0/dot(_xyz, vec3(1.0, 1.0, 1.0) );
	return vec3(_xyz.y, _xyz.x*inv, _xyz.y*inv);
}

vec3 Yxy2xyz(vec3 _Yxy)
{
	vec3 xyz;
	xyz.x = _Yxy.x*_Yxy.y/_Yxy.z;
	xyz.y = _Yxy.x;
	xyz.z = _Yxy.x*(1.0 - _Yxy.y - _Yxy.z)/_Yxy.z;
	return xyz;
}

vec3 rgb2Yxy(vec3 _rgb)
{
	return xyz2Yxy(rgb2xyz(_rgb) );
}

vec3 Yxy2rgb(vec3 _Yxy)
{
	return xyz2rgb(Yxy2xyz(_Yxy) );
}
