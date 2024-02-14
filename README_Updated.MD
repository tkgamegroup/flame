# ECS 3D-Game-Engine

An ECS (Elastic Container Service) 3D-Game-Engine designed to empower game developers with efficient tools and resources.

## Features
- **Audio**: Functionalities for loading, managing, and playing audio files in various formats (WAV, OGG) using the OpenAL library. Components for managing audio devices, listeners, and sources within the 3D space of the game environment.
- **Blueprint System (bp)**: Define and manage entities and behaviors within the game.
- **Foundation**: Core functionalities and utilities essential for the engine's operation.
- **Graphics**: Rendering functionalities including managing textures and shaders.
- **Effects (fx)**: Rendering effects such as particle systems, shaders, post-processing effects, etc.
- **Typeinfogen**: Generate type information, for serialization or reflection.
- **Universe**: Manage game world, scene composition, and game state.
- **VS Automation**: Tools for Visual Studio for project setup and build automation.
- **ImGui**: Immediate Mode Graphical User Interface for debugging and UI development.
- **Serialization**: Save and load game state or resources using JSON, XML, or Base64.
- **Math Utilities**: Essential for calculations within the game engine.

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

      ### Dependencies Installed by `setup.py`
     >- **glm**: A header-only library for mathematics operations.
     >- **PugiXML**: A lightweight XML processing library.
     >- **nlohmann json**: A modern JSON library for C++.
     >- **cppcodec**: A header-only C++ library for base64 and hex conversions.
     >- **stb**: A collection of single-file public domain libraries for C/C++.
     >- **msdfgen**: A library for generating Multi-channel Signed Distance Fields.
     >- **sha1**: A simple SHA-1 hash function implementation in C++.
     >- **gli**: A header-only library for handling textures.
     >- **SPIRV-Cross**: A tool for converting SPIR-V to other shader languages.
     >- **exprtk**: A C++ library for parsing and evaluating mathematical expressions.
     >- **assimp**: A library to import various 3D model formats.
     >- **recastnavigation**: A navigation mesh construction toolset for games.
     >- **Font-Awesome**: A font and icon toolkit.
     >- **imgui**: A GUI library for immediate-mode graphical user interfaces.
     >- **ImGuizmo**: A 3D gizmo widget for ImGui.
     >- **ImGui-Node-Editor**: A node editor using ImGui.
> **Warning:** Other dependencies may be required that are not built into setup.py.

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
