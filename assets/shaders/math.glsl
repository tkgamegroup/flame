const float PI = 3.14159265359;
const float PI_INV = 1.0 / PI;

float sum(vec3 v) 
{ 
	return v.x + v.y + v.z;
}

float length_squared(vec2 v)
{
	return v.x * v.x + v.y * v.y;
}

float length_squared(vec3 v)
{
	return v.x * v.x + v.y * v.y + v.z * v.z;
}

float saturate(float x)
{
	return clamp(x, 0.0, 1.0);
}

vec2 saturate(vec2 x)
{
	return clamp(x, vec2(0.0), vec2(1.0));
}

vec3 saturate(vec3 x)
{
	return clamp(x, vec3(0.0), vec3(1.0));
}

vec4 saturate(vec4 x)
{
	return clamp(x, vec4(0.0), vec4(1.0));
}

float map_01(float x, float v0, float v1)
{
  return (x - v0) / (v1 - v0);
}

vec2 map_01(vec2 x, float v0, float v1)
{
  return vec2(map_01(x.x, v0, v1), map_01(x.y, v0, v1));
}

vec3 map_01(vec3 x, float v0, float v1)
{
  return vec3(map_01(x.x, v0, v1), map_01(x.y, v0, v1), map_01(x.z, v0, v1));
}

vec4 map_01(vec4 x, float v0, float v1)
{
  return vec4(map_01(x.x, v0, v1), map_01(x.y, v0, v1), map_01(x.z, v0, v1), map_01(x.w, v0, v1));
}

float linstep(float low, float high, float v)
{
	return saturate((v - low) / (high - low));
}

float rand(vec2 co)
{
    return fract(sin(dot(co, vec2(12.9898, 78.233))) * 43758.5453);
}

float rand(vec3 co)
{
    return fract(sin(dot(co, vec3(12.9898, 78.233, 53.539))) * 43758.5453);
}

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

float sd_circle(vec2 p, float r)
{
	return length(p) - r;
}

float sd_ori_box(vec2 p, vec2 a, vec2 b, float th)
{
	float l = length(b - a);
	vec2 d = (b - a) / l;
	vec2 q = (p - (a + b) * 0.5);
	q = mat2(d.x, -d.y, d.y, d.x) * q;
	q = abs(q) - vec2(l, th) * 0.5;
	return length(max(q, 0.0)) + min(max(q.x, q.y), 0.0);
}

float sd_sphere(vec3 p, float s) 
{
    return length(p) - s;
}

float ud_box(vec3 p, vec3 b) 
{
    return length(max(abs(p) - b, 0.0));
}

float ud_round_box(vec3 p, vec3 b, float r) 
{
    return length(max(abs(p) - b, 0.0)) - r;
}

float op_smooth_union( float d1, float d2, float k )
{
    float h = max(k-abs(d1-d2),0.0);
    return min(d1, d2) - h*h*0.25/k;
}

float op_smooth_subtraction( float d1, float d2, float k )
{
    float h = max(k-abs(-d1-d2),0.0);
    return max(-d1, d2) + h*h*0.25/k;
}

float interpolate(float v, float a, float b, float t)
{
	float a0 = a - t * 0.5;
	float a1 = a + t * 0.5;
	float b0 = b - t * 0.5;
	float b1 = b + t * 0.5;
	if (v <= a0 || v >= b1)
		return 0.f;
	if (v >= a1 && v <= b0)
		return 1.f;
	if (v < a1)
	{
		if (a0 < 0)
			return 1.f;
		return 1.f - (a1 - v) / t;
	}
	else if (v > b0)
	{
		if (b1 > 1)
			return 1.f;
		return 1.f - (v - b0) / t;
	}
	return 0.f;
}

vec3 cube_coord(vec3 v)
{
	return vec3(v.x, v.y, v.z);
}

mat3 rotation(vec3 axis, float angle)
{
	float c = cos(angle);
	float s = sin(angle);

	vec3 temp = (1.0 - c) * axis;

	mat3 ret;
	ret[0][0] = c + temp[0] * axis[0];
	ret[0][1] = temp[0] * axis[1] + s * axis[2];
	ret[0][2] = temp[0] * axis[2] - s * axis[1];

	ret[1][0] = temp[1] * axis[0] - s * axis[2];
	ret[1][1] = c + temp[1] * axis[1];
	ret[1][2] = temp[1] * axis[2] + s * axis[0];

	ret[2][0] = temp[2] * axis[0] + s * axis[1];
	ret[2][1] = temp[2] * axis[1] - s * axis[0];
	ret[2][2] = c + temp[2] * axis[2];
	return ret;
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

float random(ivec2 st)
{
    return fract(sin(dot(st, vec2(12.9898, 78.233))) * 43758.5453123);
}

float noise(vec2 st)
{
    ivec2 i = ivec2(floor(st));
    vec2 f = fract(st);

    float a = random(i);
    float b = random(i + ivec2(1, 0));
    float c = random(i + ivec2(0, 1));
    float d = random(i + ivec2(1, 1));

    vec2 u = f * f * (3.0 - 2.0 * f);

    return mix(a, b, u.x) +
            (c - a)* u.y * (1.0 - u.x) +
            (d - b) * u.x * u.y;
}

#define PERLIN_FBM_OCTAVES 6
float perlin(vec2 st) 
{
    float value = 0.0;
    float amplitude = 0.5;

    for (int i = 0; i < PERLIN_FBM_OCTAVES; i++) 
    {
        value += amplitude * noise(st);
        st *= 2.0;
        amplitude *= 0.5;
    }
    return value;
}
