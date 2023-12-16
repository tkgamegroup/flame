import pathlib
import os
import glob

current_directory = pathlib.Path.cwd()
thirdparty_directory = current_directory / "thirdparty"
print("Current Directory: " + str(current_directory))

os.system("chcp 65001")

os.system("vswhere.exe -latest -property installationPath > temp.txt")
f = open("temp.txt")
vs_path = pathlib.Path(f.read().strip("\n"))
f.close()
if not vs_path.exists():
	print("Cannot find visual studio, abort")
	exit(0)

if os.system("cmake --version") != 0:
	print("Cannot find CMake, abort")
	exit(0)

print("Download or build libraries?\n 1 - All\n 2 - Manually\n 3 - All Required\n 4 - Skip")
op = int(input())

if op != 4:
	print("== library glm ==")
	ok = True
	address = "https://github.com/tkgamegroup/glm.git"
	lib_dir = thirdparty_directory / "glm"
	if not lib_dir.exists():
		if op == 2:
			print("Download glm from %s into %s ? y/n" % (address, str(lib_dir)))
			ok = input() == "y"
		if ok:
			os.system("git clone --depth 1 %s %s && echo ok" % (address, str(lib_dir)))
	else:
		print("%s exists, skip download" % str(lib_dir))
	print("====\n")
			
	print("== library PugiXML ==")
	ok = True
	address = "https://github.com/tkgamegroup/pugixml.git"
	lib_dir = thirdparty_directory / "pugixml"
	if not lib_dir.exists():
		if op == 2:
			print("Download PugiXML from %s into %s ? y/n" % (address, str(lib_dir)))
			ok = input() == "y"
		if ok:
			os.system("git clone --depth 1 %s %s && echo ok" % (address, str(lib_dir)))
	else:
		print("%s exists, skip download" % str(lib_dir))
	print("====\n")

	ok = True
	bud_dir = lib_dir / "build"
	if not bud_dir.exists():
		if op == 2:
			print("Build PugiXML into %s ? y/n" % str(bud_dir))
			ok = input() == "y"
		if ok:
			bud_dir.mkdir()
			os.system("cmake -S \"%s\" -B \"%s\"" % (lib_dir, bud_dir))
			os.chdir("%s/MSBuild/Current/Bin" % str(vs_path));
			files = glob.glob("%s/*.sln" % str(bud_dir))
			if len(files) > 0:
				os.system("msbuild \"%s\"" % files[0])
			else:
				print("CMake Failed")
			os.chdir(current_directory)	
	else:
		print("%s exists, skip build" % str(bud_dir))
	print("====\n")
			
	print("== library nlohmann json ==")
	ok = True
	address = "https://github.com/tkgamegroup/json.git"
	lib_dir = thirdparty_directory / "njson"
	if not lib_dir.exists():
		if op == 2:
			print("Download nlohmann json from %s into %s ? y/n" % (address, str(lib_dir)))
			ok = input() == "y"
		if ok:
			os.system("git clone --depth 1 %s %s && echo ok" % (address, str(lib_dir)))
	else:
		print("%s exists, skip download" % str(lib_dir))
	print("====\n")
			
	print("== library cppcodec ==")
	ok = True
	address = "https://github.com/tkgamegroup/cppcodec.git"
	lib_dir = thirdparty_directory / "cppcodec"
	if not lib_dir.exists():
		if op == 2:
			print("Download cppcodec from %s into %s ? y/n" % (address, str(lib_dir)))
			ok = input() == "y"
		if ok:
			os.system("git clone --depth 1 %s %s && echo ok" % (address, str(lib_dir)))
	else:
		print("%s exists, skip download" % str(lib_dir))
	print("====\n")
			
	print("== library stb ==")
	ok = True
	address = "https://github.com/tkgamegroup/stb.git"
	lib_dir = thirdparty_directory / "stb"
	if not lib_dir.exists():
		if op == 2:
			print("Download STB from %s into %s ? y/n" % (address, str(lib_dir)))
			ok = input() == "y"
		if ok:
			os.system("git clone --depth 1 %s %s && echo ok" % (address, str(lib_dir)))
	else:
		print("%s exists, skip download" % str(lib_dir))
	print("====\n")
			
	# (Optional)
	if op != 3:
		print("== library msdfgen ==")
		ok = True
		address = "https://github.com/tkgamegroup/msdfgen.git"
		lib_dir = thirdparty_directory / "msdfgen"
		if not lib_dir.exists():
			if op == 2:
				print("Download msdfgen from %s into %s ? y/n" % (address, str(lib_dir)))
				ok = input() == "y"
			if ok:
				os.system("git clone --depth 1 %s %s && echo ok" % (address, str(lib_dir)))
		else:
			print("%s exists, skip download" % str(lib_dir))

		ok = True
		bud_dir = lib_dir / "build"
		if not bud_dir.exists():
			if op == 2:
				print("Build msdfgen into %s ? y/n" % str(bud_dir))
				ok = input() == "y"
			if ok:
				bud_dir.mkdir()
				os.system("cmake -S \"%s\" -B \"%s\" -DMSDFGEN_CORE_ONLY:BOOL=ON -DMSDFGEN_DYNAMIC_RUNTIME:BOOL=ON -DMSDFGEN_BUILD_STANDALONE:BOOL=OFF -DMSDFGEN_USE_SKIA:BOOL=OFF -DMSDFGEN_USE_VCPKG:BOOL=OFF" % (lib_dir, bud_dir))
				os.chdir("%s/MSBuild/Current/Bin" % str(vs_path));
				files = glob.glob("%s/*.sln" % str(bud_dir))
				if len(files) > 0:
					os.system("msbuild \"%s\"" % files[0])
				else:
					print("CMake Failed")
				os.chdir(current_directory)
		else:
			print("%s exists, skip build" % str(bud_dir))
		print("====\n")
			
	print("== library sha1 ==")
	ok = True
	address = "https://github.com/tkgamegroup/sha1.git"
	lib_dir = thirdparty_directory / "sha1"
	if not lib_dir.exists():
		if op == 2:
			print("Download SHA1 from %s into %s ? y/n" % (address, str(lib_dir)))
			ok = input() == "y"
		if ok:
			os.system("git clone --depth 1 %s %s && echo ok" % (address, str(lib_dir)))
	else:
		print("%s exists, skip download" % str(lib_dir))
	print("====\n")
			
	print("== library gli ==")
	ok = True
	address = "https://github.com/tkgamegroup/gli"
	lib_dir = thirdparty_directory / "gli"
	if not lib_dir.exists():
		if op == 2:
			print("Download GLI from %s into %s ? y/n" % (address, str(lib_dir)))
			ok = input() == "y"
		if ok:
			os.system("git clone --depth 1 %s %s && echo ok" % (address, str(lib_dir)))
	else:
		print("%s exists, skip download" % str(lib_dir))
	print("====\n")
			
	print("== library SPIRV-Cross ==")
	ok = True
	address = "https://github.com/tkgamegroup/SPIRV-Cross.git"
	lib_dir = thirdparty_directory / "SPIRV-Cross"
	if not lib_dir.exists():
		if op == 2:
			print("Download SPIRV-Cross from %s into %s ? y/n" % (address, str(lib_dir)))
			ok = input() == "y"
		if ok:
			os.system("git clone --depth 1 %s %s && echo ok" % (address, str(lib_dir)))
	else:
		print("%s exists, skip download" % str(lib_dir))
	print("====\n")
			
	ok = True
	bud_dir = lib_dir / "build"
	if not bud_dir.exists():
		if op == 2:
			print("Build SPIRV-Cross into %s ? y/n" % str(bud_dir))
			ok = input() == "y"
		if ok:
			bud_dir.mkdir()
			os.system("cmake -S \"%s\" -B \"%s\"" % (lib_dir, bud_dir))
			os.chdir("%s/MSBuild/Current/Bin" % str(vs_path));
			files = glob.glob("%s/*.sln" % str(bud_dir))
			if len(files) > 0:
				os.system("msbuild \"%s\"" % files[0])
			else:
				print("CMake Failed")
			os.chdir(current_directory)
	else:
		print("%s exists, skip build" % str(bud_dir))
	print("====\n")
			
	print("== library exprtk ==")
	ok = True
	address = "https://github.com/tkgamegroup/exprtk.git"
	lib_dir = thirdparty_directory / "exprtk"
	if not lib_dir.exists():
		if op == 2:
			print("Download exprtk from %s into %s ? y/n" % (address, str(lib_dir)))
			ok = input() == "y"
		if ok:
			os.system("git clone --depth 1 %s %s && echo ok" % (address, str(lib_dir)))
	else:
		print("%s exists, skip download" % str(lib_dir))
	print("====\n")
		
	# (Optional)
	if op != 3:
		print("== library assimp ==")
		ok = True
		address = "https://github.com/tkgamegroup/assimp.git"
		lib_dir = thirdparty_directory / "assimp"
		if not lib_dir.exists():
			if op == 2:
				print("Download Assimp from %s into %s ? y/n" % (address, str(lib_dir)))
				ok = input() == "y"
			if ok:
				os.system("git clone --depth 1 %s %s && echo ok" % (address, str(lib_dir)))
		else:
			print("%s exists, skip download" % str(lib_dir))

		ok = True
		bud_dir = lib_dir / "build"
		if not bud_dir.exists():
			if op == 2:
				print("Build Assimp into %s ? y/n" % str(bud_dir))
				ok = input() == "y"
			if ok:
				bud_dir.mkdir()
				os.system("cmake -S \"%s\" -B \"%s\" -DASSIMP_WARNINGS_AS_ERRORS=OFF" % (lib_dir, bud_dir))
				os.system("xcopy %s %s" % (lib_dir / "build/include/assimp/config.h", lib_dir / "include/assimp/config.h"))
				os.chdir("%s/MSBuild/Current/Bin" % str(vs_path));
				files = glob.glob("%s/*.sln" % str(bud_dir))
				if len(files) > 0:
					os.system("msbuild \"%s\"" % files[0])
				else:
					print("CMake Failed")
				os.chdir(current_directory)
		else:
			print("%s exists, skip build" % str(bud_dir))
		print("====\n")
			
	# (Optional)
	'''
	if op != 3:
		print("== library PhysX ==")
		ok = True
		address = "https://github.com/NVIDIAGameWorks/PhysX.git"
		lib_dir = parent_directory / "PhysX"
		if op == 2:
			print("Download PhysX from %s into %s ? y/n" % (address, str(lib_dir)))
			ok = input() == "y"
		if ok:
			if not lib_dir.exists():
				os.system("git clone --depth 1 %s %s && echo ok" % (address, str(lib_dir)))
			else:
				print("%s exists, skip download" % str(lib_dir))
		print("====\n")
	'''
			
	# (Optional)
	if op != 3:
		print("== library recastnavigation ==")
		ok = True
		address = "https://github.com/tkgamegroup/recastnavigation.git"
		lib_dir = thirdparty_directory / "recastnavigation"
		if not lib_dir.exists():
			if op == 2:
				print("Download recastnavigation from %s into %s ? y/n" % (address, str(lib_dir)))
				ok = input() == "y"
			if ok:
				os.system("git clone --depth 1 %s %s && echo ok" % (address, str(lib_dir)))
		else:
			print("%s exists, skip download" % str(lib_dir))

		ok = True
		bud_dir = lib_dir / "build"
		if not bud_dir.exists():
			if op == 2:
				print("Build recastnavigation into %s ? y/n" % str(bud_dir))
				ok = input() == "y"
			if ok:
				bud_dir.mkdir()
				os.system("cmake -S \"%s\" -B \"%s\" -D RECASTNAVIGATION_DEMO=OFF -D RECASTNAVIGATION_EXAMPLES=OFF -D RECASTNAVIGATION_TESTS=OFF" % (lib_dir, bud_dir))
				os.chdir("%s/MSBuild/Current/Bin" % str(vs_path));
				files = glob.glob("%s/*.sln" % str(bud_dir))
				if len(files) > 0:
					os.system("msbuild \"%s\"" % files[0])
				else:
					print("CMake Failed")
				os.chdir(current_directory)
		else:
			print("%s exists, skip build" % str(bud_dir))
		print("====\n")
	
	# (Optional)
	if op != 3:
		print("== library Font-Awesome ==")
		ok = True
		address = "https://github.com/tkgamegroup/Font-Awesome.git"
		lib_dir = thirdparty_directory / "Font-Awesome"
		if not lib_dir.exists():
			if op == 2:
				print("Download Font-Awesome from %s into %s ? y/n" % (address, str(lib_dir)))
				ok = input() == "y"
			if ok:
				os.system("git clone --depth 1 %s %s && echo ok" % (address, str(lib_dir)))
		else:
			print("%s exists, skip download" % str(lib_dir))
		print("====\n")
			
	# (Optional)
	if op != 3:
		print("== library imgui ==")
		ok = True
		address = "https://github.com/tkgamegroup/imgui.git"
		lib_dir = thirdparty_directory / "imgui"
		if not lib_dir.exists():
			if op == 2:
				print("Download imgui from %s into %s ? y/n" % (address, str(lib_dir)))
				ok = input() == "y"
			if ok:
				os.system("git clone --depth 1 --branch docking %s %s && echo ok" % (address, str(lib_dir)))
		else:
			print("%s exists, skip download" % str(lib_dir))
		print("====\n")
			
	# (Optional)
	if op != 3:
		print("== library ImGuizmo ==")
		ok = True
		address = "https://github.com/tkgamegroup/ImGuizmo.git"
		lib_dir = thirdparty_directory / "ImGuizmo"
		if not lib_dir.exists():
			if op == 2:
				print("Download ImGuizmo from %s into %s ? y/n" % (address, str(lib_dir)))
				ok = input() == "y"
			if ok:
				os.system("git clone --depth 1 %s %s && echo ok" % (address, str(lib_dir)))
		else:
			print("%s exists, skip download" % str(lib_dir))
		print("====\n")
		
	# (Optional)
	if op != 3:
		print("== library ImGui-Node-Editor ==")
		ok = True
		address = "https://github.com/tkgamegroup/imgui-node-editor.git"
		lib_dir = thirdparty_directory / "imgui-node-editor"
		if not lib_dir.exists():
			if op == 2:
				print("Download ImGui-Node-Editor from %s into %s ? y/n" % (address, str(lib_dir)))
				ok = input() == "y"
			if ok:
				os.system("git clone --depth 1 %s %s && echo ok" % (address, str(lib_dir)))
		else:
			print("%s exists, skip download" % str(lib_dir))
		print("====\n")
			
	# (Optional)
	if op != 3:
		print("== library FortuneAlgorithm ==")
		ok = True
		address = "https://github.com/tkgamegroup/FortuneAlgorithm.git"
		lib_dir = thirdparty_directory / "FortuneAlgorithm"
		if not lib_dir.exists():
			if op == 2:
				print("Download FortuneAlgorithm from %s into %s ? y/n" % (address, str(lib_dir)))
				ok = input() == "y"
			if ok:
				os.system("git clone --depth 1 %s %s && echo ok" % (address, str(lib_dir)))
		else:
			print("%s exists, skip download" % str(lib_dir))

		ok = True
		bud_dir = lib_dir / "build"
		if not bud_dir.exists():
			if op == 2:
				print("Build FortuneAlgorithm into %s ? y/n" % str(bud_dir))
				ok = input() == "y"
			if ok:
				bud_dir.mkdir()
				os.system("cmake -S \"%s\" -B \"%s\"" % (lib_dir, bud_dir))
				os.chdir("%s/MSBuild/Current/Bin" % str(vs_path));
				files = glob.glob("%s/*.sln" % str(bud_dir))
				if len(files) > 0:
					os.system("msbuild \"%s\"" % files[0])
				else:
					print("CMake Failed")
				os.chdir(current_directory)
		else:
			print("%s exists, skip build" % str(bud_dir))
		print("====\n")

