import pathlib
import os

current_directory = pathlib.Path.cwd()
parent_directory = current_directory.parent
print("Current Directory: " + str(current_directory))

os.system("chcp 65001")

print("Download or build required libraries?\n 1 - Automaticly\n 2 - Manually\n 3 - Skip")
op = int(input())

if op != 3:
	ok = False
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

	ok = False
	address = "https://github.com/zeux/pugixml"
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
		if lib_dir.exists() and not bud_dir.exists():
			bud_dir.mkdir()
			os.system("cmake -S \"%s\" -B \"%s\"" % (lib_dir, bud_dir))
		else:
			print("%s exists, skip build" % str(bud_dir))

os.system("setx FLAME_PATH %s" % str(current_directory))

p = os.environ.get("VS170COMCOMNTOOLS")
if p is not None:
	vs_path = pathlib.Path(p).parent.parent
else:
	p = os.environ.get("VS160COMCOMNTOOLS")
	if p is not None:
		vs_path = pathlib.Path(p).parent.parent
	else:
		p = os.environ.get("VS150COMCOMNTOOLS")
		if p is not None:
			vs_path = pathlib.Path(p).parent.parent
		else:
			vs_path = None
			print("Cannot find visual studio location")
if vs_path is not None:
	os.system("regsvr32 \"%s/DIA SDK/bin/amd64/msdia140.dll\"" % vs_path)
