#ifdef MAKE_DSL
#define DEFERRED_SET 0
#endif

layout (set = DEFERRED_SET, binding = 0) uniform sampler2D img_col_met;
layout (set = DEFERRED_SET, binding = 1) uniform sampler2D img_nor_rou;
layout (set = DEFERRED_SET, binding = 2) uniform sampler2D img_ao;
layout (set = DEFERRED_SET, binding = 3) uniform sampler2D img_dep;
