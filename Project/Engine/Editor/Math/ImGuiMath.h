#pragma once

/// std
#include <string>
#include <vector>

/// external
#include <imgui.h>
#include <magic_enum/magic_enum.hpp>

/// engine
#include "Engine/Core/Utility/Math/Vector3.h"
#include "Engine/Core/Utility/Math/Vector4.h"
#include "Engine/Graphics/Buffer/Data/GPUMaterial.h"

namespace ONEngine::Asset {
class AssetCollection;
}

/// 前方宣言
namespace Editor {

/// ////////////////////////////////////////////////////////
/// ImGui用のMath関数群
/// ////////////////////////////////////////////////////////
namespace ImMathf {

/// @brief Vector4 -> ImVec4 変換
/// @param vec 自作のVector4
/// @return 変換されたImVec4
ImVec4 ToImVec4(const ONEngine::Vector4& vec);

/// @brief Vector2 -> ImVec2 変換
/// @param vec 自作のVector2
/// @return 変換されたImVec2
ImVec2 ToImVec2(const ONEngine::Vector2& vec);

/// 色の編集
bool ColorEdit(const char* label, ONEngine::Vector4* color, ImGuiColorEditFlags flags = 0);

/// テキストの入力
bool InputText(const char* label, std::string* text, ImGuiInputTextFlags flags = 0);

/// 数値の入力
bool InputFloat(const char* label, float* v, float step = 0.0f, float step_fast = 0.0f, const char* format = "%.3f", ImGuiInputTextFlags flags = 0);

/// 数値のドラッグ入力
bool DragFloat(const char* label, float* v, float speed = 0.1f, float min = 0.0f, float max = 0.0f, const char* format = "%.3f", ImGuiInputTextFlags flags = 0);

/// Vector3のドラッグ入力
bool DragFloat3(const char* label, ONEngine::Vector3* v, float speed = 0.1f, float min = 0.0f, float max = 0.0f, const char* format = "%.3f", ImGuiInputTextFlags flags = 0);

/// ドラッグ中に画面端でカーソルをループさせる処理
void LoopCursorIfDragging();

/// Enumの入力
template <typename T>
bool InputEnum(const char* label, T* value) {
    auto names = magic_enum::enum_names<T>();
    int current_item = static_cast<int>(magic_enum::enum_index(*value).value_or(0));
    
    std::vector<const char*> item_ptrs;
    for (const auto& name : names) {
        item_ptrs.push_back(name.data());
    }
    
    if (ImGui::Combo(label, &current_item, item_ptrs.data(), static_cast<int>(item_ptrs.size()))) {
        *value = magic_enum::enum_value<T>(current_item);
        return true;
    }
    return false;
}

/// マテリアルの編集
bool MaterialEdit(const char* label, ONEngine::GPUMaterial* material, ONEngine::Asset::AssetCollection* assetCollection);

/// UV変形の編集
bool UVTransformEdit(const char* label, ONEngine::UVTransform* uvTransform);


ImVec2 CalculateAspectFitSize(const ONEngine::Vector2& textureSize, float maxSize);
ImVec2 CalculateAspectFitSize(const ONEngine::Vector2& textureSize, const ImVec2& maxSize);
} /// ImMathf

/// -----------------------------------------------
/// まだImMathfに移動していない関数
/// -----------------------------------------------

bool ImGuiInputText(const char* label, std::string* text, ImGuiInputTextFlags flags = 0);
bool ImGuiInputText(const char* label, std::string* text, ImGuiInputTextFlags flags, const char* hint);

void ImGuiInputTextReadOnly(const char* label, const std::string& text);

bool ImGuiColorEdit(const char* label, ONEngine::Vector4* color);


} /// Editor


namespace ONEngine {
/// //////////////////////////////////////////////
/// componentのデバッグ表示関数を定義 (今後各Componentの.h .cppに移動予定)
/// //////////////////////////////////////////////

void AudioSourceDebug(class AudioSource* audioSource);

void CustomMeshRendererDebug(class CustomMeshRenderer* customMeshRenderer);

void EffectDebug(class Effect* effect);
void ParticleSystemDebug(class ParticleSystem* ps);
void ParticleSystem2DDebug(class ParticleSystem2D* ps);

/// Unity-like modular header with checkbox
bool BeginModuleHeader(const char* label, bool* enabled);
void EndModuleHeader();

} /// ONEngine