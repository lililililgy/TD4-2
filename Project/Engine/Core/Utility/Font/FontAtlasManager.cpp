#include "FontAtlasManager.h"
#include <format>

namespace ONEngine {

FontAtlas* FontAtlasManager::GetOrCreateAtlas(const std::string& fontPath, int fontSize, int outlineWidth) {
	std::lock_guard<std::mutex> lock(mutex_);

	std::string key = std::format("{}_{}_{}", fontPath, fontSize, outlineWidth);
	auto it = atlases_.find(key);
	if (it != atlases_.end()) {
		return it->second.get();
	}

	auto atlas = std::make_unique<FontAtlas>(fontPath, fontSize, outlineWidth);
	if (atlas->Initialize()) {
		FontAtlas* ptr = atlas.get();
		atlases_[key] = std::move(atlas);
		return ptr;
	}

	return nullptr;
}

void FontAtlasManager::UpdateAllAtlases() {
	std::lock_guard<std::mutex> lock(mutex_);
	for (auto& pair : atlases_) {
		pair.second->UpdateGpuTexture();
	}
}

void FontAtlasManager::Clear() {
	std::lock_guard<std::mutex> lock(mutex_);
	atlases_.clear();
}

} // namespace ONEngine
