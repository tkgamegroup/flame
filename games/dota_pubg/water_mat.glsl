vec2 dir1 = normalize(vec2(3, 1));
vec2 dir2 = normalize(vec2(2, -1));
vec2 uv0 = i_uv * 8.0 + dir1 * 0.002 * render_data.time;
vec2 uv1 = i_uv * 8.0 + dir2 * 0.001 * render_data.time;
vec3 nor = texture(maps[material.map_indices[0]], uv0).rgb * 0.5 + texture(maps[material.map_indices[0]], uv1).rgb * 0.5;
nor = nor * 2.0 - vec3(1.0);
vec3 N = vec3(nor.x, nor.z, nor.y);
o_color = shading(i_coordw, N, 0.0, vec3(0.1, 0.1, 0.2), vec3(0.30), 0.05);
