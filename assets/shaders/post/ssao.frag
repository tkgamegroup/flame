#include "../math.glsl"

layout (location = 0) in vec2 i_uv;

layout (location = 0) out float o_ao;

void main()
{
	float frag_dep = texture(img_dep, i_uv).x;
	vec4 p = scene.proj_inv * vec4(i_uv * 2.0 - 1.0, frag_dep, 1.0);
	p /= p.w;
    vec3 frag_pos = p.xyz;

	vec3 normal = texture(img_nor_rou, i_uv).xyz;
	normal = mat3(scene.view) * normalize(normal * 2.0 - 1.0);
    vec3 rand_vec = sample_noises[
        (int(gl_FragCoord.y) % NOISE_CY) * NOISE_CX
        + int(gl_FragCoord.x) % NOISE_CX].xyz;
    vec3 tangent = normalize(rand_vec - normal * dot(rand_vec, normal));
    vec3 bitangent = cross(tangent, normal);

    float ao = 0.0;
    for (int i = 0; i < SAMPLE_COUNT; i++)
    {
        vec3 pos = sample_locations[i].xyz;
        pos = pos.x * tangent + pos.y * normal + pos.z * bitangent;
        pos = frag_pos + pos * pc.radius;

        vec4 tap = scene.proj * vec4(pos, 1.0);
        tap /= tap.w;
        tap.xy = tap.xy * 0.5 + 0.5;

        float z = texture(img_dep, tap.xy).x;
        z = -linear_depth(scene.zNear, scene.zFar, z * 2.0 - 1.0);
        float range_check = smoothstep(0.0, 1.0, pc.radius / abs(frag_pos.z - z));
        ao += (z >= pos.z + pc.bias ? 1.0 : 0.0) * range_check;
    }
    ao = 1.0 - (ao / SAMPLE_COUNT);
    o_ao = ao;
}
