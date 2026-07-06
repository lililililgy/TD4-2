#pragma once

/// std
#include <unordered_map>
#include <memory>

/// engine
#include "../Interface/ECSISystem.h"

/// ///////////////////////////////////////////////////
/// ECSのシステムを管理するクラス
/// ///////////////////////////////////////////////////
namespace ONEngine {

class SystemCollection final {
public:
	/// ===================================================
	/// public : methods
	/// ===================================================

	SystemCollection() = default;
	~SystemCollection() = default;

	/// @brief 新規systemを追加する
	void AddSystem(std::unique_ptr<ECSISystem> system);

	/// @brief runtime外の更新処理を行う (runtime中でも処理される)
	void OutsideOfRuntimeUpdate(class ECSGroup* ecs);

	/// @brief runtime中の更新処理を行う
	void RuntimeUpdate(class ECSGroup* ecs);

	/// @brief 特定のシステムを取得する
	template<typename T>
	T* GetSystem() {
		for (auto& sys : systems_) {
			if (auto target = dynamic_cast<T*>(sys.get())) {
				return target;
			}
		}
		return nullptr;
	}

private:
	/// ===================================================
	/// private : objects
	/// ===================================================

	std::vector<std::unique_ptr<ECSISystem>> systems_;

};


} /// ONEngine
