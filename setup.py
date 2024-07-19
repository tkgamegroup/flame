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
	os.system("vcpkg install glm")
	print("====\n")
			
	print("== library PugiXML ==")
	os.system("vcpkg install pugixml")
	print("====\n")
			
	print("== library nlohmann json ==")
	os.system("vcpkg install nlohmann_json")
	print("====\n")
			
	print("== library cppcodec ==")
	os.system("vcpkg install cppcodec")
	print("====\n")
			
	print("== library stb ==")
	os.system("vcpkg install stb")
	print("====\n")
			
	print("== library exprtk ==")
	os.system("vcpkg install exprtk")
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
	
	# (Optional)
	if op != 3:
		print("== library curl ==")
		ok = True
		if op == 2:
			print("Install curl ? y/n")
			ok = input() == "y"
		if ok:
			os.system("vcpkg install curl[ssl]")
		print("====\n")
			
	# (Optional)
	if op != 3:
		print("== library msdfgen ==")
		ok = True
		if op == 2:
			print("Install msdfgen ? y/n")
			ok = input() == "y"
		if ok:
			os.system("vcpkg install msdfgen")
		print("====\n")
			
	print("== library gli ==")
	os.system("vcpkg install gli")
	print("====\n")
			
	print("== library SPIRV-Cross ==")
	os.system("vcpkg install spirv-cross")
	print("====\n")
		
	# (Optional)
	if op != 3:
		print("== library assimp ==")
		ok = True
		if op == 2:
			print("Install assimp ? y/n")
			ok = input() == "y"
		if ok:
			os.system("vcpkg install assimp")
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
		if op == 2:
			print("Install recastnavigation ? y/n")
			ok = input() == "y"
		if ok:
			os.system("vcpkg install recastnavigation")
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

path = thirdparty_directory / "sha1"
if path.exists():
	cmake_cl += "-DSHA1_INCLUDE_DIR=\"%s\" " % str(path)
	
path = thirdparty_directory / "PhysX"
if path.exists():
	cmake_cl += "-DPHYSX_INCLUDE_DIR=\"%s\" " % str(path)
	
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
