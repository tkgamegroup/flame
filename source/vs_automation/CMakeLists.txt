add_library(vs_automation SHARED main.cs)
set_target_properties(vs_automation PROPERTIES FOLDER "flame")
set_target_properties(vs_automation PROPERTIES VS_DOTNET_REFERENCE_Microsoft.VisualStudio.Interop "${VS_LOCATION}\\Common7\\IDE\\PublicAssemblies\\Microsoft.VisualStudio.Interop.dll")
set_target_properties(vs_automation PROPERTIES VS_DOTNET_REFERENCE_System "System.dll")
