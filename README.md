# Flame Game Engine
An ECS Game Engine Based On Reflection.

# Requirements:

- git-lfs

- VisualStudio

- VulkanSDK     - https://vulkan.lunarg.com/sdk/home#windows

- Graphviz      - (optional)
  
# Libraries:

- pugixml       - https://github.com/zeux/pugixml

- njson         - https://github.com/nlohmann/json

- STB           - https://github.com/nothings/stb
  
- freetype      - https://github.com/ubawurinna/freetype-windows-binaries

- msdfgen       - (must use my forked version, changed a little bit) https://github.com/tkgamegroup/msdfgen

- SPIRV-Cross   - https://github.com/KhronosGroup/SPIRV-Cross

- OpenAL        - http://www.openal.org/

# Build:

- cmake flame

- regsvr32 msdia**.dll in visual studio's dia sdk i.e. "vs_path/DIA SDK/bin/amd64"

- build typeinfogen first (in debug config)

- build other porjects
  
- enjoy

PS: if you want to build in release confin, always build in RelWithDebInfo, because DebugInfo is always needed

### UI Gallery:
![ui](https://github.com/tkgamegroup/flame/blob/master/screenshots/ui.png)
### Render Path Editing In Blueprint:
![editor](https://github.com/tkgamegroup/flame/blob/master/screenshots/editor.png)
