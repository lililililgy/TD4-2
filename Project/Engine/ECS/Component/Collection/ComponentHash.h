#pragma once

/// std
#include <string>

#include "../Components/Interface/IComponent.h"


namespace ONEngine {

/// @brief componentのポインタから名前を取得する
/// @param component Componentのポインタ
/// @return クラス名
inline std::string GetComponentTypeName(const IComponent* component) {
	std::string name = typeid(*component).name();
	size_t pos = name.find_last_of(':');
	if (pos != std::string::npos) {
		name = name.substr(pos + 1);
	} else {
		// handle space if no colon (e.g. "class MyComp")
		pos = name.find_last_of(' ');
		if (pos != std::string::npos) {
			name = name.substr(pos + 1);
		}
	}
	return name;
}

/// @brief Componentの型から名前を取得する
/// @tparam T Componentの型
/// @return クラス名
template <IsComponent T>
inline std::string GetComponentTypeName() {
	std::string name = typeid(T).name();
	size_t pos = name.find_last_of(':');
	if (pos != std::string::npos) {
		name = name.substr(pos + 1);
	} else {
		pos = name.find_last_of(' ');
		if (pos != std::string::npos) {
			name = name.substr(pos + 1);
		}
	}
	return name;
}

/// @brief Componentの名前からハッシュ値を取得する
/// @param name Componentの名前
/// @return Hash値
inline size_t GetComponentHash(const std::string& name) {
	return std::hash<std::string>()(name);
}

/// @brief Componentの型からハッシュ値を取得する
/// @tparam T Componentの型
/// @return Hash値
template <IsComponent T>
inline size_t GetComponentHash() {
	return GetComponentHash(GetComponentTypeName<T>());
}

} /// ONEngine