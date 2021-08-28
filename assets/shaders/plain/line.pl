<?xml version="1.0"?>
<pipeline>
  <shaders>
    <shader filename="plain/plain.vert" />
    <shader filename="plain/plain.frag" />
  </shaders>
  <layout filename="plain/plain.pll" />
  <renderpass filename="bgra8l.rp" index="0" />
  <vertex_buffers>
    <vertex_buffer>
      <vertex_attribute location="0" format="R32G32B32_SFLOAT" />
      <vertex_attribute location="1" format="R8G8B8A8_UNORM" />
    </vertex_buffer>
  </vertex_buffers>
  <primitive_topology v="LineList" />
  <cull_mode v="None" />
</pipeline>
