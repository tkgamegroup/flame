import pathlib
import os
import glob

current_directory = pathlib.Path.cwd()
parent_directory = current_directory.parent
print("Current Directory: " + str(current_directory))

os.system("chcp 65001")

os.system("vswhere.exe -latest -property installationPath > temp.txt")
file = open("temp.txt")
vs_path = pathlib.Path(file.read().strip("\n"))
file.close()
if not vs_path.exists():
	print("Cannot find visual studio, abort")
	exit(0)

print("Download or build required libraries?\n 1 - Automaticly\n 2 - Manually\n 3 - Skip")
op = int(input())

if op != 3:
	ok = True
	address = "https://github.com/g-truc/glm.git"
	lib_dir = parent_directory / "glm"
	if op == 2:
		print("Download glm from %s into %s ? y/n" % (address, str(lib_dir)))
		ok = input() == "y"
	if ok:
		if not lib_dir.exists():
			os.system("git clone --depth 1 %s %s && echo ok" % (address, str(lib_dir)))
		else:
			print("%s exists, skip download" % str(lib_dir))
			
	ok = True
	address = "https://github.com/zeux/pugixml.git"
	lib_dir = parent_directory / "pugixml"
	if op == 2:
		print("Download PugiXML from %s into %s ? y/n" % (address, str(lib_dir)))
		ok = input() == "y"
	if ok:
		if not lib_dir.exists():
			os.system("git clone --depth 1 %s %s && echo ok" % (address, str(lib_dir)))
		else:
			print("%s exists, skip download" % str(lib_dir))

	ok = True
	bud_dir = lib_dir / "build"
	if op == 2:
		print("Build PugiXML into %s ? y/n" % str(bud_dir))
		ok = input() == "y"
	if ok:
		if not bud_dir.exists():
			bud_dir.mkdir()
			os.system("cmake -S \"%s\" -B \"%s\"" % (lib_dir, bud_dir))
			os.chdir("%s/MSBuild/Current/Bin" % str(vs_path));
			os.system("msbuild \"%s\"" % glob.glob("%s/*.sln" % str(bud_dir))[0])
			os.chdir(current_directory)
		else:
			print("%s exists, skip build" % str(bud_dir))
			
	ok = True
	address = "https://github.com/nlohmann/json.git"
	lib_dir = parent_directory / "njson"
	if op == 2:
		print("Download Nlohmann Json from %s into %s ? y/n" % (address, str(lib_dir)))
		ok = input() == "y"
	if ok:
		if not lib_dir.exists():
			os.system("git clone --depth 1 %s %s && echo ok" % (address, str(lib_dir)))
		else:
			print("%s exists, skip download" % str(lib_dir))
			
	ok = True
	address = "https://github.com/tplgy/cppcodec.git"
	lib_dir = parent_directory / "cppcodec"
	if op == 2:
		print("Download cppcodec from %s into %s ? y/n" % (address, str(lib_dir)))
		ok = input() == "y"
	if ok:
		if not lib_dir.exists():
			os.system("git clone --depth 1 %s %s && echo ok" % (address, str(lib_dir)))
		else:
			print("%s exists, skip download" % str(lib_dir))
			
	ok = True
	address = "https://github.com/boostorg/regex.git"
	lib_dir = parent_directory / "boost"
	if op == 2:
		print("Download Boost Regex from %s into %s ? y/n" % (address, str(lib_dir)))
		ok = input() == "y"
	if ok:
		if not lib_dir.exists():
			os.system("git clone --depth 1 %s %s && echo ok" % (address, str(lib_dir)))
		else:
			print("%s exists, skip download" % str(lib_dir))
			
	ok = True
	address = "https://github.com/nothings/stb.git"
	lib_dir = parent_directory / "stb"
	if op == 2:
		print("Download STB from %s into %s ? y/n" % (address, str(lib_dir)))
		ok = input() == "y"
	if ok:
		if not lib_dir.exists():
			os.system("git clone --depth 1 %s %s && echo ok" % (address, str(lib_dir)))
		else:
			print("%s exists, skip download" % str(lib_dir))
			
	ok = True
	address = "https://github.com/vog/sha1.git"
	lib_dir = parent_directory / "sha1"
	if op == 2:
		print("Download SHA1 from %s into %s ? y/n" % (address, str(lib_dir)))
		ok = input() == "y"
	if ok:
		if not lib_dir.exists():
			os.system("git clone --depth 1 %s %s && echo ok" % (address, str(lib_dir)))
		else:
			print("%s exists, skip download" % str(lib_dir))
			
	ok = True
	address = "https://github.com/g-truc/gli.git"
	lib_dir = parent_directory / "gli"
	if op == 2:
		print("Download GLI from %s into %s ? y/n" % (address, str(lib_dir)))
		ok = input() == "y"
	if ok:
		if not lib_dir.exists():
			os.system("git clone --depth 1 %s %s && echo ok" % (address, str(lib_dir)))
		else:
			print("%s exists, skip download" % str(lib_dir))
			
	ok = True
	address = "https://github.com/KhronosGroup/SPIRV-Cross.git"
	lib_dir = parent_directory / "SPIRV-Cross"
	if op == 2:
		print("Download SPIRV-Cross from %s into %s ? y/n" % (address, str(lib_dir)))
		ok = input() == "y"
	if ok:
		if not lib_dir.exists():
			os.system("git clone --depth 1 %s %s && echo ok" % (address, str(lib_dir)))
		else:
			print("%s exists, skip download" % str(lib_dir))
			
	ok = True
	bud_dir = lib_dir / "build"
	if op == 2:
		print("Build SPIRV-Cross into %s ? y/n" % str(bud_dir))
		ok = input() == "y"
	if ok:
		if not bud_dir.exists():
			bud_dir.mkdir()
			os.system("cmake -S \"%s\" -B \"%s\"" % (lib_dir, bud_dir))
			os.chdir("%s/MSBuild/Current/Bin" % str(vs_path));
			os.system("msbuild \"%s\"" % glob.glob("%s/*.sln" % str(bud_dir))[0])
			os.chdir(current_directory)
		else:
			print("%s exists, skip build" % str(bud_dir))
			
	ok = True
	address = "https://github.com/lua/lua.git"
	lib_dir = parent_directory / "lua"
	if op == 2:
		print("Download LUA from %s into %s ? y/n" % (address, str(lib_dir)))
		ok = input() == "y"
	if ok:
		if not lib_dir.exists():
			os.system("git clone --depth 1 %s %s && echo ok" % (address, str(lib_dir)))
		else:
			print("%s exists, skip download" % str(lib_dir))

	ok = True
	bud_dir = lib_dir / "build"
	if op == 2:
		print("Build LUA into %s ? y/n" % str(bud_dir))
		ok = input() == "y"
	if ok:
		if not bud_dir.exists():
			os.system("\"\"%s/VC/Auxiliary/Build/vcvars64.bat\" && nmake -f \"%s\"\"" % (str(vs_path), str(bud_dir) + "/makefile"))
		else:
			print("%s exists, skip build" % str(bud_dir))
		
	ok = True
	address = "https://github.com/assimp/assimp.git"
	lib_dir = parent_directory / "assimp"
	if op == 2:
		print("Download Assimp from %s into %s ? y/n" % (address, str(lib_dir)))
		ok = input() == "y"
	if ok:
		if not lib_dir.exists():
			os.system("git clone --depth 1 %s %s && echo ok" % (address, str(lib_dir)))
		else:
			print("%s exists, skip download" % str(lib_dir))

	ok = True
	bud_dir = lib_dir / "build"
	if op == 2:
		print("Build Assimp into %s ? y/n" % str(bud_dir))
		ok = input() == "y"
	if ok:
		if not bud_dir.exists():
			bud_dir.mkdir()
			os.system("cmake -S \"%s\" -B \"%s\"" % (lib_dir, bud_dir))
			os.chdir("%s/MSBuild/Current/Bin" % str(vs_path));
			os.system("msbuild \"%s\"" % glob.glob("%s/*.sln" % str(bud_dir))[0])
			os.chdir(current_directory)
		else:
			print("%s exists, skip build" % str(bud_dir))
			
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
			
	ok = True
	address = "https://github.com/ocornut/imgui.git"
	lib_dir = parent_directory / "imgui"
	if op == 2:
		print("Download imgui from %s into %s ? y/n" % (address, str(lib_dir)))
		ok = input() == "y"
	if ok:
		if not lib_dir.exists():
			os.system("git clone --depth 1 %s %s && git checkout docking && echo ok" % (address, str(lib_dir)))
		else:
			print("%s exists, skip download" % str(lib_dir))
			
	ok = True
	address = "https://github.com/dfranx/ImFileDialog.git"
	lib_dir = parent_directory / "ImFileDialog"
	if op == 2:
		print("Download ImFileDialog from %s into %s ? y/n" % (address, str(lib_dir)))
		ok = input() == "y"
	if ok:
		if not lib_dir.exists():
			os.system("git clone --depth 1 %s %s && echo ok" % (address, str(lib_dir)))
		else:
			print("%s exists, skip download" % str(lib_dir))

os.system("setx FLAME_PATH \"%s\"" % str(current_directory))

os.system("regsvr32 \"%s/DIA SDK/bin/amd64/msdia140.dll\"" % str(vs_path))
