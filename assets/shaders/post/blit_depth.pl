<?xml version="1.0"?>
<pipeline>
  <shaders>
    <shader filename="fullscreen.vert" />
    <shader filename="post/blit.frag" defines="DEPTH" />
  </shaders>
  <layout filename="post/post.pll" />
  <compare_op v="Always" />
  <renderpass filename="d16.rp" index="0" />
</pipeline>