cmake_cl = "cmake -B build "

path = thirdparty_directory / "glm"
if path.exists():
	cmake_cl += "-DGLM_INCLUDE_DIR=\"%s\" " % str(path)
	
path = thirdparty_directory / "pugixml/src"
if path.exists():
	cmake_cl += "-DPUGIXML_INCLUDE_DIR=\"%s\" " % str(path)
	
path = thirdparty_directory / "pugixml/build/debug/pugixml.lib"
if path.exists():
	cmake_cl += "-DPUGIXML_DEBUG_LIB=\"%s\" " % str(path)
	
path = thirdparty_directory / "pugixml/build/release/pugixml.lib"
if path.exists():
	cmake_cl += "-DPUGIXML_RELEASE_LIB=\"%s\" " % str(path)
	
path = thirdparty_directory / "njson/include"
if path.exists():
	cmake_cl += "-DNJSON_INCLUDE_DIR=\"%s\" " % str(path)
	
path = thirdparty_directory / "cppcodec"
if path.exists():
	cmake_cl += "-DCPPCODEC_INCLUDE_DIR=\"%s\" " % str(path)
	
path = thirdparty_directory / "stb"
if path.exists():
	cmake_cl += "-DSTB_INCLUDE_DIR=\"%s\" " % str(path)
	
