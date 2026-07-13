#pragma once

/// engine
#include "Engine/Asset/Assets/IAsset.h"
#include <vector>
#include <cstdint>

namespace ONEngine::Asset {

class FontAsset final : public IAsset {
public:
	struct MetaData {
		// 空のメタデータ構造体
	};

public:
	FontAsset() = default;
	~FontAsset() override = default;

	// フォントファイル（.ttf）のバイナリデータ
	std::vector<uint8_t> fontData;
};

} // namespace ONEngine::Asset
