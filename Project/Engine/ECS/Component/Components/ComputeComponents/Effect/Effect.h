#pragma once

/// std
#include <vector>
#include <string>
#include <functional>

/// engine
#include "Engine/Core/Utility/Math/Vector3.h"
#include "Engine/Core/Utility/Math/Vector4.h"
#include "Engine/Core/Utility/Math/Color.h"
#include "../../Interface/IComponent.h"
#include "../Transform/Transform.h"
#include "EmitShape/EffectEmitShape.h"
#include "MainModule/EffectMainModule.h"

/// ///////////////////////////////////////////////////
/// Effectクラス
/// ///////////////////////////////////////////////////
namespace ONEngine {

class Effect : public IComponent {
	friend class EffectUpdateSystem; ///< @brief EffectUpdateSystemからアクセスを許可  
public:
	/// ============================================  
	/// public : sub class  
	/// ============================================  

	enum class EmitType {
		Time,     ///< 時間で指定する場合
		Distance, ///< 距離で指定する場合
	};

	enum class BlendMode {
		Normal,
		Add,
		Sub,
		Multiply,
		Screen,
	};

	/// /////////////////////////////////////////////  
	/// @brief Effectの一要素  
	/// /////////////////////////////////////////////  
	struct Element final {
		Transform transform; /// 座標(SRT)  
		Color   color = Color::kWhite;     /// RGBA 0.0f ~ 1.0f  
		float     lifeTime = 0.0f;  /// エフェクトの寿命  
		Vector3   velocity = Vector3::Zero;  /// エフェクトの移動速度  
	};

	/// /////////////////////////////////////////////
	/// 出現するまでの距離を指定する場合のデータ
	/// /////////////////////////////////////////////
	struct DistanceEmitData final {
		Vector3 currentPosition{};
		Vector3 nextPosition{};
		float moveLength = 0.0f;
		float emitDistance = 0.0f;
		float emitInterval = 0.0f;
	};

	/// /////////////////////////////////////////////
	/// 出現するまでの時間を指定する場合のデータ
	/// /////////////////////////////////////////////
	struct TimeEmitData final {
		float emitTime = 0.0f;
		float emitInterval = 0.0f;
	};


public:
	/// ===================================================  
	/// public : methods  
	/// ===================================================  
	Effect();
	~Effect() = default;

	/// @brief 新しい要素の作成
	/// @param color 
	void CreateElement(const Vector3& position, const Color& color = Color::kWhite);
	void CreateElement(const Vector3& position, const Vector3& velocity, const Color& color);
	void CreateElement(const Vector3& position, const Vector3& scale, const Vector3& rotate, const Vector3& velocity, const Color& color);

	/// @brief Elementを削除する
	/// @param index 削除する要素のインデックス
	void RemoveElement(size_t index);

	/// @brief particle を出現させるかのフラグを取得
	bool GetIsCreateParticle() const { return isCreateParticle_; }

private:
	/// ===================================================  
	/// private : objects  
	/// ===================================================  

	bool isCreateParticle_ = true; ///!< これがtrueじゃないとパーティクルが出現しない


	size_t maxEffectCount_ = 1000;
	std::string meshPath_;
	std::string texturePath_;
	std::vector<Element> elements_;

	bool useBillboard_ = false; ///< ビルボードを使用するかどうか

	EffectMainModule mainModule_; ///< メインモジュール
	EffectEmitShape emitShape_;  ///< エミット形状

	EmitType emitType_ = EmitType::Time;
	DistanceEmitData distanceEmitData_{};
	TimeEmitData timeEmitData_{};

	size_t emitInstanceCount_ = 1; /// emitごとに生成するインスタンス数

	std::function<void(Element*)> elementUpdateFunc_ = nullptr; ///< エフェクトの更新関数

	BlendMode blendMode_ = BlendMode::Normal; ///< ブレンドモード



public:
	/// ===================================================  
	/// public : accessors  
	/// ===================================================  

