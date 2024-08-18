import os

os.system("reg add HKEY_CLASSES_ROOT\\Directory\\Background\\shell\\TexturePacker /VE /F /D TexturePacker")
os.system("reg add HKEY_CLASSES_ROOT\\Directory\\Background\\shell\\TexturePacker\\Command /VE /F /D %s\\texture_packer.exe" % (os.environ["FLAME_PATH"] + "\\bin\\debug"))
