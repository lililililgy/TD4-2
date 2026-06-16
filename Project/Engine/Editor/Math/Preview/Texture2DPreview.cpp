#include "Texture2DPreview.h"

/// externals
#include <imgui.h>

void Editor::ShowTexture2DPreview(const std::string& name, ONEngine::Asset::Texture* texture, const ONEngine::Vector2& textureSize, float previewFactor) {
	if(name != "") {
		ImGui::Text("%s", name.c_str());
	} else {
		ImGui::Text("Texture2D Preview");
	}

	if(texture) {
		if(texture->IsStandard2D()) {
			ONEngine::Vector2 aspectRatio = textureSize;
			aspectRatio /= (std::max)(aspectRatio.x, aspectRatio.y);

			ImTextureID texId = reinterpret_cast<ImTextureID>(texture->GetSRVGPUHandle().ptr);
			ImGui::Image(texId, ImVec2(previewFactor * aspectRatio.x, previewFactor * aspectRatio.y));
		} else {
			ImGui::Text("Preview not supported\n(CubeMap or 3D Texture)");
		}
	} else {
		/// テクスチャがない場合はドラッグドロップ領域を表示する
		ImVec2 size = ImVec2(previewFactor, previewFactor);
		ImVec2 pos = ImGui::GetCursorScreenPos();
		ImDrawList* drawList = ImGui::GetWindowDrawList();

		/// InvisibleButton はクリック判定やDragDropのターゲット領域になる
		ImGui::InvisibleButton("DropArea", size);

		/// 視覚的な四角形を描く
		ImU32 imColor = IM_COL32(100, 100, 255, 100); // 半透明の青
		drawList->AddRectFilled(pos, ImVec2(pos.x + size.x, pos.y + size.y), imColor, 4.0f);

		/// 枠線
		drawList->AddRect(pos, ImVec2(pos.x + size.x, pos.y + size.y), IM_COL32(255, 255, 255, 200), 4.0f, 0, 2.0f);
	}
}
