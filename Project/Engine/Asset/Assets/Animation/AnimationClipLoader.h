#pragma once

/// engine
#include "../IAssetLoader.h"
#include "AnimationClip.h"
#include "Engine/Asset/Meta/MetaFile.h"

namespace ONEngine::Asset {

/// ///////////////////////////////////////////////////
/// AnimationClip用のアセットローダー
/// ///////////////////////////////////////////////////
template<>
class AssetLoader<AnimationClip> : public IAssetLoader {
public:
    AssetLoader() = default;
    ~AssetLoader() override = default;

    /// @brief アニメーションクリップの読み込みを行う
    [[nodiscard]]
    std::optional<AnimationClip> Load(const std::string& filepath, typename Meta<AnimationClip::MetaData> meta);

    /// @brief アニメーションクリップの再読み込みを行う
    std::optional<AnimationClip> Reload(const std::string& filepath, AnimationClip* src, typename Meta<AnimationClip::MetaData> meta);

    /// @brief MetaDataの取得を行う
    Meta<typename AnimationClip::MetaData> GetMetaData(const std::string& filepath);
};

} /// namespace ONEngine::Asset
