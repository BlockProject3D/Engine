import sys
import os
import os.path
import platform
import urllib.request
import zipfile

ReleaseNumber = "6b36c02"

release = ""

def is_os_64bit():
    return platform.machine().endswith('64')

if (platform.system() == "Darwin"):
    release = "MacOSX"
else:
    release = platform.system()

if (is_os_64bit()):
    release = release + "-x64"
else:
    release = release + "-x86"

if (len(sys.argv) > 1):
    release = sys.argv[1]

fileName = "Framework-" + release + ".zip"

if (not(os.path.exists(fileName))):
    dlUrl = "https://github.com/BlockProject3D/Framework/releases/download/" + release + "-" + ReleaseNumber + "/Framework.zip"
    urllib.request.urlretrieve(dlUrl, "Framework-" + release + ".zip")
    handle = zipfile.ZipFile(fileName, "r")
    handle.extractall("Framework-" + release)
    handle.close()
