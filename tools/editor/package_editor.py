import pathlib
import shutil

def copy_executable(src, dst):
    shutil.copyfile(src, dst / src.name)
    
    pdb_filename = src.stem + '.pdb'
    pdb_source_path = src.with_name(pdb_filename)

    if pdb_source_path.is_file():
        shutil.copy(pdb_source_path, dst / pdb_filename)
    
    ti_filename = src.stem + '.typeinfo'
    ti_source_path = src.with_name(ti_filename)

    if ti_source_path.is_file():
        shutil.copy(ti_source_path, dst / ti_filename)
    
print("Destination path: ")
path = pathlib.Path(input())
parent_directory = path.parent
parent_directory.mkdir(parents=True, exist_ok=True)

for dll_file in pathlib.Path("../../bin/debug").glob("*.dll"):
    copy_executable(pathlib.Path(dll_file), parent_directory)

copy_executable(pathlib.Path("../../bin/debug/editor.exe"), parent_directory)
shutil.copyfile("preferences.ini", parent_directory / "preferences.ini")
shutil.copyfile("imgui.ini", parent_directory / "imgui.ini")

shutil.copytree("../../assets", parent_directory / "flame")
