import os
import xml.etree.ElementTree as ET

flame_path = os.environ["FLAME_PATH"]
os.system("reg add HKEY_CLASSES_ROOT\\Directory\\Background\\shell\\NewComponent /VE /F /D \"New Component\"")
os.system("reg add HKEY_CLASSES_ROOT\\Directory\\Background\\shell\\NewComponent\\Command /VE /F /D \"%s\\code_helper.exe -cmd new_component\""  % (flame_path + "\\bin\\debug"))

os.system("\"%s\\vswhere.exe\" -latest -property catalog_productLine > temp.txt" % flame_path)
file = open("temp.txt")
vs_id = file.read().strip("\n").strip("Dev") + ".0"
file.close()

os.system("\"%s\\vswhere.exe\" -latest -property instanceId > temp.txt" % flame_path)
file = open("temp.txt")
vs_id = vs_id + "_" + file.read().strip("\n")
file.close()

vs_settings = ET.parse("%s\\microsoft\\VisualStudio\\%s\\Settings\\CurrentSettings.vssettings" % (os.environ["LOCALAPPDATA"], vs_id))
vs_settings.find("ExternalTools")

# Have not find a good way to add by code
print("You can add this tool to visual studio's external tools, use \"-path $(ItemPath)\" as parameter")
