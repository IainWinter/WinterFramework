cfgdir = "%{cfg.buildcfg}"

src_framework =
{
	path.getabsolute("src")
}

lib_framework =
{
	path.getabsolute("lib/%{cfgdir}")
}

defines_all =
{
	"SDL_MAIN_HANDLED",
	"wTOOLS=\"" .. path.getabsolute("tools/") .. "\"",
	"wVENDOR=\"" .. path.getabsolute("vendor/") .. "\""
}

include_vendor =
{
	path.getabsolute("vendor/_shared/include"),

	path.getabsolute("vendor/box2d/include"),
	path.getabsolute("vendor/imgui/include"),
	path.getabsolute("vendor/hitbox/include"),
	path.getabsolute("vendor/phonon_fmod/include")
}

src_vendor =
{
	path.getabsolute("vendor/_shared/src/**.h"),
	path.getabsolute("vendor/_shared/src/**.hpp"),
	path.getabsolute("vendor/_shared/src/**.cpp"),
	path.getabsolute("vendor/_shared/src/**.c"),

	path.getabsolute("vendor/imgui/backends/imgui_impl_sdl.cpp"),
	path.getabsolute("vendor/imgui/backends/imgui_impl_opengl3.cpp")
}

libdir_vendor_self = 
{
	path.getabsolute("vendor/box2d/lib/%{cfgdir}"),
	path.getabsolute("vendor/imgui/lib/%{cfgdir}"),
	path.getabsolute("vendor/hitbox/lib/%{cfgdir}"),
	path.getabsolute("vendor/fmod_steamaudio/lib/%{cfgdir}")
}

libdir_vendor_win_x64 =
{
	path.getabsolute("vendor/_shared/lib/win_x64")
}

include_vendor_win =
{
	path.getabsolute("vendor/_shared/include/_win")
}

libdir_vendor_mac_arm64 =
{
	path.getabsolute("vendor/_shared/lib/mac_arm64"),

	"/opt/homebrew/Cellar/sdl2/2.0.22/lib",
	"/opt/homebrew/Cellar/assimp/5.2.5/lib/"
}

include_vendor_mac =
{
	"/opt/homebrew/Cellar/sdl2/2.0.22/include",
	"/opt/homebrew/Cellar/assimp/5.2.5/include/"
}

libdir_vendor_linux_arm64 =
{
	path.getabsolute("vendor/_shared/lib/linux_x64")
}

include_vendor_linux =
{
	-- fill this in
}

links_vendor =
{
	"SDL2",
	"box2d",
	"ImGui", 
	"hitbox",
	"fmod",
	"fmodstudio",
	"phonon",
	"phonon_fmod",
	"assimp"
}

links_windows =
{
	"gdi32",
	"ws2_32",
	"wsock32",
	"user32",
	"kernel32",
	"shell32"
}

defines_win =
{
	"wPREMAKE=\"premake5.exe\"",
	"wPREMAKE_BUILD=\"vs2022\"",
	"wBUILD_CLI=\"msbuild\"",
	"IPL_OS_WINDOWS"
}

defines_mac = 
{
	"wPREMAKE=\"mac_premake5\"",
	"wPREMAKE_BUILD=\"xcode4\"",
	"wBUILD_CLI=\"needs link\"",
	"IPL_OS_MACOSX"
}

defines_linux = {
	"wPREMAKE=\"premake5\"",
	"wPREMAKE_BUILD=\"gmake2\"",
	"wBUILD_CLI=\"make\""
}