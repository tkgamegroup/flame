<?xml version="1.0"?>
<pipeline>
  <shaders>
    <shader filename="element/element.vert" />
    <shader filename="element/element.frag" />
  </shaders>
  <layout filename="element/element.pll" />
  <renderpass filename="rgba8c.rp" index="0" />
  <vertex_buffers>
    <vertex_buffer>
      <vertex_attribute location="0" format="R32G32_SFLOAT" />
      <vertex_attribute location="1" format="R32G32_SFLOAT" />
      <vertex_attribute location="2" format="R8G8B8A8_UNORM" />
    </vertex_buffer>
  </vertex_buffers>
  <blend_options>
    <blend_option enable="true" src_color="SrcAlpha" dst_color="OneMinusSrcAlpha" 
                  src_alpha="One" dst_alpha="Zero" />
  </blend_options>
</pipeline>
