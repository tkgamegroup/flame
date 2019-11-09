# Flame Game Engine
An ECS Game Engine Based On Reflection.

# thanks to:

- pugixml     - for parsing and loading XML (https://github.com/zeux/pugixml)

- nlohmannjson     - for parsing and loading JSON (https://github.com/nlohmann/json)

- STB         - for image loading/saving (https://github.com/nothings/stb)
  
- freetype    - for font loading/composition (https://github.com/ubawurinna/freetype-windows-binaries)

- msdfgen     - for generating sdf (https://github.com/Chlumsky/msdfgen)

- VulkanSDK   - graphics API (https://vulkan.lunarg.com/sdk/home#windows)

- SPIRV-Cross - for shader reflection (https://github.com/KhronosGroup/SPIRV-Cross)

- OpenAL      - for sound playing and recording (http://www.openal.org/)

- Graphviz    - for visualizing bp nodes
  

# build:

- regsvr32 msdia**.dll in visual studio's dia sdk

- download & install git-lfs

- download & build pugixml

- download STB

- download freetype

- download & build msdfgen (must use my forked version, changed a little bit: https://github.com/tkgamegroup/msdfgen)

- download & install VulkanSDK

- download & build SPIRV-Cross

- download OpenAL

- download & install Graphviz (optional)
  
- cmake flame
  
- enjoy

![ui](https://github.com/tkgamegroup/flame/blob/master/screenshots/ui.png)
![editor](https://github.com/tkgamegroup/flame/blob/master/screenshots/editor.png)
