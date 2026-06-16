#pragma once

/// external
#include <nlohmann/json.hpp>

/// engine
#include "../../Interface/IComponent.h"

/// @brief ポストエフェクトの種類
enum PostEffectType {
	PostEffectType_Grayscale,  ///< グレースケール
	PostEffectType_RadialBlur, ///< ラジアルブラー
	PostEffectType_Count	   ///< 要素数
};

/// ///////////////////////////////////////////////////
/// スクリーンにかけるポストエフェクトのフラグを持つコンポーネント
/// ///////////////////////////////////////////////////
namespace ONEngine {

class ScreenPostEffectTag : public IComponent {
public:
	/// ===================================================
	/// public : methods
	/// ===================================================

	ScreenPostEffectTag() = default;
	~ScreenPostEffectTag() override = default;

	/// @brief ポストエフェクトの有効/無効を設定する
	/// @param type 対象のポストエフェクトの種類
	/// @param enable true: 有効 false: 無効
	void SetPostEffectEnable(PostEffectType type, bool enable);

	/// @brief 指定した種類のポストエフェクトが有効かどうかを返す
	/// @param type 確認するポストエフェクトの種類
	/// @return true: 有効 false: 無効
	bool GetPostEffectEnable(PostEffectType type) const;

private:
	/// ===================================================
	/// private : methods
	/// ===================================================

};


namespace ComponentDebug {
	void ScreenPostEffectTagDebug(ScreenPostEffectTag* component);
}


/// Json変換
void from_json(const nlohmann::json& j, ScreenPostEffectTag& c);
void to_json(nlohmann::json& j, const ScreenPostEffectTag& c);

} /// ONEngine
