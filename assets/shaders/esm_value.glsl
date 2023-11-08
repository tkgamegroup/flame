float depth = i_position.z / i_position.w;
o_exp_depth = exp(depth * lighting.esm_factor);
