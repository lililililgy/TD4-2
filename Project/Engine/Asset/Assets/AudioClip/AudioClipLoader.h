#pragma once

#include "../IAssetLoader.h"
#include "../../Meta/MetaFile.h"
#include "AudioClip.h"

namespace ONEngine::Asset {

template<>
class AssetLoader<AudioClip> : public IAssetLoader {
public:
	/// ===================================================
	/// public : methods
	/// ===================================================

	AssetLoader() = default;
	~AssetLoader() override = default;

	std::optional<AudioClip> Load(const std::string& filepath, Meta<typename AudioClip::MetaData> meta);
	std::optional<AudioClip> Reload(const std::string& filepath, AudioClip* src = nullptr, Meta<typename AudioClip::MetaData> meta = {});

	Meta<typename AudioClip::MetaData> GetMetaData(const std::string& filepath);

};

} /// namespace ONEngine::Asset