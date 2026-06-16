#pragma once

/// engine
#include "Engine/Asset/Assets/Texture/Texture.h"
#include "Engine/Core/Utility/Utility.h"

namespace Editor {

void ShowTexture2DPreview(const std::string& name, ONEngine::Asset::Texture* texture, const ONEngine::Vector2& textureSize, float previewFactor);

} /// namespace Editor
