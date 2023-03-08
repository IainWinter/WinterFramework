workspace "YourProgram"
	configurations { "Debug", "Release" }
	platforms { "x64" }

asset_root_path = path.getabsolute("app/assets")

include "WinterFramework"
include "app"



