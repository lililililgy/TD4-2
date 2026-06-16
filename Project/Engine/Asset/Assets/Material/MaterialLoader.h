#pragma once

/// engine
#include "../IAssetLoader.h"
#include "../../Meta/MetaFile.h"
#include "Material.h"

namespace ONEngine::Asset {

template<>
class AssetLoader<Material> : public IAssetLoader {
public:
	/// ==================================================
	/// public : methods
	/// ==================================================

	AssetLoader() = default;
	~AssetLoader() override = default;

	/// @brief モデルの読み込みを行う
	/// @param filepath 対象のファイルパス
	/// @return 読み込んだモデル
	[[nodiscard]]
	std::optional<Material> Load(const std::string& filepath, Meta<typename Material::MetaData> meta);

	/// @brief モデルの再読み込みを行う
	/// @param filepath 対象のファイルパス
	/// @param src 元のモデル(この関数内で使用されないのでnullptrで良い)
	/// @return 再読み込みしたモデル
	[[nodiscard]]
	std::optional<Material> Reload(const std::string& filepath, Material* src = nullptr, Meta<typename Material::MetaData> meta = {});

	Meta<typename Material::MetaData> GetMetaData(const std::string& filepath);

};

} /// namespace ONEngine::Asset