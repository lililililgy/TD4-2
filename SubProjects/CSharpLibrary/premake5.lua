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

-- Hook VS actions to patch csproj files dynamically to use portable pdb
local vs_actions = { "vs2022", "vs2019", "vs2017" }
for _, actionName in ipairs(vs_actions) do
    local action = premake.action.get(actionName)
    if action then
        local baseOnProject = action.onProject
        action.onProject = function(prj)
            baseOnProject(prj)
            if prj.language == "C#" then
                local path = prj.name .. ".csproj"
                local f, err = io.open(path, "r")
                if f then
                    local content = f:read("*all")
                    f:close()
                    -- Replace <DebugType>...</DebugType> with <DebugType>portable</DebugType>
                    local newContent, count = string.gsub(content, "<DebugType>[%w_]+</DebugType>", "<DebugType>portable</DebugType>")
                    if count > 0 then
                        local wf, werr = io.open(path, "w")
                        if wf then
                            wf:write(newContent)
                            wf:close()
                            print("Hook (" .. actionName .. "): Successfully patched DebugType to portable in " .. path)
                        else
                            print("Hook WARNING (" .. actionName .. "): Failed to write patched project file: " .. tostring(werr))
                        end
                    end
                else
                    print("Hook WARNING (" .. actionName .. "): Failed to open project file for reading: " .. tostring(err))
                end
            end
        end
    end
end
