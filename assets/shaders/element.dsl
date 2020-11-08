#ifdef MAKE_DSL
#define ELEMENT_SET 0
#endif

layout (set = ELEMENT_SET, binding = 0) uniform sampler2D images[64];
