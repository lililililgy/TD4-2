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
    postbuildcommands {
        "call \"$(ProjectDir)RenameDll.bat\"",
        "call \"$(ProjectDir)KeepLatest.bat\""
    }
