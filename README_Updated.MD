# ECS 3D-Game-Engine

An ECS 3D-Game-Engine designed to empower game developers with efficient tools and resources.

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

- VulkanSDK
- GLM
- PugiXML
- NJson
- cppcodec
- stb
- msdfgen
- SHA1
- GLI
- NVTT
- SPIRV-Cross
- exprtk
- Assimp
- FBX SDK
- PhysX
- recastnavigation
- Font-Awesome
- ImGui
- ImGuizmo
- Imgui-node-editor
- FortuneAlgorithm
- OpenAL

This README serves as a guide to get you started with building and using the ECS 3D-Game-Engine. If you encounter any issues or have questions, feel free to reach out to us for assistance. Happy coding!
