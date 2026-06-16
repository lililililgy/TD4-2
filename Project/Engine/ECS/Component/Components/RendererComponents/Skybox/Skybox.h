#pragma once

/// std
#include <string>

/// externals
#include <nlohmann/json.hpp>

/// engine
#include "../../Interface/IComponent.h"

/// ///////////////////////////////////////////////////
/// Skyboxのコンポーネントクラス
/// ///////////////////////////////////////////////////
namespace ONEngine {

class Skybox : public IRenderComponent {
public:
	/// ===================================================
	/// public : methods
	/// ===================================================

	Skybox();
	~Skybox() override;


	/// @brief DDSテクスチャのパスを設定する
	/// @param texturePath テクスチャのパス
	void SetDDSTexturePath(const std::string& texturePath);

	/// @brief DDSテクスチャのパスを返す
	const std::string& GetDDSTexturePath() const;

private:
	/// ===================================================
	/// private : objects
	/// ===================================================

	std::string texturePath_;

};


namespace ComponentDebug {
	void SkyboxDebug(const Skybox* skybox);
}

/// Json変換
void from_json(const nlohmann::json& j, Skybox& s);
void to_json(nlohmann::json& j, const Skybox& s);

} /// ONEngine