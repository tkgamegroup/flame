<?xml version="1.0"?>
<pipeline>
  <shaders>
    <shader filename="fullscreen.vert" />
    <shader filename="mesh/deferred_shade.frag" />
  </shaders>
  <layout filename="mesh/deferred_shade.pll" />
  <renderpass filename="deferred.rp" index="1" />
</pipeline>
