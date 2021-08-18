<?xml version="1.0"?>
<pipeline>
  <shaders>
    <shader filename="fullscreen.vert" />
    <shader filename="post/blit.frag" />
  </shaders>
  <layout filename="post/post.pll" />
  <renderpass filename="rgba16ms4.rp" index="0" />
  <sample_count v="4" />
</pipeline>
