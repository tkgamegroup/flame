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
			#else
				o_color = pc.f;
			#endif
		#else
			o_res_col_met = vec4(1.0, 1.0, 1.0, 0.0);
			o_res_nor_rou = vec4(i_normal * 0.5 + 0.5, 1.0);
		#endif
	#endif
#endif
