#include "MaterialLoader.h"

/// std
#include <fstream>

/// engine
#include "Engine/Asset/Meta/MetaFile.h"


namespace ONEngine::Asset {


std::optional<Material> AssetLoader<Material>::Load(const std::string& filepath, Meta<Material::MetaData> meta) {
	/// ファイルを開く
	std::ifstream ifs(filepath);
	if(!ifs) {
		Console::LogError("[Load Failed] [Material] - File not found: \"" + filepath + "\"");
		return std::nullopt;
	}


	/// 読み込んだMaterialを格納するオブジェクト
	Material material;
	material.guid = meta.base.guid;

	/// ----------------------------------------------
	/// ファイルの読み込み
	/// ----------------------------------------------
	std::string line;
	while(std::getline(ifs, line)) {
		/// ----- 各文字列ごとに対応した処理を行う ----- ///
		/// guidはmetaファイルから読み込むように変更したが、ファイル内にもある場合はスキップするか上書きするか
		/// ここではMetaファイルを正とする

	}

	/// コンソールにログを出力
	Console::Log("[Load] [Material] - path:\"" + filepath + "\"");

	return std::move(material);
}

std::optional<Material> AssetLoader<Material>::Reload(const std::string& filepath, Material* /*src*/, Meta<Material::MetaData> meta) {
	/// Materialの再読み込みは新規読み込みと同じ処理を行う
	return std::move(Load(filepath, meta));
}


Meta<Material::MetaData> AssetLoader<Material>::GetMetaData(const std::string& filepath) {
	Meta<Material::MetaData> res{};

	const std::string metaPath = filepath + ".meta";
	res.base = LoadOrGenerateMetaBase(metaPath, filepath);

	nlohmann::json j;
	std::ifstream ifs(metaPath);
	if(!ifs.is_open()) {
		return {};
	}

	try {
		ifs >> j;
		ifs.close();
	} catch (const nlohmann::json::parse_error& e) {
		Console::LogError("[Material Meta Error] JSON parse error in " + metaPath + ": " + e.what());
		ifs.close();
		return res;
	}

	Material::MetaData data;
	data.useShader = j.value("useShader", std::string(""));
	data.albedoColor = j.value("albedoColor", Vector4::One);
	data.albedoTextureGuid = j.value("albedoTextureGuid", Guid::kInvalid);
	data.normalTextureGuid = j.value("normalTextureGuid", Guid::kInvalid);

	res.data = data;

	return res;
}


} /// namespace ONEngine::Asset