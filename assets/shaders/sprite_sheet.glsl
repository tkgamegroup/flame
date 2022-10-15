#ifdef COLOR_MAP
	o_color = sample_map(material.map_indices[COLOR_MAP], i_uv) * i_col;
#else
	o_color = i_col;
#endif