	/// @brief メッシュパスを設定  
	/// @param path メッシュパス  
	void SetMeshPath(const std::string& path) { meshPath_ = path; }

	/// @brief テクスチャパスを設定  
	/// @param path テクスチャパス  
	void SetTexturePath(const std::string& path) { texturePath_ = path; }

	void SetMainModule(const EffectMainModule& module);
	void SetEmitShape(const EffectEmitShape& shape);

	void SetEmitType(EmitType type);

	/// @brief 最大エフェクト数を設定  
	/// @param maxCount 最大エフェクト数  
	void SetMaxEffectCount(size_t maxCount);

	/// @brief 距離でのエミットタイプを設定  
	/// @param interval エミット間隔  
	/// @param emitInstanceCount エミットごとのインスタンス数  
	void SetEmitTypeDistance(float interval, size_t emitInstanceCount);
	void SetEmitTypeDistance(const DistanceEmitData& data);

	/// @brief 時間でのエミットタイプを設定  
	/// @param data 時間エミットデータ  
	void SetEmitTypeTime(const TimeEmitData& data, size_t emitInstanceCount);
	void SetEmitTypeTime(const TimeEmitData& data);

	void SetEmitInstanceCount(size_t emitInstanceCount);

	/// @brief 残り寿命を設定  
	/// @param time 残り寿命  
	void SetLifeLeftTime(float time);

	/// @brief 要素の更新関数を設定
	/// @param func 
	void SetElementUpdateFunc(std::function<void(Element*)> func);

	/// @brief ビルボードの使用を設定
	/// @param use true: ビルボードを使用する, false: 使用しない
	void SetUseBillboard(bool use);

	/// @brief particle を出現させるかのフラグ
	/// @param isCreateParticle true: 出現できる false: 出現できない
	void SetIsCreateParticle(bool isCreateParticle);

	void SetBlendMode(BlendMode blendMode);

	void SetStartSize(const Vector3& size);
	void SetStartSize(const Vector3& size1, const Vector3& size2);

	void SetStartRotate(const Vector3& rotate);
	void SetStartRotate(const Vector3& rotate1, const Vector3& rotate2);

	void SetStartColor(const Color& color);
	void SetStartColor(const Color& color1, const Color& color2);

	void SetStartSpeed(float speed);
	void SetStartSpeed(float speed1, float speed2);

	/// @brief sphereのエミット形状を設定
	/// @param center 中心
	/// @param radius 半径
	void SetEmitShape(const Vector3& center, float radius);

	/// @brief cubeのエミット形状を設定
	/// @param center 中心 
	/// @param size cubeのサイズ
	void SetEmitShape(const Vector3& center, const Vector3& size);

	/// @brief coneのエミット形状を設定
	/// @param apex 天辺の位置
	/// @param angle coneの角度
	/// @param radius coneの半径
	/// @param height coneの高さ
	void SetEmitShape(const Vector3& apex, float angle, float radius, float height);




	bool IsCreateParticle() const;
	size_t GetMaxEffectCount() const;
	const std::string& GetMeshPath() const;
	const std::string& GetTexturePath() const;


	/// @brief エフェクト要素を取得
	/// @return エフェクト要素のリスト
	const std::vector<Element>& GetElements() const;

	BlendMode GetBlendMode() const;

	EffectMainModule* GetMainModule();
	const EffectMainModule& GetMainModule() const;

	EffectEmitShape* GetEmitShape();
	const EffectEmitShape& GetEmitShape() const;

	int GetEmitType() const;

	const DistanceEmitData& GetDistanceEmitData() const;
	const TimeEmitData& GetTimeEmitData() const;
	size_t GetEmitInstanceCount() const;

};


namespace ComponentDebug {
	/// @brief Effectのデバッグ関数
	/// @param effect デバッグするEffect
	void EffectDebug(Effect* effect);
} 

} /// ONEngine
