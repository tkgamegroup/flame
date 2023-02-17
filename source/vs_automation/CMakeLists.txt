add_executable(vs_automation main.cs)
set_target_properties(vs_automation PROPERTIES FOLDER "flame")
set_target_properties(vs_automation PROPERTIES VS_DOTNET_REFERENCES  "${VS_LOCATION}\\Common7\\IDE\\PublicAssemblies\\Microsoft.VisualStudio.Interop.dll;System")
