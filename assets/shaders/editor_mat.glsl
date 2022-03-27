#ifdef MAT_FILE
	MaterialInfo material = material_infos[i_matid];
	#include MAT_FILE
#else
	#ifndef DEPTH_PASS
		#ifndef DEFERRED
			#ifdef PICKUP
				o_color = pack_uint_to_v4(pc.i[0]);
			#elifdef NORMAL_DATA
				o_color = vec4(i_normal * 0.5 + vec3(0.5), 1.0);
			#elifdef CAMERA_LIGHT
				MaterialInfo material = material_infos[i_matid];
				#ifdef COLOR_MAP
					vec4 color = texture(material_maps[material.map_indices[COLOR_MAP]], i_uv);
				#else
					vec4 color = material.color;
				#endif
				o_color = vec4(vec3(dot(i_normal, -scene.camera_dir)) * color.rgb, 1.0);
			#else
				o_color = pc.f;
			#endif
		#else
			o_res_col_met = vec4(1.0, 1.0, 1.0, 0.0);
			o_res_nor_rou = vec4(i_normal * 0.5 + 0.5, 1.0);
		#endif
	#endif
#endif
