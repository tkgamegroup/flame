import os

os.system("reg add HKEY_CLASSES_ROOT\\Directory\\Background\\shell\\PackageProject /VE /F /D PackageProject")
os.system("reg add HKEY_CLASSES_ROOT\\Directory\\Background\\shell\\PackageProject\\Command /VE /F /D %s\\project_packager.exe" % (os.environ["FLAME_PATH"] + "\\bin\\debug"))
