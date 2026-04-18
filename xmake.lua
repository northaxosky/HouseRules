-- set minimum xmake version
set_xmakever("2.9.4")

-- include CommonLibF4 (default: submodule at lib/commonlibf4; override with COMMONLIBF4_PATH env var)
local commonlib_path = os.getenv("COMMONLIBF4_PATH") or "lib/commonlibf4"
includes(commonlib_path)

-- set project
set_project("SurvivalArchitect")
set_version("0.1.0")
set_license("MIT")

-- set defaults
set_languages("c++23")
set_warnings("allextra")
set_encodings("utf-8")

-- set policies
set_policy("package.requires_lock", true)

-- add rules
add_rules("mode.debug", "mode.releasedbg")
add_rules("plugin.vsxmake.autoupdate")

-- targets
target("SurvivalArchitect")
    add_deps("commonlibf4")

    add_rules("commonlibf4.plugin", {
        name = "SurvivalArchitect",
        author = "Kuz",
        description = "Curated survival mode configuration for Fallout 4"
    })

    add_files("src/**.cpp")
    add_headerfiles("src/**.h")
    add_includedirs("src")
    set_pcxxheader("src/PCH.h")

    add_extrafiles(".clang-format")
