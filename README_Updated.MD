# ECS 3D-Game-Engine

An ECS (Elastic Container Service) 3D-Game-Engine designed to empower game developers with efficient tools and resources.

## Build Instructions

### Prerequisites

- Python 3
- CMake
- Visual Studio

### Setup

1. Ensure you have Visual Studio installed.
2. In Visual Studio, navigate to the **Tools and Features** section.
3. Under the **C++ destop development** tab, ensure **C++ ATL (x86 & x64)** is checked.
4. Under the **Individual Components** tab, ensure **C++ ATL (x86 & x64) for latest v143** is checked.
5. Download and build the required libraries using the `setup.py` script. Run it as an administrator or double-click on `run_setup_as_admin.bat`.
6. CMake will configure extra libraries such as FBX SDK and OpenAL.
7. Build the project in Visual Studio. For the release version, build in **RelWithDebInfo** configuration, as **DebugInfo** is always needed.

### Note

- Currently, only the 'editor' project is actively maintained.

## Platform Support

Currently, we only support the Windows platform.

## Acknowledgements

This project would not have been possible without the invaluable contributions and support from the following libraries:

- [VulkanSDK](https://vulkan.lunarg.com/sdk/home#windows)
- [GLM](https://github.com/g-truc/glm)
- [PugiXML](https://github.com/zeux/pugixml)
- [NJson](https://github.com/nlohmann/json)
- [cppcodec](https://github.com/tplgy/cppcodec)
- [stb](https://github.com/nothings/stb)
- [msdfgen](https://github.com/Chlumsky/msdfgen)
- [SHA1](https://github.com/vog/sha1)
- [GLI](https://github.com/g-truc/gli)
- [NVTT](https://developer.nvidia.com/gpu-accelerated-texture-compression)
- [SPIRV-Cross](https://github.com/KhronosGroup/SPIRV-Cross)
- [exprtk](https://github.com/ArashPartow/exprtk)
- [Assimp](https://github.com/assimp/assimp)
- [FBX SDK](https://www.autodesk.com/developer-network/platform-technologies/fbx-sdk-2020-2-1)
- [PhysX](https://github.com/NVIDIAGameWorks/PhysX)
- [recastnavigation](https://github.com/recastnavigation/recastnavigation)
- [Font-Awesome](https://github.com/FortAwesome/Font-Awesome)
- [ImGui](https://github.com/ocornut/imgui)
- [ImGuizmo](https://github.com/CedricGuillemet/ImGuizmo)
- [Imgui-node-editor](https://github.com/thedmd/imgui-node-editor)
- [FortuneAlgorithm](https://github.com/pvigier/FortuneAlgorithm)
- [OpenAL](https://www.openal.org/)

This README serves as a guide to get you started with building and using the ECS 3D-Game-Engine. If you encounter any issues or have questions, feel free to reach out to us for assistance. Happy coding!