path = thirdparty_directory / "msdfgen"
if path.exists():
	cmake_cl += "-DMSDFGEN_INCLUDE_DIR=\"%s\" " % str(path)
	
path = thirdparty_directory / "msdfgen/build/debug/msdfgen-core.lib"
if path.exists():
	cmake_cl += "-DMSDFGEN_LIB_PATH=\"%s\" " % str(path)
	
path = thirdparty_directory / "sha1"
if path.exists():
	cmake_cl += "-DSHA1_INCLUDE_DIR=\"%s\" " % str(path)
	
path = thirdparty_directory / "gli"
if path.exists():
	cmake_cl += "-DGLI_INCLUDE_DIR=\"%s\" " % str(path)
	
path = thirdparty_directory / "exprtk"
if path.exists():
	cmake_cl += "-DEXPRTK_INCLUDE_DIR=\"%s\" " % str(path)
	
path = thirdparty_directory / "SPIRV-Cross"
if path.exists():
	cmake_cl += "-DSPIRV_CROSS_INCLUDE_DIR=\"%s\" " % str(path)
	
path = thirdparty_directory / "SPIRV-Cross/build/debug"
if path.exists():
	cmake_cl += "-DSPIRV_CROSS_DEBUG_LIBS_DIR=\"%s\" " % str(path)
	
