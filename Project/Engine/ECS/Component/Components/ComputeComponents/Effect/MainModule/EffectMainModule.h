#pragma once

/// std
#include <variant>

/// engine
#include "Engine/Core/Utility/Utility.h"

/// //////////////////////////////////////////////////
/// Effectのmainモジュール
/// //////////////////////////////////////////////////
namespace ONEngine {

class EffectMainModule final {
	friend class Effect;
	friend class EffectUpdateSystem;
public:
	/// ===================================================
	/// public : sub class
	/// ===================================================

	enum {
		Constant,
		TwoConstant,
	};


	/// @brief 定数データ
	template<typename T>
	struct ConstantData {
		ConstantData() = default;
		ConstantData(const T& data) : constant(data) {}

		T constant;
	};

	/// @brief 2つの定数データ
	template<typename T>
	struct TwoConstantData {
		TwoConstantData() = default;
		TwoConstantData(const std::pair<T, T>& data) : constant(data) {}
		TwoConstantData(const T& data1, const T& data2) : constant(std::make_pair(data1, data2)) {}

		std::pair<T, T> constant;
	};

	/// @brief 値の型
	template<typename T>
	using Value = std::variant<ConstantData<T>, TwoConstantData<T>>;

	/// @brief Valueの中の値を取得する  
	/// @tparam T 値の型  
	/// @param value Value型のデータ  
	/// @return 中の値  
	template<typename T>
	std::pair<T, T> GetValue(const Value<T>& value) const {
		return std::visit([](auto&& arg) -> std::pair<T, T> {
			using ArgType = std::decay_t<decltype(arg)>;
			if constexpr (std::is_same_v<ArgType, ConstantData<T>>) {
				return std::make_pair(arg.constant, arg.constant);
			} else if constexpr (std::is_same_v<ArgType, TwoConstantData<T>>) {
				// 2つの定数の場合、最初の値を返す  
				return arg.constant;
			}
			},
			value
		);
	}

public:
	/// ===================================================
	/// public : methods
	/// ===================================================

	EffectMainModule();
	~EffectMainModule();


private:
	/// ==================================================
	/// private : objects
	/// ==================================================

	float lifeLeftTime_ = 0.0f; ///< 残り寿命
	float startSpeed_ = 0.0f;   ///< 開始速度

	Value<float> speedStartData_;    ///< 開始速度のデータ
	Value<Vector3> sizeStartData_;   ///< 開始サイズのデータ
	Value<Vector3> rotateStartData_; ///< 開始回転のデータ
	Value<Color> colorStartData_;    ///< 開始色のデータ

	float gravityModifier_ = 0.0f; ///< 重力の影響



public:
	/// ====================================================
	/// public : accessors
	/// ====================================================

	float GetLifeLeftTime() const { return lifeLeftTime_; }
	float GetStartSpeed() const { return startSpeed_; }
	float GetGravityModifier() const { return gravityModifier_; }

	void SetLifeLeftTime(float time) { lifeLeftTime_ = time; }
	void SetStartSpeed(float speed) { startSpeed_ = speed; }
	void SetGravityModifier(float gravity) { gravityModifier_ = gravity; }

	void SetSpeedStartData(const ConstantData<float>& data) { speedStartData_ = data; }
	void SetSpeedStartData(const TwoConstantData<float>& data) { speedStartData_ = data; }

	void SetSizeStartData(const ConstantData<Vector3>& data) { sizeStartData_ = data; }
	void SetSizeStartData(const TwoConstantData<Vector3>& data) { sizeStartData_ = data; }

	void SetRotateStartData(const ConstantData<Vector3>& data) { rotateStartData_ = data; }
	void SetRotateStartData(const TwoConstantData<Vector3>& data) { rotateStartData_ = data; }

	void SetColorStartData(const ConstantData<Color>& data) { colorStartData_ = data; }
	void SetColorStartData(const TwoConstantData<Color>& data) { colorStartData_ = data; }




	std::pair<float, float> GetSpeedStartData() const {
		return GetValue(speedStartData_);
	}

	std::pair<Vector3, Vector3> GetSizeStartData() const {
		return GetValue(sizeStartData_);
	}

	std::pair<Vector3, Vector3> GetRotateStartData() const {
		return GetValue(rotateStartData_);
	}

	std::pair<Color, Color> GetColorStartData() const {
		return GetValue(colorStartData_);
	}




};

} /// ONEngine
