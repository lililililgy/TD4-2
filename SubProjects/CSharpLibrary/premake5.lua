workspace "CSharpLibrary"
    configurations { "Debug", "Release" }
    platforms { "x64" }
    location "."

project "CSharpLibrary"
    kind "SharedLib"
    language "C#"
    clr "Unsafe"
    dotnetframework "4.7.2"
    architecture "x64"

    targetdir "../../Project/Packages/Scripts"
    objdir "../../Generated/CSharpScripts/%{cfg.buildcfg}"

    -- 全ての.csファイルを再帰的に含める
    files { 
        "**.cs"
    }

    -- 不要なフォルダを除外（あれば）
    removefiles {
        "bin/**",
        "obj/**",
        "packages/**"
    }

    -- 参照設定
    links {
        "System",
        "System.Core",
        "System.Xml",
        "System.Xml.Linq",
        "System.Data",
        "System.Data.DataSetExtensions",
        "System.Net.Http",
        "Microsoft.CSharp",
        "packages/Newtonsoft.Json.13.0.3/lib/net45/Newtonsoft.Json.dll"
    }

    filter "configurations:Debug"
        symbols "On"
        defines { "DEBUG", "TRACE" }

    filter "configurations:Release"
        optimize "On"
        defines { "TRACE" }

    filter {}

    -- ビルド後イベント
    filter "system:windows"
        postbuildcommands {
            [[powershell -NoProfile -ExecutionPolicy Bypass -File "$(ProjectDir)RenameDll.ps1"]],
            [[call "$(ProjectDir)KeepLatest.bat"]]
        }

-- Hook vs2022 onProject to patch csproj files dynamically to use portable pdb
local vs2022 = premake.action.get("vs2022")
local baseOnProject = vs2022.onProject
vs2022.onProject = function(prj)
    baseOnProject(prj)
    if prj.language == "C#" then
        local path = prj.name .. ".csproj"
        local f = io.open(path, "r")
        if f then
            local content = f:read("*all")
            f:close()
            -- Replace <DebugType>...</DebugType> with <DebugType>portable</DebugType>
            local newContent, count = string.gsub(content, "<DebugType>[%w_]+</DebugType>", "<DebugType>portable</DebugType>")
            if count > 0 then
                f = io.open(path, "w")
                f:write(newContent)
                f:close()
                print("Hook: Successfully patched DebugType to portable in " .. path)
            end
        end
    end
end
