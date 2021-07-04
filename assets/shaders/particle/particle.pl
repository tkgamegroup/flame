<?xml version="1.0"?>
<pipeline>
  <shaders>
    <shader filename="particle/particle.vert" />
    <shader filename="particle/particle.frag" />
  </shaders>
  <layout filename="particle/particle.pll" />
  <renderpass filename="particle/particle.rp" index="0" />
  <vertex_buffers>
    <vertex_buffer>
      <vertex_attribute location="0" format="R32G32B32_SFLOAT" />
      <vertex_attribute location="1" format="R32G32_SFLOAT" />
      <vertex_attribute location="2" format="R8G8B8A8_UNORM" />
    </vertex_buffer>
  </vertex_buffers>
  <cull_mode v="None" />
  <depth_test v="true" />
  <depth_write v="false" />
  <blend_options>
    <blend_option enable="true" src_color="SrcAlpha" dst_color="OneMinusSrcAlpha" 
                  src_alpha="One" dst_alpha="Zero" />
  </blend_options>
</pipeline>
