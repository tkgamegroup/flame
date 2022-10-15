#ifdef COLOR_MAP
	uint sheet_cx = material.i[0];
	uint sheet_cy = material.i[1];
	uint n_sprites = sheet_cx * sheet_cy;
	uint idx = uint(mod(i_time * material.f[0], n_sprites));
	uint x = uint(mod(idx, sheet_cx));
	uint y = idx / sheet_cx;
	o_color = sample_map(material.map_indices[COLOR_MAP], (vec2(x, y) + i_uv) / vec2(sheet_cx, sheet_cy)) * i_col;
#else
	o_color = i_col;
#endif
