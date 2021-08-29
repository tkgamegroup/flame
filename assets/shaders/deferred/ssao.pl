<?xml version="1.0"?>
<pipeline>
  <shaders>
    <shader filename="fullscreen.vert"/>
    <shader filename="deferred/ssao.frag"/>
  </shaders>
  <layout filename="deferred/ssao.pll"/>
  <renderpass filename="r8.rp" index="0"/>
</pipeline>