path = thirdparty_directory / "SPIRV-Cross/build/release"
if path.exists():
	cmake_cl += "-DSPIRV_CROSS_RELEASE_LIBS_DIR=\"%s\" " % str(path)
	
path = thirdparty_directory / "assimp/include"
if path.exists():
	cmake_cl += "-DASSIMP_INCLUDE_DIR=\"%s\" " % str(path)
	
path = thirdparty_directory / "assimp/build/lib/Debug/assimp-vc143-mtd.lib"
if path.exists():
	cmake_cl += "-DASSIMP_LIB_PATH=\"%s\" " % str(path)
else:
	path = thirdparty_directory / "assimp/build/lib/Debug/assimp-vc142-mtd.lib"
	if path.exists():
		cmake_cl += "-DASSIMP_LIB_PATH=\"%s\" " % str(path)
	
path = thirdparty_directory / "assimp/build/bin/Debug/assimp-vc143-mtd.dll"
if path.exists():
	cmake_cl += "-DASSIMP_DLL_PATH=\"%s\" " % str(path)
else:
	path = thirdparty_directory / "assimp/build/bin/Debug/assimp-vc142-mtd.dll"
	if path.exists():
		cmake_cl += "-DASSIMP_DLL_PATH=\"%s\" " % str(path)
	
