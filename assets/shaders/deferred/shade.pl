<?xml version="1.0"?>
<pipeline>
  <shaders>
    <shader filename="fullscreen.vert" />
    <shader filename="deferred/shade.frag" />
  </shaders>
  <layout filename="deferred/shade.pll" />
  <renderpass filename="deferred.rp" index="1" />
</pipeline>
