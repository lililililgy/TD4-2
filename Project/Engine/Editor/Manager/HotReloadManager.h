#pragma once

#include <string>
#include <vector>
#include <mutex>
#include <filesystem>
#include <algorithm>
#include "Engine/Editor/Math/FileWatcher/FileWatcher.h"
#include "Engine/Core/Utility/Tools/Log.h"
#include "Engine/Script/MonoScriptEngine.h"
#include "Engine/Asset/AssetType.h"

namespace Editor {

class HotReloadManager {
public:
    static HotReloadManager& GetInstance() {
        static HotReloadManager instance;
        return instance;
    }

    void Initialize() {
        std::lock_guard<std::mutex> lock(mutex_);
        if (isInitialized_) return;

        std::vector<std::wstring> watchDirs = {
            L"./Assets",
            L"./Packages",
            L"../SubProjects/CSharpLibrary/Scripts"
        };
        fileWatcher_.Start(watchDirs);
        isInitialized_ = true;
    }

    void Update() {
        Initialize(); // 未初期化なら初期化する

        // ホットリロードの処理中（コピー処理中など）であれば、
        // その間の FileWatcher イベントは無限ループ防止のためすべてスルーして破棄する
        if (ONEngine::MonoScriptEngine::GetInstance().IsReloading()) {
            auto events = fileWatcher_.ConsumeEvents();
            for (const auto& ev : events) {
                std::wstring pathW = ev.path;
                std::string pathA(pathW.begin(), pathW.end());
                ONEngine::Console::Log("[MonoDbg] (Ignored during reload) FileWatcher detected change: " + pathA, ONEngine::LogCategory::ScriptEngine);
            }
            return;
        }

        // ファイル監視イベントの処理
        auto events = fileWatcher_.ConsumeEvents();
        if (events.empty()) return;

        std::lock_guard<std::mutex> lock(mutex_);
        for (const auto& ev : events) {
            latestEvents_.push_back(ev);

            // デバッグ用のログを出力
            std::string actionStr = "Unknown";
            if (ev.action == FileEvent::Action::Added) actionStr = "Added";
            else if (ev.action == FileEvent::Action::Removed) actionStr = "Removed";
            else if (ev.action == FileEvent::Action::Modified) actionStr = "Modified";
            else if (ev.action == FileEvent::Action::RenamedOld) actionStr = "RenamedOld";
            else if (ev.action == FileEvent::Action::RenamedNew) actionStr = "RenamedNew";

            std::wstring pathW = ev.path;
            std::string pathA(pathW.begin(), pathW.end());
            ONEngine::Console::Log("[MonoDbg] FileWatcher detected change: " + pathA + " (Action: " + actionStr + ")", ONEngine::LogCategory::ScriptEngine);

            if (ev.type == FileEvent::Type::File) {
                std::string relPath = GetRelativePath(ev.path);
                ONEngine::Console::Log("[MonoDbg]   File type: " + relPath, ONEngine::LogCategory::ScriptEngine);

                if (ev.action == FileEvent::Action::Added || ev.action == FileEvent::Action::Modified || ev.action == FileEvent::Action::RenamedNew) {
                    // アセットの拡張子である場合のみアセット再ロードを要求する
                    std::string extension = std::filesystem::path(relPath).extension().string();
                    ONEngine::Asset::AssetType assetType = ONEngine::Asset::GetAssetTypeFromExtension(extension);
                    if (assetType != ONEngine::Asset::AssetType::None) {
                        // アセットの再ロード要求
                        pendingAssetReloads_.push_back(relPath);
                    }

                    // タイムスタンプ付きの DLL が更新された場合のみホットリロード要求
                    // (固定名 DLL/PDB や PDB 更新での複数回トリガー、およびビルド前の .cs 更新での無駄なトリガーを防ぐため)
                    bool isTimestampedDll = relPath.find("CSharpLibrary_") != std::string::npos && relPath.ends_with(".dll");

                    if (isTimestampedDll) {
                        ONEngine::Console::Log("[MonoDbg] Triggering ScriptHotReload for: " + relPath, ONEngine::LogCategory::ScriptEngine);
                        pendingScriptHotReload_ = true;
                    }
                }
            }
        }
    }

    std::vector<FileEvent> ConsumeLatestEvents() {
        std::lock_guard<std::mutex> lock(mutex_);
        return std::move(latestEvents_);
    }

    void ClearQueuedEvents() {
        std::lock_guard<std::mutex> lock(mutex_);
        // ファイル監視キューに溜まっている未処理イベントをすべて破棄する
        fileWatcher_.ConsumeEvents();
        pendingAssetReloads_.clear();
        pendingScriptHotReload_ = false;
        latestEvents_.clear();
        ONEngine::Console::Log("[MonoDbg] Cleared queued FileWatcher events during HotReload completion to prevent self-loop.", ONEngine::LogCategory::ScriptEngine);
    }

    void RequestAssetReload(const std::string& path) {
        std::lock_guard<std::mutex> lock(mutex_);
        pendingAssetReloads_.push_back(path);
    }

    void RequestScriptHotReload() {
        std::lock_guard<std::mutex> lock(mutex_);
        pendingScriptHotReload_ = true;
    }

    struct Requests {
        std::vector<std::string> assetPaths;
        bool scriptHotReload;
    };

    Requests ConsumeRequests() {
        std::lock_guard<std::mutex> lock(mutex_);
        if (pendingScriptHotReload_) {
            ONEngine::Console::Log("[MonoDbg] ConsumeRequests - Consuming script hot reload request.", ONEngine::LogCategory::ScriptEngine);
        }
        Requests reqs = { std::move(pendingAssetReloads_), pendingScriptHotReload_ };
        pendingAssetReloads_.clear();
        pendingScriptHotReload_ = false;
        return reqs;
    }

private:
    HotReloadManager() = default;

    std::string GetRelativePath(const std::filesystem::path& absolutePath) {
        const std::filesystem::path basePath = std::filesystem::absolute("./");
        std::filesystem::path relativePath = std::filesystem::relative(absolutePath, basePath);
        std::string relativeStr = relativePath.string();

        if (!relativeStr.empty() && relativeStr[0] != '.') {
            relativeStr = "./" + relativeStr;
        } else if (relativeStr == ".") {
            relativeStr = "./";
        }
        std::replace(relativeStr.begin(), relativeStr.end(), '\\', '/');
        return relativeStr;
    }

    std::mutex mutex_;
    FileWatcher fileWatcher_;
    bool isInitialized_ = false;
    std::vector<FileEvent> latestEvents_;
    std::vector<std::string> pendingAssetReloads_;
    bool pendingScriptHotReload_ = false;
};

} // namespace Editor
