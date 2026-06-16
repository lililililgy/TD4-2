#pragma once

/// directX
#include <d3d12.h>
#include <DirectXTex.h>

/// engine
#include "../IAssetLoader.h"
#include "Texture.h"
#include "Engine/Asset/Meta/MetaFile.h"

#include "Engine/Core/DirectX12/Resource/DxResource.h"

namespace ONEngine::Asset {
class AssetCollection;
}

namespace ONEngine::Asset {

/// ///////////////////////////////////////////////////
/// Texture用のアセットローダー
/// ///////////////////////////////////////////////////
template<>
class AssetLoader<Texture> : public IAssetLoader {
public:

	AssetLoader(DxManager* dxm, AssetCollection* ac);
	~AssetLoader() override = default;



	/// @brief テクスチャの読み込みを行う
	/// @param filepath 対象のファイルパス
	/// @return 読み込んだテクスチャ
	[[nodiscard]]
	std::optional<Texture> Load(const std::string& filepath, typename Meta<Texture::MetaData> meta);

	/// @brief テクスチャの再読み込みを行う
	/// @param filepath 対象のファイルパス
	/// @return 読み込んだテクスチャ
	std::optional<Texture> Reload(const std::string& filepath, Texture* src, typename Meta<Texture::MetaData> meta);

	/// @brief MetaDataの取得を行う
	/// @param filepath メタデータを取得する対象のファイルパス
	/// @return メタデータ
	Meta<typename Texture::MetaData> GetMetaData(const std::string& filepath);




	/// @brief テクスチャ2Dの読み込みを行う
	/// @param filepath 対象のファイルパス
	/// @param meta MetaFile
	/// @return 読み込んだテクスチャ
	std::optional<Texture> Load2DTexture(const std::string& filepath);

	/// @brief テクスチャ3Dの読み込みを行う
	/// @param filepath 対象 of ファイルパス
	/// @param meta MetaFile
	/// @return 読み込んだテクスチャ
	std::optional<Texture> Load3DTexture(const std::string& filepath);


	/// @brief テクスチャ2Dの再読み込みを行う
	/// @param filepath 対象のファイルパス
	/// @param src 再読み込み元のテクスチャ
	/// @param meta MetaFile
	/// @return 再読み込みしたテクスチャ
	std::optional<Texture> Reload2DTexture(const std::string& filepath, Texture* src);

	/// @brief テクスチャ3Dの再読み込みを行う
	/// @param filepath 対象のファイルパス
	/// @param src 再読み込み元のテクスチャ
	/// @param meta MetaFile
	/// @return 再読み込みしたテクスチャ
	std::optional<Texture> Reload3DTexture(const std::string& filepath, Texture* src);


	/// @brief 2Dテクスチャ用のScratchImageを読み込む
	/// @param filepath 対象のファイルパス
	/// @param meta MetaFile
	/// @return 2Dテクスチャ用のScratchImage
	DirectX::ScratchImage LoadScratchImage2D(const std::string& filepath);

	/// @brief 3Dテクスチャ用のScratchImageを読み込む
	/// @param filepath 対象のファイルパス
	/// @param meta MetaFile
	/// @return 3Dテクスチャ用のScratchImage
	DirectX::ScratchImage LoadScratchImage3D(const std::string& filepath);


	/// @brief テクスチャのリソースを作成する(2D)
	/// @param dxDevice DxDeviceのポインタ
	/// @param metadata Metadata情報
	/// @return 作成したテクスチャリソース
	[[nodiscard]]
	DxResource CreateTextureResource2D(class DxDevice* dxDevice, const DirectX::TexMetadata& metadata);

	/// @brief テクスチャのリソースを作成する(3D)
	/// @param dxDevice DxDeviceのポインタ
	/// @param metadata Metadata情報
	/// @return 作成したテクスチャリソース
	[[nodiscard]]
	DxResource CreateTextureResource3D(class DxDevice* dxDevice, const DirectX::TexMetadata& metadata);

	/// @brief テクスチャデータをアップロードする
	/// @param texture 対象のテクスチャリソース
	/// @param mipScratchImage ミップマップを含むスクラッチイメージ
	/// @return アップロードに使用した中間リソース
	[[nodiscard]]
	DxResource UploadTextureData(ID3D12Resource* texture, const DirectX::ScratchImage& mipScratchImage);

private:

	DxManager* pDxManager_;
	AssetCollection* pAssetCollection_;

};

} /// namespace ONEngine