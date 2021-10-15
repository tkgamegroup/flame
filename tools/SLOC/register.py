import os

os.system("reg add HKEY_CLASSES_ROOT\\Directory\\Background\\shell\\SLOC /VE /F /D SLOC")
os.system("reg add HKEY_CLASSES_ROOT\\Directory\\Background\\shell\\SLOC\\Command /VE /F /D %s\\SLOC.exe" % (os.environ["FLAME_PATH"] + "\\bin\\debug"))
