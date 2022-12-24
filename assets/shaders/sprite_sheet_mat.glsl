#ifdef COLOR_MAP
	uint sheet_cx = material.i[0];
	uint sheet_cy = material.i[1];
	uint n_sprites = sheet_cx * sheet_cy;
	uint idx = uint(mod(i_time * material.f[0], n_sprites));
	uint x = uint(idx % sheet_cx);
	uint y = idx / sheet_cx;
	vec4 color = sample_map(material.map_indices[COLOR_MAP], (vec2(x, y) + i_uv) / vec2(sheet_cx, sheet_cy));
	#ifdef ALPHA_MUL
		color = vec4(color.rgb * color.a, 1.0);
	#endif
	o_color = color * i_col;
#else
	o_color = i_col;
#endif