path = thirdparty_directory / "PhysX"
if path.exists():
	cmake_cl += "-DPHYSX_INCLUDE_DIR=\"%s\" " % str(path)
	
path = thirdparty_directory / "recastnavigation/Detour/Include"
if path.exists():
	cmake_cl += "-DDETOUR_INCLUDE_DIR=\"%s\" " % str(path)
	
path = thirdparty_directory / "recastnavigation/build/Detour/Debug/Detour-d.lib"
if path.exists():
	cmake_cl += "-DDETOUR_DEBUG_LIB_PATH=\"%s\" " % str(path)
	
path = thirdparty_directory / "recastnavigation/build/Detour/Release/Detour.lib"
if path.exists():
	cmake_cl += "-DDETOUR_RELEASE_LIB_PATH=\"%s\" " % str(path)
	
path = thirdparty_directory / "recastnavigation/DetourCrowd/Include"
if path.exists():
	cmake_cl += "-DDETOUR_CROWD_INCLUDE_DIR=\"%s\" " % str(path)
	
path = thirdparty_directory / "recastnavigation/build/DetourCrowd/Debug/DetourCrowd-d.lib"
if path.exists():
	cmake_cl += "-DDETOUR_CROWD_DEBUG_LIB_PATH=\"%s\" " % str(path)
	
