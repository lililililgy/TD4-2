#pragma once

/// std
#include <optional>

/// engine
#include "../IAsset.h"
#include "Engine/Asset/Guid/Guid.h"
#include "Engine/Core/Utility/Utility.h"
#include "Engine/Graphics/Buffer/Data/UVTransform.h"


namespace ONEngine::Asset {


/// ShowGuiMaterialように前方宣言
class Material;

/// @brief デフォルトのマテリアルを生成
Material GenerateMaterial();

/// @brief Materialファイルを生成
/// @param filepath 生成先のファイルパス
/// @param material 参照するマテリアル nullptrならデフォルト値で生成
void GenerateMaterialFile(const std::string& filepath, Material* material);

/// Json変換
void from_json(const nlohmann::json& j, Material& material);
void to_json(nlohmann::json& j, const Material& material);

/// ////////////////////////////////////////////////////////
/// マテリアル
/// ////////////////////////////////////////////////////////
class Material final : public IAsset {
	/// friend functions
	friend void from_json(const nlohmann::json& j, Material& material);
	friend void to_json(nlohmann::json& j, const Material& material);

public:

	/// @brief Materialのメタデータ
	struct MetaData {
		std::string useShader;
		Vector4 albedoColor;
		Guid albedoTextureGuid;
		Guid normalTextureGuid;
	};


public:
	/// ==================================================
	/// public : methods
	/// ==================================================

	Material();
	~Material();


	/// @brief BaseTextureのGuidを持っているかチェック
	/// @return true: 持っている, false: 持っていない
	bool HasBaseTexture() const;

	/// @brief BaseTextureのGuidを取得
	/// @return BaseTextureのGuid
	const Guid& GetBaseTextureGuid() const;

	/// @brief base textureのGuidを設定
	/// @param textureGuid base textureのGuid
	void SetBaseTextureGuid(const Guid& textureGuid);


	/// @brief 法線テクスチャのGuidを持っているかチェック
	/// @return true: 持っている, false: 持っていない
	bool HasNormalTexture() const;

	/// @brief 法線テクスチャのGuidを取得
	/// @return 法線テクスチャのGuid
	const Guid& GetNormalTextureGuid() const;

	/// @brief normal textureのGuidを設定
	/// @param textureGuid normal textureのGuid
	void SetNormalTextureGuid(const Guid& textureGuid);

private:
	/// ==================================================
	/// private : objects
	/// ==================================================

	std::optional<Guid> baseTextureGuid_;   /// ベーステクスチャのGUID
	std::optional<Guid> normalTextureGuid_; /// 法線テクスチャのGUID


public:
	/// ==================================================
	/// public : objects
	/// ==================================================

	Vector4             baseColor;
	uint32_t            postEffectFlags;
	UVTransform         uvTransform;
	float               bloomIntensity;
	float               bloomThreshold;
	float               bloomRadius;
};


} /// namespace ONEngine::Asset
