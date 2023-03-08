import os;
import sys;

def build_with_type(typeString):
    buildDir = "build/{}".format(typeString);
    cmakeString = "mkdir {0} & cd {0} & cmake -DCMAKE_BUILD_TYPE={1} ../../".format(buildDir, typeString)

    os.makedirs(buildDir, exist_ok = True)
    os.chdir(buildDir)
    os.system(cmakeString)

def panic_help():
    print("Specify r/d/release/debug");
    exit()

if (len(sys.argv) == 1):
    panic_help()

buildType = sys.argv[1].lower();

if (buildType == "r" or buildType == "release"):
    build_with_type("Release");

elif (buildType == "d" or buildType == "debug"):
    build_with_type("Debug");

else:
    panic_help()

# os.system("make");
