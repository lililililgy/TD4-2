#pragma once

/// std
#include <vector>

/// externals
#include <nlohmann/json.hpp>

/// engine
#include "../../Interface/IComponent.h"
#include "Terrain.h"


/// @brief  Debug関数様に前方宣言
namespace ONEngine {

class TerrainCollider;

/// @brief Componentのデバッグ表示
namespace ComponentDebug {
void TerrainColliderDebug(TerrainCollider* collider);
}

/// Json変換関数
void from_json(const nlohmann::json& j, TerrainCollider& c);
void to_json(nlohmann::json& j, const TerrainCollider& c);

/// ///////////////////////////////////////////////////
/// 地形のコライダーコンポーネント
/// ///////////////////////////////////////////////////
class TerrainCollider : public IComponent {
	/// ----- friend class  ----- ///
	friend class TerrainColliderVertexGenerator;

	/// ----- friend functions ----- ///
	friend void ComponentDebug::TerrainColliderDebug(TerrainCollider* collider);
	friend void from_json(const nlohmann::json& j, TerrainCollider& c);
	friend void to_json(nlohmann::json& j, const TerrainCollider& c);

public:
	/// =========================================
	/// public : methods
	/// =========================================

	TerrainCollider();
	~TerrainCollider() override = default;

	/// @brief 地形Componentをアタッチする
	void AttachTerrain();

	/// @brief 地形の頂点情報をコピーする
	void CopyVertices(class DxManager* dxm);

	/// @brief 地形の高さを取得する
	/// @param position 取得したい場所
	/// @return 高さ
	float GetHeight(const Vector3& position);

	/// @brief 引数座標の勾配を取得する
	/// @param position ワールド座標
	Vector3 GetGradient(const Vector3& position);

	/// @brief 地形の内側にいるかどうかを判定する
	/// @param position 判定したい座標
	/// @return true: 内側 false: 外側
	bool IsInsideTerrain(const Vector3& position);


private:
	/// =========================================
	/// private : objects
	/// =========================================

	/// ----- other class  ----- ///
	Terrain* pTerrain_;

	std::vector<std::vector<TerrainVertex>> vertices_;

	bool isVertexGenerationRequested_;
	bool isCreated_;

	float maxSlopeAngle_;

public:
	/// =========================================
	/// public : accessor
	/// =========================================

	/// @brief Terrainへのポインタ
	/// @return 
	Terrain* GetTerrain() const;

	/// @brief Colliderの頂点情報
	const std::vector<std::vector<TerrainVertex>>& GetVertices() const;
	std::vector<std::vector<TerrainVertex>>& GetVertices();

	bool GetIsCreated() const;


	void SetIsVertexGenerationRequested(bool isRequested);

	/// @brief 地形の移動制限に用いる最大傾斜各を得る
	/// @return 最大傾斜角
	float GetMaxSlopeAngle() const;

};


} /// ONEngine
