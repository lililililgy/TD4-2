#pragma once
#include "FontAtlas.h"
#include <string>
#include <unordered_map>
#include <memory>
#include <mutex>

namespace ONEngine {

class FontAtlasManager {
private:
	std::unordered_map<std::string, std::unique_ptr<FontAtlas>> atlases_;
	std::mutex mutex_;

	FontAtlasManager() = default;
	~FontAtlasManager() = default;

public:
	static FontAtlasManager& GetInstance() {
		static FontAtlasManager instance;
		return instance;
	}

	FontAtlasManager(const FontAtlasManager&) = delete;
	FontAtlasManager& operator=(const FontAtlasManager&) = delete;

	// アトラスを取得または作成
	FontAtlas* GetOrCreateAtlas(const std::string& fontPath, int fontSize, int outlineWidth);

	// 全てのアトラスのGPUテクスチャを更新
	void UpdateAllAtlases();

	// 全アトラスを解放
	void Clear();
};

} // namespace ONEngine
