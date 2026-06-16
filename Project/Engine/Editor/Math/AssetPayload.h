#pragma once

/// std
#include <algorithm>
#include <string>

/// engine
#include "Engine/Asset/Guid/Guid.h"

/// @brief Assetのペイロード (string + Guid)
namespace Editor {

struct AssetPayload {
	std::string    filePath;
	ONEngine::Guid guid;

	AssetPayload() = default;
	AssetPayload(const std::string& path, const ONEngine::Guid& guid)
		: filePath(path), guid(guid) {
	}

};

} /// Editor
