#pragma once
#include "FontAsset.h"
#include "Engine/Asset/Assets/IAssetLoader.h"
#include "Engine/Asset/Meta/MetaFile.h"
#include "Engine/Core/Utility/Tools/Log.h"
#include <fstream>

namespace ONEngine::Asset {

template<>
class AssetLoader<FontAsset> final : public IAssetLoader {
public:
	AssetLoader(DxManager* dxm, AssetCollection* ac)
		: pDxManager_(dxm), pAssetCollection_(ac) {}
	~AssetLoader() override = default;

	std::optional<FontAsset> Load(const std::string& filepath, Meta<typename FontAsset::MetaData> meta) {
		std::ifstream file(filepath, std::ios::binary | std::ios::ate);
		if (!file.is_open()) {
			Console::LogError("[Font Loader] Failed to open file: \"" + filepath + "\"");
			return std::nullopt;
		}

		std::streamsize size = file.tellg();
		file.seekg(0, std::ios::beg);

		FontAsset fontAsset;
		fontAsset.fontData.resize(size);
		if (file.read(reinterpret_cast<char*>(fontAsset.fontData.data()), size)) {
			fontAsset.guid = meta.base.guid;
			Console::Log("[Font Loader] Successfully loaded font: \"" + filepath + "\"");
			return fontAsset;
		}

		Console::LogError("[Font Loader] Failed to read font file data: \"" + filepath + "\"");
		return std::nullopt;
	}

	std::optional<FontAsset> Reload(const std::string& filepath, FontAsset* src, Meta<typename FontAsset::MetaData> meta) {
		auto fontOpt = Load(filepath, meta);
		if (fontOpt.has_value() && src) {
			src->fontData = std::move(fontOpt->fontData);
			src->guid = meta.base.guid;
			return *src;
		}
		return std::nullopt;
	}

	Meta<typename FontAsset::MetaData> GetMetaData(const std::string& filepath) {
		Meta<typename FontAsset::MetaData> res{};
		const std::string metaPath = filepath + ".meta";
		res.base = LoadOrGenerateMetaBase(metaPath, filepath);
		return res;
	}

private:
	DxManager* pDxManager_;
	AssetCollection* pAssetCollection_;
};

} // namespace ONEngine::Asset