path = thirdparty_directory / "recastnavigation/build/DetourCrowd/Release/DetourCrowd.lib"
if path.exists():
	cmake_cl += "-DDETOUR_CROWD_RELEASE_LIB_PATH=\"%s\" " % str(path)
	
path = thirdparty_directory / "recastnavigation/DetourTileCache/Include"
if path.exists():
	cmake_cl += "-DDETOUR_TILE_CACHE_INCLUDE_DIR=\"%s\" " % str(path)
	
path = thirdparty_directory / "recastnavigation/build/DetourTileCache/Debug/DetourTileCache-d.lib"
if path.exists():
	cmake_cl += "-DDETOUR_TILE_CACHE_DEBUG_LIB_PATH=\"%s\" " % str(path)
	
path = thirdparty_directory / "recastnavigation/build/DetourTileCache/Release/DetourTileCache.lib"
if path.exists():
	cmake_cl += "-DDETOUR_TILE_CACHE_RELEASE_LIB_PATH=\"%s\" " % str(path)
	
path = thirdparty_directory / "recastnavigation/Recast/Include"
if path.exists():
	cmake_cl += "-DRECAST_INCLUDE_DIR=\"%s\" " % str(path)
	
path = thirdparty_directory / "recastnavigation/build/Recast/Debug/Recast-d.lib"
if path.exists():
	cmake_cl += "-DRECAST_DEBUG_LIB_PATH=\"%s\" " % str(path)
	
path = thirdparty_directory / "recastnavigation/build/Recast/Release/Recast.lib"
if path.exists():
	cmake_cl += "-DRECAST_RELEASE_LIB_PATH=\"%s\" " % str(path)
	
path = thirdparty_directory / "Font-Awesome"
if path.exists():
	cmake_cl += "-DFONT_AWESOME_DIR=\"%s\" " % str(path)
	
path = thirdparty_directory / "imgui"
if path.exists():
	cmake_cl += "-DIMGUI_DIR=\"%s\" " % str(path)
	
path = thirdparty_directory / "ImGuizmo"
if path.exists():
	cmake_cl += "-DIM_GUIZMO_DIR=\"%s\" " % str(path)
	
path = thirdparty_directory / "imgui-node-editor"
if path.exists():
	cmake_cl += "-DIMGUI_NODE_EDITOR_DIR=\"%s\" " % str(path)
	
path = thirdparty_directory / "FortuneAlgorithm"
if path.exists():
	cmake_cl += "-DFORTUNE_ALGORITHM_DIR=\"%s\" " % str(path)
	
os.system(cmake_cl)
	
print("\nSet environment variable of Flame Engine:")
os.system("setx FLAME_PATH \"%s\"" % str(current_directory))

print("\nRegister DIA dll ? y/n")
if input() == "y":
	os.system("regsvr32 \"%s/DIA SDK/bin/amd64/msdia140.dll\"" % str(vs_path))
	
print("\nDone.")
os.system("pause")
