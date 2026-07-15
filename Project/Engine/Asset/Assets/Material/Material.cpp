#include "Material.h"

/// std
#include <fstream>
#include <filesystem>

/// externals
#include <imgui.h>

/// engine
#include "Engine/Editor/Commands/ImGuiCommand/ImGuiCommand.h"

namespace ONEngine::Asset {

Material GenerateMaterial() {
	/// ----- 新規のMaterialを作成して返す ----- ///
	Material material;

	material.guid = GenerateGuid();
	material.baseColor = Vector4::White;
	material.postEffectFlags = 1;
	material.bloomIntensity = 1.0f;
	material.bloomThreshold = 0.8f;

	return material;
}

void GenerateMaterialFile(const std::string& filepath, Material* material) {
	/// filepathにマテリアル情報を書き込む

	/// filepathがないなら生成する
	if(std::filesystem::exists(filepath) == false) {
		std::ofstream ofs(filepath);
		ofs.close();
	}

	std::ofstream ofs(filepath);
	if(!ofs) {
		return;
	}


	/// 引数のマテリアル情報を使用する
	/// nullptrならデフォルト値で生成する
	Material mat;
	if(material) {
		mat = *material;
	} else {
		mat = GenerateMaterial();
	}

	/// ファイルに情報を書きこむ
	ofs << "MaterialFileVersion: 1\n";
	ofs << "guid: " << mat.guid.ToString() << "\n";
	ofs << "BaseColor: " << mat.baseColor.x << " " << mat.baseColor.y << " " << mat.baseColor.z << " " << mat.baseColor.w << "\n";
	ofs << "PostEffectFlags: " << mat.postEffectFlags << "\n";
	ofs << "UVTransform_Position: " << mat.uvTransform.position.x << " " << mat.uvTransform.position.y << "\n";
	ofs << "UVTransform_Scale: " << mat.uvTransform.scale.x << " " << mat.uvTransform.scale.y << "\n";
	ofs << "UVTransform_Rotate: " << mat.uvTransform.rotate << "\n";
	ofs << "BloomIntensity: " << mat.bloomIntensity << "\n";
	ofs << "BloomThreshold: " << mat.bloomThreshold << "\n";

	ofs.close();
}


/// ---------------------------------------------------
/// Json変換
/// ---------------------------------------------------

void from_json(const nlohmann::json& j, Material& material) {
	/// ----- JsonデータをMaterialに変換する ----- ///

	j.at("baseColor").get<ONEngine::Vector4>();

	material.guid = j.value("guid", Guid{});
	material.baseColor = j.value("baseColor", Vector4::Red);
	material.postEffectFlags = j.value("postEffectFlags", 1u);
	material.uvTransform = j.value("uvTransform", UVTransform{});
	material.bloomIntensity = j.value("bloomIntensity", 1.0f);
	material.bloomThreshold = j.value("bloomThreshold", 0.8f);

	Guid baseTextureGuid = j.value("baseTextureGuid", Guid::kInvalid);
	if(baseTextureGuid.CheckValid()) {
		material.baseTextureGuid_ = baseTextureGuid;
	} else {
		material.baseTextureGuid_ = std::nullopt;
	}

	Guid normalTextureGuid = j.value("normalTextureGuid", Guid::kInvalid);
	if(normalTextureGuid.CheckValid()) {
		material.normalTextureGuid_ = normalTextureGuid;
	} else {
		material.normalTextureGuid_ = std::nullopt;
	}
}

void to_json(nlohmann::json& j, const Material& material) {
	/// ----- MaterialデータをJsonに変換する ----- ///
	j = {
		{ "guid", material.guid },
		{ "baseColor", material.baseColor },
		{ "postEffectFlags", material.postEffectFlags },
		{ "uvTransform", material.uvTransform },
		{ "baseTextureGuid", material.baseTextureGuid_.has_value() ? material.baseTextureGuid_.value() : Guid::kInvalid },
		{ "normalTextureGuid", material.normalTextureGuid_.has_value() ? material.normalTextureGuid_.value() : Guid::kInvalid },
		{ "bloomIntensity", material.bloomIntensity },
		{ "bloomThreshold", material.bloomThreshold }
	};
}


/// //////////////////////////////////////////////////////////
/// Material
/// //////////////////////////////////////////////////////////

Material::Material() {
	baseColor = Vector4::White;
	postEffectFlags = 1;
	uvTransform = UVTransform();
	bloomIntensity = 1.0f;
	bloomThreshold = 0.8f;
};
Material::~Material() = default;



bool Material::HasBaseTexture() const {
	return baseTextureGuid_.has_value();
}

const Guid& Material::GetBaseTextureGuid() const {
	return baseTextureGuid_.value();
}

void Material::SetBaseTextureGuid(const Guid& textureGuid) {
	/// ----- base texture guidの設定 ----- ///
	if(baseTextureGuid_.has_value()) {
		baseTextureGuid_.value() = textureGuid;
	} else {
		baseTextureGuid_ = std::make_optional<Guid>();
		baseTextureGuid_ = textureGuid;
	}
}

bool Material::HasNormalTexture() const {
	return normalTextureGuid_.has_value();
}

const Guid& Material::GetNormalTextureGuid() const {
	return normalTextureGuid_.value();
}

void Material::SetNormalTextureGuid(const Guid& textureGuid) {
	/// ----- 法線 texture の guid を登録 ----- ///
	if(normalTextureGuid_.has_value()) {
		normalTextureGuid_.value() = textureGuid;
	} else {
		normalTextureGuid_ = std::make_optional<Guid>();
		normalTextureGuid_ = textureGuid;
	}
}

} /// namespace ONEngine::Asset