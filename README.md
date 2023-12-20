# Flame Game Engine
An ECS 3D-Game-Engine.

# With The Greate Help Of:

- VulkanSDK https://vulkan.lunarg.com/sdk/home#windows

- GLM https://github.com/g-truc/glm

- PugiXML https://github.com/zeux/pugixml

- NJson https://github.com/nlohmann/json

- cppcodec https://github.com/tplgy/cppcodec

- stb https://github.com/nothings/stb

- msdfgen https://github.com/Chlumsky/msdfgen

- SHA1 https://github.com/vog/sha1

- GLI https://github.com/g-truc/gli

- NVTT https://developer.nvidia.com/gpu-accelerated-texture-compression

- SPIRV-Cross https://github.com/KhronosGroup/SPIRV-Cross

- exprtk https://github.com/ArashPartow/exprtk

- Assimp https://github.com/assimp/assimp

- FBX SDK https://www.autodesk.com/developer-network/platform-technologies/fbx-sdk-2020-2-1

- PhysX https://github.com/NVIDIAGameWorks/PhysX

- recastnavigation https://github.com/recastnavigation/recastnavigation

- Font-Awesome https://github.com/FortAwesome/Font-Awesome

- ImGui https://github.com/ocornut/imgui

- ImGuizmo https://github.com/CedricGuillemet/ImGuizmo

- Imgui-node-editor https://github.com/thedmd/imgui-node-editor

- FortuneAlgorithm https://github.com/pvigier/FortuneAlgorithm

- OpenAL https://www.openal.org/

*Most of the requirements can be downloaded and built using the setup.py script*

# Build:

*Now we only support windows*
*You need python3, cmake and visualstudio*
*Make sure you check C++ ATL (x86 & x64) in c++ destop development in visualstudio's Tools and Features*
*Most of the libraries should use my forked versions*
*Libraries can be downloaded and built using the setup.py script*

- Run setup.py as administrator or db-click run_setup_as_admin.bat

- Cmake flame, configure extra libraries, such as FBX SDK, OpenAL

- Build it in visualstudio

*If you want to build the release version, build in RelWithDebInfo, because DebugInfo is always needed*
*Currently only 'editor' project is maintaining*