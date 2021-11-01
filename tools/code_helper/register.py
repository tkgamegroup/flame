import os

flame_path = os.environ["FLAME_PATH"]
os.system("reg add HKEY_CLASSES_ROOT\\Directory\\Background\\shell\\NewComponentTemplate /VE /F /D \"New Component Template\"")
os.system("reg add HKEY_CLASSES_ROOT\\Directory\\Background\\shell\\NewComponentTemplate\\Command /VE /F /D \"%s\\code_helper.exe -cmd new_component_template\""  % (flame_path + "\\bin\\debug"))
os.system("reg add HKEY_CLASSES_ROOT\\Directory\\Background\\shell\\NewSystemTemplate /VE /F /D \"New System Template\"")
os.system("reg add HKEY_CLASSES_ROOT\\Directory\\Background\\shell\\NewSystemTemplate\\Command /VE /F /D \"%s\\code_helper.exe -cmd new_system_template\""  % (flame_path + "\\bin\\debug"))
os.system("reg add HKEY_CLASSES_ROOT\\Directory\\Background\\shell\\NewGeneralTemplate /VE /F /D \"New General Template\"")
os.system("reg add HKEY_CLASSES_ROOT\\Directory\\Background\\shell\\NewGeneralTemplate\\Command /VE /F /D \"%s\\code_helper.exe -cmd new_general_template\""  % (flame_path + "\\bin\\debug"))

# Have not find a good way to add by code
print("You can add this tool to visual studio's external tools, use \"-path $(ItemPath)\" as parameter")
