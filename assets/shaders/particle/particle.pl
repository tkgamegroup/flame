<?xml version="1.0"?>
<pipeline>
  <shaders>
    <shader filename="particle/particle.vert" />
    <shader filename="particle/particle.geom" />
    <shader filename="particle/particle.frag" />
  </shaders>
  <layout filename="particle/particle.pll" />
  <renderpass filename="forward_ms4.rp" index="0" />
  <vertex_buffers>
    <vertex_buffer>
      <vertex_attribute location="0" format="R32G32B32_SFLOAT" />
      <vertex_attribute location="1" format="R32G32B32_SFLOAT" />
      <vertex_attribute location="2" format="R32G32B32_SFLOAT" />
      <vertex_attribute location="3" format="R32G32B32A32_SFLOAT" />
      <vertex_attribute location="4" format="R32G32B32A32_SFLOAT" />
    </vertex_buffer>
  </vertex_buffers>
  <cull_mode v="None" />
  <primitive_topology v="PointList" />
  <sample_count v="4" />
  <depth_test v="true" />
  <depth_write v="false" />
  <blend_options>
    <blend_option enable="true" src_color="SrcAlpha" dst_color="OneMinusSrcAlpha" 
                  src_alpha="One" dst_alpha="Zero" />
  </blend_options>
</pipeline>
