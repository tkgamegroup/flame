<?xml version="1.0"?>
<pipeline>
  <shaders>
    <shader filename="fullscreen.vert"/>
    <shader filename="deferred/deferred.frag" defines="NORMAL_DATA"/>
  </shaders>
  <layout filename="deferred/deferred.pll"/>
  <renderpass filename="rgba16.rp" index="0"/>
</pipeline>
