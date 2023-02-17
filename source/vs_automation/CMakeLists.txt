add_library(vs_automation_csharp SHARED main.cs)
set_target_properties(vs_automation_csharp PROPERTIES FOLDER "flame")
set_target_properties(vs_automation_csharp PROPERTIES VS_DOTNET_REFERENCES  "${VS_LOCATION}\\Common7\\IDE\\PublicAssemblies\\Microsoft.VisualStudio.Interop.dll;System")

add_executable(vs_automation main.cpp)
set_target_properties(vs_automation PROPERTIES FOLDER "flame")
set_target_properties(vs_automation PROPERTIES COMMON_LANGUAGE_RUNTIME "")
set_target_properties(vs_automation PROPERTIES COMPILE_FLAGS "/permissive")
target_link_libraries(vs_automation vs_automation_csharp)
