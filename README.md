# Flame Game Engine
An ECS 3D-Game-Engine.

# Build

*Now we only support windows*

*You need Python3, CMake, Vcpkg, and Visual Studio*

*Make sure you check C++ ATL (x86 & x64) in C++ destop development tab and check C++ ATL (x86 & x64) for lastest v143 in individual components tab in Visual Studio's Tools and Features*

- Run setup.py as administrator or db-click run_setup_as_admin.bat

- CMake Flame, deal with some extra libraries, such as FBX SDK, OpenAL

- Build it in Visual Studio

*If you want to build the release version, build in RelWithDebInfo, because DebugInfo is always needed*

*Currently only 'editor' project is maintaining*