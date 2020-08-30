# Flame Game Engine
An ECS Game Engine Based On Reflection.

# Requirements:

- VisualStudio

- VulkanSDK     - https://vulkan.lunarg.com/sdk/home#windows

- Graphviz      - (optional)
  
# Libraries:

- pugixml       - https://github.com/zeux/pugixml or https://gitee.com/tkgamegroup/pugixml

- njson         - https://github.com/nlohmann/json or https://gitee.com/tkgamegroup/json

- STB           - https://github.com/nothings/stb or https://gitee.com/tkgamegroup/stb

- OpenAL        - http://www.openal.org/

# Build:

- cmake flame

- install Vulkan SDK

- install OpenAL SDK

- run ./set_env.bat (you need to restart your IDE to take that effect)

- regsvr32 msdiaXXX.dll in visual studio's dia sdk i.e. "vs_path/DIA SDK/bin/amd64"

- build
  
- enjoy

** if you want to build in release config, always build in RelWithDebInfo, because DebugInfo is always needed
