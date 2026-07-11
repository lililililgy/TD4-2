#include "Engine/ECS/Component/Components/ComputeComponents/ParticleSystem/ParticleSystem.h"
#include "Engine/ECS/Component/Components/ComputeComponents/ParticleSystem2D/ParticleSystem2D.h"
#include "ImGuiMath.h"

#define NOMINMAX

/// std
#include <numbers>
#include <format>
#include <variant>
#include <algorithm>
#include <cstdio> 
#include <cmath> 
#include <unordered_map>

/// external
#include <imgui_internal.h> // PushMultiItemsWidths に必要
#include <Externals/imgui/dialog/ImGuiFileDialog.h>
#include <ImCurveEdit.h>

/// win32 (カーソルループ用)
#include <Windows.h>

/// engine
#include "Engine/Asset/Collection/AssetCollection.h"
#include "Engine/ECS/EntityComponentSystem/EntityComponentSystem.h"
#include "Engine/ECS/Component/Components/ComputeComponents/Light/Light.h"
#include "Engine/ECS/Component/Components/ComputeComponents/Effect/Effect.h"
#include "Engine/ECS/Component/Components/RendererComponents/Mesh/MeshRenderer.h"
#include "Engine/ECS/Component/Components/RendererComponents/Mesh/CustomMeshRenderer.h"

/// editor
#include "Engine/Editor/Manager/EditCommand.h"
#include "Engine/Editor/Commands/ImGuiCommand/ImGuiCommand.h" 
#include "Engine/Editor/Math/AssetPayload.h"
#include "Engine/Editor/Commands/LambdaCommand.h"

using namespace Editor;
using namespace ONEngine;

namespace {

float rotateSpeed = 3.14159f / 100.0f;
std::string variableName = "";

// --- Helpers for ParticleSystem ---

void DrawMinMaxFloat(const char* label, ONEngine::MinMaxFloat& val) {
    ImGui::PushID(label);
    
    ImGui::TextUnformatted(label);
    ImGui::SameLine(ImGui::GetWindowWidth() * 0.4f);
    
    float availW = ImGui::GetContentRegionAvail().x;
    float buttonW = 24.0f;
    float fieldW = availW - buttonW - ImGui::GetStyle().ItemSpacing.x;
    
    if (val.state == ONEngine::MinMaxState::Constant) {
        ImGui::SetNextItemWidth(fieldW);
        ImGui::DragFloat("##constant", &val.constant, 0.1f);
    } else if (val.state == ONEngine::MinMaxState::RandomBetweenTwoConstants) {
        float halfW = (fieldW - ImGui::GetStyle().ItemSpacing.x * 2.0f - ImGui::CalcTextSize("-").x) * 0.5f;
        ImGui::SetNextItemWidth(halfW);
        ImGui::DragFloat("##min", &val.minVal, 0.1f);
        ImGui::SameLine();
        ImGui::Text("-");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(halfW);
        ImGui::DragFloat("##max", &val.maxVal, 0.1f);
    }

    ImGui::SameLine();
    if (ImGui::Button("v", ImVec2(buttonW, 0))) {
        ImGui::OpenPopup("MinMaxPopup");
    }

    if (ImGui::BeginPopup("MinMaxPopup")) {
        if (ImGui::MenuItem("Constant", nullptr, val.state == ONEngine::MinMaxState::Constant)) val.state = ONEngine::MinMaxState::Constant;
        if (ImGui::MenuItem("Random Between Two Constants", nullptr, val.state == ONEngine::MinMaxState::RandomBetweenTwoConstants)) val.state = ONEngine::MinMaxState::RandomBetweenTwoConstants;
        ImGui::EndPopup();
    }

    ImGui::PopID();
}

void DrawMinMaxColor(const char* label, ONEngine::MinMaxColor& val) {
    ImGui::PushID(label);
    ImGui::TextUnformatted(label);
    ImGui::SameLine(ImGui::GetWindowWidth() * 0.4f);

    float availW = ImGui::GetContentRegionAvail().x;
    float buttonW = 24.0f;
    float fieldW = availW - buttonW - ImGui::GetStyle().ItemSpacing.x;

    if (val.state == ONEngine::MinMaxState::Constant) {
        ONEngine::Vector4 editColor;
        editColor.x = val.constant.r;
        editColor.y = val.constant.g;
        editColor.z = val.constant.b;
        editColor.w = val.constant.a;

        ImGui::SetNextItemWidth(fieldW);
        if (ImGui::ColorEdit4("##constant", &editColor.x)) {
            val.constant.r = editColor.x;
            val.constant.g = editColor.y;
            val.constant.b = editColor.z;
            val.constant.a = editColor.w;
        }
    } else if (val.state == ONEngine::MinMaxState::RandomBetweenTwoConstants) {
        ONEngine::Vector4 cmin;
        cmin.x = val.minVal.r; cmin.y = val.minVal.g; cmin.z = val.minVal.b; cmin.w = val.minVal.a;
        ONEngine::Vector4 cmax;
        cmax.x = val.maxVal.r; cmax.y = val.maxVal.g; cmax.z = val.maxVal.b; cmax.w = val.maxVal.a;
        
        // 2つ並べるときはNoInputsで小さくする
        if (ImGui::ColorEdit4("##min", &cmin.x, ImGuiColorEditFlags_NoInputs)) {
            val.minVal.r = cmin.x;
            val.minVal.g = cmin.y;
            val.minVal.b = cmin.z;
            val.minVal.a = cmin.w;
        }
        ImGui::SameLine();
        ImGui::Text("-");
        ImGui::SameLine();
        if (ImGui::ColorEdit4("##max", &cmax.x, ImGuiColorEditFlags_NoInputs)) {
            val.maxVal.r = cmax.x;
            val.maxVal.g = cmax.y;
            val.maxVal.b = cmax.z;
            val.maxVal.a = cmax.w;
        }
    }

    ImGui::SameLine();
    if (ImGui::Button("v", ImVec2(buttonW, 0))) {
        ImGui::OpenPopup("MinMaxPopup");
    }
    if (ImGui::BeginPopup("MinMaxPopup")) {
        if (ImGui::MenuItem("Constant", nullptr, val.state == ONEngine::MinMaxState::Constant)) val.state = ONEngine::MinMaxState::Constant;
        if (ImGui::MenuItem("Random Between Two Constants", nullptr, val.state == ONEngine::MinMaxState::RandomBetweenTwoConstants)) val.state = ONEngine::MinMaxState::RandomBetweenTwoConstants;
        ImGui::EndPopup();
    }

    ImGui::PopID();
}

// ImCurveEdit Delegate for ParticleSystemGradient (Red, Green, Blue, Alpha)
class GradientCurveDelegate : public ImCurveEdit::Delegate {
public:
    GradientCurveDelegate(ONEngine::ParticleSystemGradient& gradient, float minY, float maxY)
        : gradient_(gradient) {
        min_ = ImVec2(0.0f, minY);
        max_ = ImVec2(1.0f, maxY);
        SyncFromGradient();
    }

    void SyncFromGradient() {
        // Sort keys by time
        std::sort(gradient_.colorKeys.begin(), gradient_.colorKeys.end(), [](const auto& a, const auto& b) { return a.time < b.time; });
        std::sort(gradient_.alphaKeys.begin(), gradient_.alphaKeys.end(), [](const auto& a, const auto& b) { return a.time < b.time; });

        // RGB
        size_t colorCount = gradient_.colorKeys.size();
        points_[0].resize(colorCount);
        points_[1].resize(colorCount);
        points_[2].resize(colorCount);
        for (size_t i = 0; i < colorCount; i++) {
            float t = std::clamp(gradient_.colorKeys[i].time, 0.0f, 1.0f);
            points_[0][i] = ImVec2(t, gradient_.colorKeys[i].color.r);
            points_[1][i] = ImVec2(t, gradient_.colorKeys[i].color.g);
            points_[2][i] = ImVec2(t, gradient_.colorKeys[i].color.b);
        }

        // Alpha
        size_t alphaCount = gradient_.alphaKeys.size();
        points_[3].resize(alphaCount);
        for (size_t i = 0; i < alphaCount; i++) {
            float t = std::clamp(gradient_.alphaKeys[i].time, 0.0f, 1.0f);
            points_[3][i] = ImVec2(t, gradient_.alphaKeys[i].alpha);
        }
    }

    void SyncToGradient() {
        // RGB
        size_t colorCount = points_[0].size();
        gradient_.colorKeys.resize(colorCount);
        for (size_t i = 0; i < colorCount; i++) {
            gradient_.colorKeys[i].time = points_[0][i].x;
            gradient_.colorKeys[i].color.r = std::clamp(points_[0][i].y, 0.0f, 1.0f);
            gradient_.colorKeys[i].color.g = std::clamp(points_[1][i].y, 0.0f, 1.0f);
            gradient_.colorKeys[i].color.b = std::clamp(points_[2][i].y, 0.0f, 1.0f);
        }

        // Alpha
        size_t alphaCount = points_[3].size();
        gradient_.alphaKeys.resize(alphaCount);
        for (size_t i = 0; i < alphaCount; i++) {
            gradient_.alphaKeys[i].time = points_[3][i].x;
            gradient_.alphaKeys[i].alpha = std::clamp(points_[3][i].y, 0.0f, 1.0f);
        }
    }

    size_t GetCurveCount() override { return 4; }
    bool IsVisible(size_t) override { return true; }
    ImCurveEdit::CurveType GetCurveType(size_t) const override { return ImCurveEdit::CurveBezier; }
    ImVec2& GetMin() override { return min_; }
    ImVec2& GetMax() override { return max_; }

    size_t GetPointCount(size_t curveIndex) override {
        return points_[curveIndex].size();
    }

    uint32_t GetCurveColor(size_t curveIndex) override {
        if (curveIndex == 0) return 0xFF0000FF; // Red
        if (curveIndex == 1) return 0xFF00FF00; // Green
        if (curveIndex == 2) return 0xFFFF0000; // Blue
        return 0xFFFFFFFF; // White for Alpha
    }

    ImVec2* GetPoints(size_t curveIndex) override {
        return points_[curveIndex].empty() ? nullptr : points_[curveIndex].data();
    }

    int EditPoint(size_t curveIndex, int pointIndex, ImVec2 value) override {
        if (pointIndex < 0 || pointIndex >= (int)points_[curveIndex].size()) return pointIndex;

        value.x = std::clamp(value.x, 0.0f, 1.0f);

        if (curveIndex < 3) {
            for (int c = 0; c < 3; c++) {
                points_[c][pointIndex].x = value.x;
            }
            points_[curveIndex][pointIndex].y = value.y;
        } else {
            points_[curveIndex][pointIndex] = value;
        }

        SyncToGradient();
        return pointIndex;
    }

    void AddPoint(size_t curveIndex, ImVec2 value) override {
        value.x = std::clamp(value.x, 0.0f, 1.0f);
        if (curveIndex < 3) {
            ONEngine::GradientColorKey key;
            key.time = value.x;
            Color current = gradient_.Evaluate(value.x);
            key.color.r = current.r;
            key.color.g = current.g;
            key.color.b = current.b;
            if (curveIndex == 0) key.color.r = value.y;
            if (curveIndex == 1) key.color.g = value.y;
            if (curveIndex == 2) key.color.b = value.y;
            gradient_.colorKeys.push_back(key);
        } else {
            ONEngine::GradientAlphaKey key;
            key.time = value.x;
            key.alpha = value.y;
            gradient_.alphaKeys.push_back(key);
        }
        SyncFromGradient();
        SyncToGradient();
    }

    float GetCurveValue(size_t curveIndex, float time) override {
        Color c = gradient_.Evaluate(time);
        if (curveIndex == 0) return c.r;
        if (curveIndex == 1) return c.g;
        if (curveIndex == 2) return c.b;
        return c.a;
    }

private:
    ONEngine::ParticleSystemGradient& gradient_;
    std::vector<ImVec2> points_[4];
    ImVec2 min_;
    ImVec2 max_;
};

void DrawGradient(const char* label, ONEngine::ParticleSystemGradient& gradient) {
    ImGui::PushID(label);
    if (ImGui::TreeNode(label)) {
        float minY = 0.0f;
        float maxY = 1.0f;

        float availW = ImGui::GetContentRegionAvail().x;

        // ビジュアルグラデーションエディタ
        GradientCurveDelegate delegate(gradient, minY, maxY);
        float editorWidth = std::max(availW, 100.0f);

        static ImVector<ImCurveEdit::EditPoint> selectedPoints;
        selectedPoints.clear();

        ImCurveEdit::Edit(delegate, ImVec2(editorWidth, 120), (unsigned int)ImGui::GetID("##gradientEdit"), NULL, &selectedPoints);

        // 選択された制御点の接線（ハンドル）および値の編集UI
        if (!selectedPoints.empty()) {
            int curveIdx = selectedPoints[0].curveIndex;
            int pointIdx = selectedPoints[0].pointIndex;
            if (curveIdx >= 0 && curveIdx < 4) {
                if (curveIdx < 3) {
                    if (pointIdx >= 0 && pointIdx < (int)gradient.colorKeys.size()) {
                        ImGui::Text("Selected Color Key #%d (Time: %.2f)", pointIdx, gradient.colorKeys[pointIdx].time);
                        ImGui::SetNextItemWidth(availW * 0.4f);
                        Editor::ImMathf::DragFloat("In Tangent", &gradient.colorKeys[pointIdx].inTangent, 0.05f);
                        ImGui::SameLine();
                        ImGui::SetNextItemWidth(availW * 0.4f);
                        Editor::ImMathf::DragFloat("Out Tangent", &gradient.colorKeys[pointIdx].outTangent, 0.05f);

                        ONEngine::Vector4 col = { gradient.colorKeys[pointIdx].color.r, gradient.colorKeys[pointIdx].color.g, gradient.colorKeys[pointIdx].color.b, 1.0f };
                        if (ImGui::ColorEdit3("Color Value", &col.x)) {
                            gradient.colorKeys[pointIdx].color.r = col.x;
                            gradient.colorKeys[pointIdx].color.g = col.y;
                            gradient.colorKeys[pointIdx].color.b = col.z;
                        }
                    }
                } else {
                    if (pointIdx >= 0 && pointIdx < (int)gradient.alphaKeys.size()) {
                        ImGui::Text("Selected Alpha Key #%d (Time: %.2f)", pointIdx, gradient.alphaKeys[pointIdx].time);
                        ImGui::SetNextItemWidth(availW * 0.4f);
                        Editor::ImMathf::DragFloat("In Tangent", &gradient.alphaKeys[pointIdx].inTangent, 0.05f);
                        ImGui::SameLine();
                        ImGui::SetNextItemWidth(availW * 0.4f);
                        Editor::ImMathf::DragFloat("Out Tangent", &gradient.alphaKeys[pointIdx].outTangent, 0.05f);

                        ImGui::SetNextItemWidth(availW * 0.4f);
                        Editor::ImMathf::DragFloat("Alpha Value", &gradient.alphaKeys[pointIdx].alpha, 0.01f, 0.0f, 1.0f);
                    }
                }
            }
        }

        // キーの手動追加・削除
        if (ImGui::Button("+ Color Key")) gradient.colorKeys.push_back({ Color::kWhite, 1.0f });
        for (size_t i = 0; i < gradient.colorKeys.size(); ++i) {
            ImGui::PushID((int)i);
            ONEngine::Vector4 col = { gradient.colorKeys[i].color.r, gradient.colorKeys[i].color.g, gradient.colorKeys[i].color.b, 1.0f };
            if (ImGui::ColorEdit3("##col", &col.x, ImGuiColorEditFlags_NoInputs)) {
                gradient.colorKeys[i].color.r = col.x; gradient.colorKeys[i].color.g = col.y; gradient.colorKeys[i].color.b = col.z;
            }
            ImGui::SameLine();
            ImGui::SetNextItemWidth(availW * 0.35f);
            if (ImGui::DragFloat("##time", &gradient.colorKeys[i].time, 0.01f, 0.0f, 1.0f)) {
                gradient.colorKeys[i].time = std::clamp(gradient.colorKeys[i].time, 0.0f, 1.0f);
            }
            ImGui::SameLine(); if (ImGui::Button("x")) { gradient.colorKeys.erase(gradient.colorKeys.begin() + i); ImGui::PopID(); break; }
            ImGui::PopID();
        }
        if (ImGui::Button("+ Alpha Key")) gradient.alphaKeys.push_back({ 1.0f, 1.0f });
        for (size_t i = 0; i < gradient.alphaKeys.size(); ++i) {
            ImGui::PushID((int)i + 1000);
            ImGui::SetNextItemWidth(availW * 0.3f);
            if (ImGui::DragFloat("##alpha", &gradient.alphaKeys[i].alpha, 0.01f, 0.0f, 1.0f)) {
                gradient.alphaKeys[i].alpha = std::clamp(gradient.alphaKeys[i].alpha, 0.0f, 1.0f);
            }
            ImGui::SameLine();
            ImGui::SetNextItemWidth(availW * 0.3f);
            if (ImGui::DragFloat("##time", &gradient.alphaKeys[i].time, 0.01f, 0.0f, 1.0f)) {
                gradient.alphaKeys[i].time = std::clamp(gradient.alphaKeys[i].time, 0.0f, 1.0f);
            }
            ImGui::SameLine(); if (ImGui::Button("x")) { gradient.alphaKeys.erase(gradient.alphaKeys.begin() + i); ImGui::PopID(); break; }
            ImGui::PopID();
        }
        ImGui::TreePop();
    }
    ImGui::PopID();
}

void DrawMinMaxGradient(const char* label, ONEngine::MinMaxGradient& val) {
    ImGui::PushID(label);
    ImGui::TextUnformatted(label);
    ImGui::SameLine(ImGui::GetWindowWidth() * 0.4f);

    if (val.state == ONEngine::MinMaxState::Constant || val.state == ONEngine::MinMaxState::Curve) {
        DrawGradient("Gradient", val.gradient);
    } else {
        DrawGradient("Min Gradient", val.gradientMin);
        DrawGradient("Max Gradient", val.gradientMax);
    }

    ImGui::SameLine();
    if (ImGui::Button("v", ImVec2(20, 0))) ImGui::OpenPopup("MinMaxPopup");
    if (ImGui::BeginPopup("MinMaxPopup")) {
        if (ImGui::MenuItem("Gradient", nullptr, val.state == ONEngine::MinMaxState::Constant)) val.state = ONEngine::MinMaxState::Constant;
        if (ImGui::MenuItem("Random Between Two Gradients", nullptr, val.state == ONEngine::MinMaxState::RandomBetweenTwoCurves)) val.state = ONEngine::MinMaxState::RandomBetweenTwoCurves;
        ImGui::EndPopup();
    }
    ImGui::PopID();
}

// ImCurveEdit Delegate for AnimationCurve
class AnimationCurveDelegate : public ImCurveEdit::Delegate {
public:
    AnimationCurveDelegate(ONEngine::AnimationCurve& curve, float minY, float maxY) : curve_(curve) {
        min_ = ImVec2(0.0f, minY);
        max_ = ImVec2(1.0f, maxY);
        SyncFromCurve();
    }

    void SyncFromCurve() {
        points_.resize(curve_.keys.size());
        for (size_t i = 0; i < curve_.keys.size(); i++) {
            points_[i] = ImVec2(std::clamp(curve_.keys[i].time, 0.0f, 1.0f), curve_.keys[i].value);
        }
    }

    void SyncToCurve() {
        curve_.keys.resize(points_.size());
        for (size_t i = 0; i < points_.size(); i++) {
            curve_.keys[i].time = std::clamp(points_[i].x, 0.0f, 1.0f);
            curve_.keys[i].value = points_[i].y;
        }
    }

    size_t GetCurveCount() override { return 1; }
    bool IsVisible(size_t) override { return true; }
    ImCurveEdit::CurveType GetCurveType(size_t) const override { return ImCurveEdit::CurveBezier; }
    ImVec2& GetMin() override { return min_; }
    ImVec2& GetMax() override { return max_; }
    size_t GetPointCount(size_t) override { return points_.size(); }
    uint32_t GetCurveColor(size_t) override { return 0xFF40FF40; } // Green curve
    ImVec2* GetPoints(size_t) override { return points_.empty() ? nullptr : points_.data(); }

    int EditPoint(size_t, int pointIndex, ImVec2 value) override {
        if (pointIndex >= 0 && pointIndex < (int)points_.size()) {
            value.x = std::clamp(value.x, 0.0f, 1.0f);
            points_[pointIndex] = value;
            SyncToCurve();
        }
        return pointIndex;
    }

    void AddPoint(size_t, ImVec2 value) override {
        value.x = std::clamp(value.x, 0.0f, 1.0f);
        // Insert sorted by time
        size_t insertPos = points_.size();
        for (size_t i = 0; i < points_.size(); i++) {
            if (value.x < points_[i].x) {
                insertPos = i;
                break;
            }
        }
        points_.insert(points_.begin() + insertPos, value);
        SyncToCurve();
    }

    float GetCurveValue(size_t, float time) override {
        return curve_.Evaluate(time);
    }

private:
    ONEngine::AnimationCurve& curve_;
    std::vector<ImVec2> points_;
    ImVec2 min_;
    ImVec2 max_;
};

// カーブエディタ: Y軸範囲を保持するための静的マップ
static std::unordered_map<ImGuiID, ImVec2> sCurveYRanges;

void DrawCurve(const char* label, ONEngine::AnimationCurve& curve) {
    ImGui::PushID(label);
    if (ImGui::TreeNode(label)) {
        // Y軸範囲の取得・初期化
        ImGuiID curveId = ImGui::GetID("##curveRange");
        auto it = sCurveYRanges.find(curveId);
        if (it == sCurveYRanges.end()) {
            sCurveYRanges[curveId] = ImVec2(0.0f, 1.0f);
            it = sCurveYRanges.find(curveId);
        }
        float& minY = it->second.x;
        float& maxY = it->second.y;

        // Y軸範囲の調整UI
        float availW = ImGui::GetContentRegionAvail().x;
        ImGui::SetNextItemWidth(availW * 0.35f);
        ImGui::DragFloat("Min Y", &minY, 0.01f);
        ImGui::SameLine();
        ImGui::SetNextItemWidth(availW * 0.35f);
        ImGui::DragFloat("Max Y", &maxY, 0.01f);

        if (maxY <= minY) maxY = minY + 0.1f;

        // ビジュアルカーブエディタ
        AnimationCurveDelegate delegate(curve, minY, maxY);
        float editorWidth = std::max(availW, 100.0f);
        
        static ImVector<ImCurveEdit::EditPoint> selectedPoints;
        selectedPoints.clear();
        
        ImCurveEdit::Edit(delegate, ImVec2(editorWidth, 120), (unsigned int)ImGui::GetID("##curveEdit"), NULL, &selectedPoints);

        // 選択された制御点の接線（ハンドル）を編集するUI
        if (!selectedPoints.empty()) {
            int selectedIdx = selectedPoints[0].pointIndex;
            if (selectedIdx >= 0 && selectedIdx < (int)curve.keys.size()) {
                ImGui::Text("Selected Key #%d (Time: %.2f)", selectedIdx, curve.keys[selectedIdx].time);
                
                ImGui::SetNextItemWidth(availW * 0.4f);
                Editor::ImMathf::DragFloat("In Tangent", &curve.keys[selectedIdx].inTangent, 0.05f);
                ImGui::SameLine();
                ImGui::SetNextItemWidth(availW * 0.4f);
                Editor::ImMathf::DragFloat("Out Tangent", &curve.keys[selectedIdx].outTangent, 0.05f);
            }
        }

        // キーの手動追加・削除
        if (ImGui::Button("+ Key")) curve.keys.push_back({ 1.0f, 1.0f });
        ImGui::SameLine();
        if (ImGui::Button("Clear")) curve.keys.clear();

        // キー一覧（折りたたみ）
        if (ImGui::TreeNode("Keys")) {
            for (size_t i = 0; i < curve.keys.size(); ++i) {
                ImGui::PushID((int)i);
                ImGui::SetNextItemWidth(availW * 0.3f);
                if (ImGui::DragFloat("##time", &curve.keys[i].time, 0.01f, 0.0f, 1.0f)) {
                    curve.keys[i].time = std::clamp(curve.keys[i].time, 0.0f, 1.0f);
                }
                ImGui::SameLine();
                ImGui::SetNextItemWidth(availW * 0.3f);
                ImGui::DragFloat("##value", &curve.keys[i].value, 0.01f);
                ImGui::SameLine();
                if (ImGui::Button("x")) { curve.keys.erase(curve.keys.begin() + i); ImGui::PopID(); break; }
                ImGui::PopID();
            }
            ImGui::TreePop();
        }
        ImGui::TreePop();
    }
    ImGui::PopID();
}

void DrawMinMaxCurve(const char* label, ONEngine::MinMaxCurve& val) {
    ImGui::PushID(label);
    ImGui::TextUnformatted(label);
    ImGui::SameLine(ImGui::GetWindowWidth() * 0.4f);

    float availW = ImGui::GetContentRegionAvail().x;
    float buttonW = 24.0f;
    float fieldW = availW - buttonW - ImGui::GetStyle().ItemSpacing.x;

    if (val.state == ONEngine::MinMaxState::Constant) {
        ImGui::SetNextItemWidth(fieldW);
        ImGui::DragFloat("##constant", &val.constant, 0.1f);
    } else if (val.state == ONEngine::MinMaxState::Curve) {
        DrawCurve("Curve", val.curve);
    } else {
        DrawCurve("Min Curve", val.curveMin);
        DrawCurve("Max Curve", val.curveMax);
    }

    ImGui::SameLine();
    if (ImGui::Button("v", ImVec2(buttonW, 0))) ImGui::OpenPopup("MinMaxPopup");
    if (ImGui::BeginPopup("MinMaxPopup")) {
        if (ImGui::MenuItem("Constant", nullptr, val.state == ONEngine::MinMaxState::Constant)) val.state = ONEngine::MinMaxState::Constant;
        if (ImGui::MenuItem("Curve", nullptr, val.state == ONEngine::MinMaxState::Curve)) val.state = ONEngine::MinMaxState::Curve;
        if (ImGui::MenuItem("Random Between Two Curves", nullptr, val.state == ONEngine::MinMaxState::RandomBetweenTwoCurves)) val.state = ONEngine::MinMaxState::RandomBetweenTwoCurves;
        ImGui::EndPopup();
    }
    ImGui::PopID();
}

void DrawAssetGuidField(const char* label, std::string& guidStr, ONEngine::Asset::AssetType targetType) {
    ImGui::PushID(label);
    ImGui::TextUnformatted(label);
    ImGui::SameLine(ImGui::GetWindowWidth() * 0.4f);

    float itemWidth = ImGui::GetContentRegionAvail().x;
    
    std::string displayStr = guidStr;
    if (displayStr.empty()) displayStr = "(None)";
    else {
        auto* assetCollection = Asset::AssetCollection::GetInstance();
        if (assetCollection) {
            Guid guid = Guid::FromString(guidStr);
            if (targetType == Asset::AssetType::Texture) displayStr = assetCollection->GetAssetPath<Asset::Texture>(guid);
            else if (targetType == Asset::AssetType::Mesh) displayStr = assetCollection->GetAssetPath<Asset::Model>(guid);
            else if (targetType == Asset::AssetType::Material) {
                displayStr = assetCollection->GetAssetPath<Asset::Material>(guid);
                if (displayStr.empty()) displayStr = assetCollection->GetAssetPath<Asset::Texture>(guid); // Fallback to texture path
            }
        }
    }

    ImGui::SetNextItemWidth(itemWidth);
    if (ImGui::Selectable(displayStr.c_str(), false, ImGuiSelectableFlags_None, ImVec2(itemWidth, 0))) {
    }

    if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("AssetData")) {
            AssetPayload* assetPayload = *static_cast<AssetPayload**>(payload->Data);
            Asset::AssetType droppedType = ONEngine::Asset::GetAssetTypeFromExtension(ONEngine::FileSystem::FileExtension(assetPayload->filePath));
            
            bool isCompatible = (droppedType == targetType);
            if (targetType == Asset::AssetType::Material && droppedType == Asset::AssetType::Texture) isCompatible = true;
            if (targetType == Asset::AssetType::Mesh && droppedType == Asset::AssetType::Mesh) isCompatible = true; // For mesh, .obj/.gltf are Mesh type
            
            if (isCompatible) {
                std::string oldGuidStr = guidStr;
                std::string newGuidStr = assetPayload->guid.ToString();
                std::string* pGuidStr = &guidStr;
                EditCommand::Execute<LambdaCommand>(
                    [pGuidStr, newGuidStr]() { *pGuidStr = newGuidStr; },
                    [pGuidStr, oldGuidStr]() { *pGuidStr = oldGuidStr; }
                );
            }
        }
        ImGui::EndDragDropTarget();
    }
    ImGui::PopID();
}

}	/// unnamed namespace

ImVec4 ImMathf::ToImVec4(const ONEngine::Vector4& vec) {
	return ImVec4(vec.x, vec.y, vec.z, vec.w);
}

ImVec2 ImMathf::ToImVec2(const ONEngine::Vector2& vec) {
	return ImVec2(vec.x, vec.y);
}

bool ImMathf::ColorEdit(const char* label, ONEngine::Vector4* color, ImGuiColorEditFlags flags) {
	if(!color) return false;
	return ImGui::ColorEdit4(label, &color->x, flags);
}

bool ImMathf::InputText(const char* label, std::string* text, ImGuiInputTextFlags flags) {
	if(!text) return false;
	return Editor::ImGuiInputText(label, text, flags);
}

bool ImMathf::InputFloat(const char* label, float* v, float step, float step_fast, const char* format, ImGuiInputTextFlags flags) {
	return ImGui::InputFloat(label, v, step, step_fast, format, flags);
}

void ImMathf::LoopCursorIfDragging() {
	if (!ImGui::IsMouseDragging(ImGuiMouseButton_Left)) return;

	POINT cursorPos;
	::GetCursorPos(&cursorPos);

	HMONITOR hMonitor = ::MonitorFromPoint(cursorPos, MONITOR_DEFAULTTONEAREST);
	MONITORINFO monitorInfo{};
	monitorInfo.cbSize = sizeof(MONITORINFO);
	if (!::GetMonitorInfoA(hMonitor, &monitorInfo)) return;

	RECT rc = monitorInfo.rcMonitor;
	const int margin = 5;
	bool wrapped = false;
	POINT newPos = cursorPos;

	if (cursorPos.x <= rc.left + margin) {
		newPos.x = rc.right - margin - 1;
		wrapped = true;
	} else if (cursorPos.x >= rc.right - margin) {
		newPos.x = rc.left + margin + 1;
		wrapped = true;
	}
	if (cursorPos.y <= rc.top + margin) {
		newPos.y = rc.bottom - margin - 1;
		wrapped = true;
	} else if (cursorPos.y >= rc.bottom - margin) {
		newPos.y = rc.top + margin + 1;
		wrapped = true;
	}

	if (wrapped) {
		::SetCursorPos(newPos.x, newPos.y);
		// ImGuiのマウス座標を補正してデルタの跳びを防ぐ
		ImGuiIO& io = ImGui::GetIO();
		ImVec2 delta = ImVec2(
			(float)(newPos.x - cursorPos.x),
			(float)(newPos.y - cursorPos.y)
		);
		io.MousePos.x += delta.x;
		io.MousePosPrev.x += delta.x;
		io.MousePos.y += delta.y;
		io.MousePosPrev.y += delta.y;
	}
}

bool ImMathf::DragFloat(const char* label, float* v, float speed, float min, float max, const char* format, ImGuiInputTextFlags flags) {
	bool result = ImGui::DragFloat(label, v, speed, min, max, format, flags);
	if (ImGui::IsItemActive()) LoopCursorIfDragging();
	return result;
}

bool ImMathf::DragFloat3(const char* label, ONEngine::Vector3* v, float speed, float min, float max, const char* format, ImGuiInputTextFlags flags) {
	bool result = ImGui::DragFloat3(label, &v->x, speed, min, max, format, flags);
	if (ImGui::IsItemActive()) LoopCursorIfDragging();
	return result;
}

bool ImMathf::MaterialEdit(const char* label, ONEngine::GPUMaterial* material, ONEngine::Asset::AssetCollection* assetCollection) {
	if(!material) return false;
	bool isEdit = false;
	if(ImGui::CollapsingHeader(label)) {
		if(ImGuiColorEdit("BaseColor", &material->baseColor)) isEdit = true;
		if(UVTransformEdit("UVTransform", &material->uvTransform)) isEdit = true;
		if(ImGui::CollapsingHeader("PostEffectFlags")) {
			if(ImGui::CheckboxFlags("Lighting", &material->postEffectFlags, PostEffectFlags_Lighting)) isEdit = true;
			if(ImGui::CheckboxFlags("Grayscale", &material->postEffectFlags, PostEffectFlags_Grayscale)) isEdit = true;
			if(ImGui::CheckboxFlags("EnvironmentReflection", &material->postEffectFlags, PostEffectFlags_EnvironmentReflection)) isEdit = true;
			if(ImGui::CheckboxFlags("Shadow", &material->postEffectFlags, PostEffectFlags_Shadow)) isEdit = true;
		}
		if(ImGui::CollapsingHeader("Texture")) {
			const std::string& texturePath = assetCollection->GetTexturePath(material->baseTextureId);
			std::string tempPath = texturePath;
			if(ImMathf::InputText("Base Texture", &tempPath, ImGuiInputTextFlags_ReadOnly)) { /* handle change if needed */ }
            if(ImGui::BeginDragDropTarget()) {
                if(const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("AssetData")) {
                    AssetPayload* assetPayload = *static_cast<AssetPayload**>(payload->Data);
                    if(ONEngine::Asset::GetAssetTypeFromExtension(ONEngine::FileSystem::FileExtension(assetPayload->filePath)) == ONEngine::Asset::AssetType::Texture) {
                        int32_t oldId = material->baseTextureId;
                        int32_t newId = static_cast<int32_t>(assetCollection->GetTextureIndex(assetPayload->filePath));
                        EditCommand::Execute<LambdaCommand>(
                            [material, newId]() { material->baseTextureId = newId; },
                            [material, oldId]() { material->baseTextureId = oldId; }
                        );
                        isEdit = true;
                    }
                }
                ImGui::EndDragDropTarget();
            }
			if(material->baseTextureId >= 0) {
				const ONEngine::Asset::Texture* tex = assetCollection->GetTexture(assetCollection->GetTexturePath(material->baseTextureId));
				if(tex) ImGui::Image((ImTextureID)tex->GetSRVGPUHandle().ptr, ImVec2(100, 100));
			}
		}
	}
	return isEdit;
}

bool ImMathf::UVTransformEdit(const char* label, ONEngine::UVTransform* uvTransform) {
	if(!uvTransform) return false;
	bool isEdit = false;
	if(ImGui::CollapsingHeader(label)) {
		if(ImGui::DragFloat2("offset", &uvTransform->position.x, 0.01f)) isEdit = true;
		if(ImGui::DragFloat2("scale", &uvTransform->scale.x, 0.01f, 0.0f, FLT_MAX)) isEdit = true;
		if(ImGui::DragFloat("rotate", &uvTransform->rotate, 0.01f, -3.14159f, 3.14159f)) isEdit = true;
	}
	return isEdit;
}

ImVec2 ImMathf::CalculateAspectFitSize(const ONEngine::Vector2& textureSize, float maxSize) {
	float aspectRatio = textureSize.x / textureSize.y;
	return (aspectRatio > 1.0f) ? ImVec2(maxSize, maxSize / aspectRatio) : ImVec2(maxSize * aspectRatio, maxSize);
}

ImVec2 ImMathf::CalculateAspectFitSize(const ONEngine::Vector2& textureSize, const ImVec2& maxSize) {
	float aspectRatio = textureSize.x / textureSize.y;
	return (aspectRatio > (maxSize.x / maxSize.y)) ? ImVec2(maxSize.x, maxSize.x / aspectRatio) : ImVec2(maxSize.y * aspectRatio, maxSize.y);
}

bool Editor::ImGuiInputText(const char* label, std::string* text, ImGuiInputTextFlags flags) {
	if(!text) return false;
	flags |= ImGuiInputTextFlags_CallbackResize;
	struct CallbackUserData { std::string* text; };
	auto callback = [](ImGuiInputTextCallbackData* data) -> int {
		if(data->EventFlag == ImGuiInputTextFlags_CallbackResize) {
			auto* user = static_cast<CallbackUserData*>(data->UserData);
			user->text->resize(data->BufTextLen);
			data->Buf = user->text->data();
		}
		return 0;
	};
	CallbackUserData userData = { text };
	return ImGui::InputText(label, text->data(), text->capacity() + 1, flags, callback, &userData);
}

bool Editor::ImGuiInputText(const char* label, std::string* text, ImGuiInputTextFlags flags, const char* hint) {
	if(!text) return false;
	flags |= ImGuiInputTextFlags_CallbackResize;
	struct CallbackUserData { std::string* text; };
	auto callback = [](ImGuiInputTextCallbackData* data) -> int {
		if(data->EventFlag == ImGuiInputTextFlags_CallbackResize) {
			auto* user = static_cast<CallbackUserData*>(data->UserData);
			user->text->resize(data->BufTextLen);
			data->Buf = user->text->data();
		}
		return 0;
	};
	CallbackUserData userData = { text };
	return ImGui::InputTextWithHint(label, hint, text->data(), text->capacity() + 1, flags, callback, &userData);
}

void Editor::ImGuiInputTextReadOnly(const char* label, const std::string& text) {
	std::string temp = text;
	ImGuiInputText(label, &temp, ImGuiInputTextFlags_ReadOnly);
}

bool Editor::ImGuiColorEdit(const char* label, ONEngine::Vector4* color) {
	return ImMathf::ColorEdit(label, color);
}

void ONEngine::CustomMeshRendererDebug(CustomMeshRenderer* customMeshRenderer) {
	if(!customMeshRenderer) return;
	if(ImGui::CollapsingHeader("CustomMeshRenderer", ImGuiTreeNodeFlags_DefaultOpen)) {
		bool enabled = (customMeshRenderer->enable != 0);
		if (ImGui::Checkbox("enable", &enabled)) {
			customMeshRenderer->enable = enabled ? 1 : 0;
		}
	}
}

void ONEngine::EffectDebug(Effect* effect) {
	if(!effect) return;
	if(ImGui::CollapsingHeader("Effect", ImGuiTreeNodeFlags_DefaultOpen)) {
		bool enabled = (effect->enable != 0);
		if (ImGui::Checkbox("enable", &enabled)) {
			effect->enable = enabled ? 1 : 0;
		}
        bool isCreate = effect->IsCreateParticle();
        if (ImGui::Checkbox("isCreateParticle", &isCreate)) {
            effect->SetIsCreateParticle(isCreate);
        }
	}
}

bool ONEngine::BeginModuleHeader(const char* label, bool* enabled) {
	ImGui::PushID(label);
	if (enabled) {
		ImGui::Checkbox("##enabled", enabled);
		ImGui::SameLine();
	} else {
		ImGui::Dummy(ImVec2(ImGui::GetFrameHeight(), ImGui::GetFrameHeight()));
		ImGui::SameLine();
	}
	bool open = ImGui::CollapsingHeader(label, ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_AllowItemOverlap);
	if (open && enabled && !(*enabled)) ImGui::BeginDisabled();
	return open;
}

void ONEngine::EndModuleHeader() {
    ImGui::PopID();
}

void ONEngine::ParticleSystemDebug(ParticleSystem* ps) {
	if (!ps) return;
	if (ImGui::CollapsingHeader("Particle System", ImGuiTreeNodeFlags_DefaultOpen)) {
		
		// --- Editor Preview Controls ---
		ImGui::TextColored(ImVec4(1, 1, 0, 1), "--- Editor Preview ---");
		ImGui::Text("Status: %s", ps->isEditorPreview_ ? (ps->isEditorPaused_ ? "Paused" : "Playing") : "Stopped");
		ImGui::Text("Time: %.2f / %.2f", ps->GetTime(), ps->main.duration);
		ImGui::Text("Alive: %llu / %d", ps->aliveCount, ps->main.maxParticles);

		if (ImGui::Button("Preview Play")) {
			ps->isEditorPreview_ = true;
			ps->isEditorPaused_ = false;
			if (!ps->IsPlaying()) ps->Play();
		}
		ImGui::SameLine();
		if (ImGui::Button("Preview Pause")) {
			ps->isEditorPaused_ = !ps->isEditorPaused_;
		}
		ImGui::SameLine();
		if (ImGui::Button("Preview Stop")) {
			ps->isEditorPreview_ = false;
			ps->isEditorPaused_ = false;
			ps->Stop();
		}
		ImGui::SameLine();
		if (ImGui::Button("Preview Restart")) {
			ps->Stop();
			ps->Play();
			ps->ResetTime(0.0f); // 内部状態を強制リセット
			ps->isEditorPreview_ = true;
			ps->isEditorPaused_ = false;
		}

		ImGui::Separator();

		// --- Playback Controls (Runtime) ---
		ImGui::Text("Runtime Status: %s", ps->IsPlaying() ? (ps->IsPaused() ? "Paused" : "Playing") : "Stopped");
		if (ImGui::Button("Play")) ps->Play(); ImGui::SameLine();
		if (ImGui::Button("Pause")) ps->Pause(); ImGui::SameLine();
		if (ImGui::Button("Stop")) ps->Stop(); ImGui::SameLine();
		if (ImGui::Button("Restart")) { ps->Stop(); ps->Play(); }
		
		ImGui::Separator();

		Editor::ImMathf::DragFloat("Duration", &ps->main.duration);
		ImGui::Checkbox("Looping", &ps->main.looping);
		ImGui::Checkbox("Prewarm", &ps->main.prewarm);
		DrawMinMaxFloat("Start Delay", ps->main.startDelay);
		DrawMinMaxFloat("Start Lifetime", ps->main.startLifetime);
		DrawMinMaxFloat("Start Speed", ps->main.startSpeed);
		DrawMinMaxFloat("Start Size", ps->main.startSize);
		DrawMinMaxFloat("Start Rotation", ps->main.startRotation);
		DrawMinMaxColor("Start Color", ps->main.startColor);
		Editor::ImMathf::DragFloat("Gravity Modifier", &ps->main.gravityModifier);
		Editor::ImMathf::InputEnum<SimulationSpace>("Simulation Space", &ps->main.simulationSpace);
		Editor::ImMathf::DragInt("Max Particles", &ps->main.maxParticles, 1, 1, 1000000);
	}
	if (BeginModuleHeader("Emission", &ps->emission.enabled)) {
		Editor::ImMathf::DragFloat("Rate over Time", &ps->emission.rateOverTime);
		if (ImGui::TreeNode("Bursts")) {
			if (ImGui::Button("+")) ps->emission.bursts.push_back({});
			for (size_t i = 0; i < ps->emission.bursts.size(); ++i) {
				ImGui::PushID((int)i);
				float burstAvailW = ImGui::GetContentRegionAvail().x;
				ImGui::SetNextItemWidth(burstAvailW * 0.3f);
				ImGui::DragFloat("##time", &ps->emission.bursts[i].time, 0.01f);
				ImGui::SameLine();
				ImGui::SetNextItemWidth(burstAvailW * 0.3f);
				ImGui::DragInt("##count", &ps->emission.bursts[i].count);
				ImGui::SameLine();
				if (ImGui::Button("x")) { ps->emission.bursts.erase(ps->emission.bursts.begin() + i); ImGui::PopID(); break; }
				ImGui::PopID();
			}
			ImGui::TreePop();
		}
		if (!ps->emission.enabled) ImGui::EndDisabled();
	}
	EndModuleHeader();
	if (BeginModuleHeader("Shape", &ps->shape.enabled)) {
		Editor::ImMathf::InputEnum<ParticleSystemShapeType>("Shape Type", &ps->shape.type);

		if (ps->shape.type == ParticleSystemShapeType::Box) {
			// Boxの場合はスケールのみ表示
			Editor::ImMathf::DragFloat3("Box Scale", &ps->shape.boxScale);
		} else {
			// それ以外の形状はRadiusを基本とする
			Editor::ImMathf::DragFloat("Radius", &ps->shape.radius);

			// Edge以外は厚みの設定が可能
			if (ps->shape.type != ParticleSystemShapeType::Edge) {
				Editor::ImMathf::DragFloat("Radius Thickness", &ps->shape.radiusThickness);
			}

			// ConeとCircleはArc（角度）の設定が可能
			if (ps->shape.type == ParticleSystemShapeType::Cone || ps->shape.type == ParticleSystemShapeType::Circle) {
				Editor::ImMathf::DragFloat("Arc", &ps->shape.arc);
			}

			// Cone特有のパラメータ
			if (ps->shape.type == ParticleSystemShapeType::Cone) {
				Editor::ImMathf::DragFloat("Angle", &ps->shape.angle);
			}
		}

		if (!ps->shape.enabled) ImGui::EndDisabled();
	}
	EndModuleHeader();

	if (BeginModuleHeader("Color over Lifetime", &ps->colorOverLifetime.enabled)) {
		DrawMinMaxGradient("Color", ps->colorOverLifetime.color);
		if (!ps->colorOverLifetime.enabled) ImGui::EndDisabled();
	}
	EndModuleHeader();

	if (BeginModuleHeader("Size over Lifetime", &ps->sizeOverLifetime.enabled)) {
		DrawMinMaxCurve("Size", ps->sizeOverLifetime.size);
		if (!ps->sizeOverLifetime.enabled) ImGui::EndDisabled();
	}
	EndModuleHeader();

	if (BeginModuleHeader("Velocity over Lifetime", &ps->velocityOverLifetime.enabled)) {
		DrawMinMaxCurve("Linear X", ps->velocityOverLifetime.x);
		DrawMinMaxCurve("Linear Y", ps->velocityOverLifetime.y);
		DrawMinMaxCurve("Linear Z", ps->velocityOverLifetime.z);
		DrawMinMaxCurve("Speed Modifier", ps->velocityOverLifetime.speedModifier);
		Editor::ImMathf::InputEnum<SimulationSpace>("Space", &ps->velocityOverLifetime.space);
		if (!ps->velocityOverLifetime.enabled) ImGui::EndDisabled();
	}
	EndModuleHeader();

	bool rendererEnabled = true;
	if (BeginModuleHeader("Renderer", &rendererEnabled)) {
		Editor::ImMathf::InputEnum<ParticleSystemRenderer::RenderMode>("Render Mode", &ps->renderer.renderMode);
		Editor::ImMathf::InputEnum<ParticleSystemRenderer::RenderAlignment>("Render Alignment", &ps->renderer.alignment);
		
		if (ps->renderer.renderMode == ParticleSystemRenderer::RenderMode::StretchedBillboard) {
			Editor::ImMathf::DragFloat("Speed Scale", &ps->renderer.speedScale);
			Editor::ImMathf::DragFloat("Length Scale", &ps->renderer.lengthScale);
		}

		Editor::ImMathf::InputEnum<ParticleSystemRenderer::BlendMode>("Blend Mode", &ps->renderer.blendMode);
		DrawAssetGuidField("Material", ps->renderer.materialGuid, Asset::AssetType::Material);
		DrawAssetGuidField("Mesh", ps->renderer.meshGuid, Asset::AssetType::Mesh);
	}
	EndModuleHeader();
}

void ONEngine::ParticleSystem2DDebug(ParticleSystem2D* ps) {
	if (!ps) return;
	if (ImGui::CollapsingHeader("Particle System 2D", ImGuiTreeNodeFlags_DefaultOpen)) {
		
		// --- Editor Preview Controls ---
		ImGui::TextColored(ImVec4(1, 1, 0, 1), "--- Editor Preview ---");
		ImGui::Text("Status: %s", ps->isEditorPreview_ ? (ps->isEditorPaused_ ? "Paused" : "Playing") : "Stopped");
		ImGui::Text("Time: %.2f / %.2f", ps->GetTime(), ps->main.duration);
		ImGui::Text("Alive: %llu / %d", ps->aliveCount, ps->main.maxParticles);

		if (ImGui::Button("Preview Play")) {
			ps->isEditorPreview_ = true;
			ps->isEditorPaused_ = false;
			if (!ps->IsPlaying()) ps->Play();
		}
		ImGui::SameLine();
		if (ImGui::Button("Preview Pause")) {
			ps->isEditorPaused_ = !ps->isEditorPaused_;
		}
		ImGui::SameLine();
		if (ImGui::Button("Preview Stop")) {
			ps->isEditorPreview_ = false;
			ps->isEditorPaused_ = false;
			ps->Stop();
		}
		ImGui::SameLine();
		if (ImGui::Button("Preview Restart")) {
			ps->Stop();
			ps->Play();
			ps->ResetTime(0.0f);
			ps->isEditorPreview_ = true;
			ps->isEditorPaused_ = false;
		}

		ImGui::Separator();

		// --- Playback Controls (Runtime) ---
		ImGui::Text("Runtime Status: %s", ps->IsPlaying() ? (ps->IsPaused() ? "Paused" : "Playing") : "Stopped");
		if (ImGui::Button("Play")) ps->Play(); ImGui::SameLine();
		if (ImGui::Button("Pause")) ps->Pause(); ImGui::SameLine();
		if (ImGui::Button("Stop")) ps->Stop(); ImGui::SameLine();
		if (ImGui::Button("Restart")) { ps->Stop(); ps->Play(); }
		
		ImGui::Separator();

		Editor::ImMathf::DragFloat("Duration", &ps->main.duration);
		ImGui::Checkbox("Looping", &ps->main.looping);
		ImGui::Checkbox("Prewarm", &ps->main.prewarm);
		DrawMinMaxFloat("Start Delay", ps->main.startDelay);
		DrawMinMaxFloat("Start Lifetime", ps->main.startLifetime);
		DrawMinMaxFloat("Start Speed", ps->main.startSpeed);
		DrawMinMaxFloat("Start Size", ps->main.startSize);
		DrawMinMaxFloat("Start Rotation", ps->main.startRotation);
		DrawMinMaxColor("Start Color", ps->main.startColor);
		Editor::ImMathf::DragFloat("Gravity Modifier", &ps->main.gravityModifier);
		Editor::ImMathf::InputEnum<SimulationSpace>("Simulation Space", &ps->main.simulationSpace);
		Editor::ImMathf::DragInt("Max Particles", &ps->main.maxParticles, 1, 1, 1000000);
	}
	if (BeginModuleHeader("Emission", &ps->emission.enabled)) {
		Editor::ImMathf::DragFloat("Rate over Time", &ps->emission.rateOverTime);
		if (ImGui::TreeNode("Bursts")) {
			if (ImGui::Button("+")) ps->emission.bursts.push_back({});
			for (size_t i = 0; i < ps->emission.bursts.size(); ++i) {
				ImGui::PushID((int)i);
				float burstAvailW = ImGui::GetContentRegionAvail().x;
				ImGui::SetNextItemWidth(burstAvailW * 0.3f);
				ImGui::DragFloat("##time", &ps->emission.bursts[i].time, 0.01f);
				ImGui::SameLine();
				ImGui::SetNextItemWidth(burstAvailW * 0.3f);
				ImGui::DragInt("##count", &ps->emission.bursts[i].count);
				ImGui::SameLine();
				if (ImGui::Button("x")) { ps->emission.bursts.erase(ps->emission.bursts.begin() + i); ImGui::PopID(); break; }
				ImGui::PopID();
			}
			ImGui::TreePop();
		}
		if (!ps->emission.enabled) ImGui::EndDisabled();
	}
	EndModuleHeader();
	if (BeginModuleHeader("Shape", &ps->shape.enabled)) {
		Editor::ImMathf::InputEnum<ParticleSystemShapeType>("Shape Type", &ps->shape.type);

		if (ps->shape.type == ParticleSystemShapeType::Box) {
			Editor::ImMathf::DragFloat3("Box Scale", &ps->shape.boxScale);
		} else {
			Editor::ImMathf::DragFloat("Radius", &ps->shape.radius);

			if (ps->shape.type != ParticleSystemShapeType::Edge) {
				Editor::ImMathf::DragFloat("Radius Thickness", &ps->shape.radiusThickness);
			}

			if (ps->shape.type == ParticleSystemShapeType::Cone || ps->shape.type == ParticleSystemShapeType::Circle) {
				Editor::ImMathf::DragFloat("Arc", &ps->shape.arc);
			}

			if (ps->shape.type == ParticleSystemShapeType::Cone) {
				Editor::ImMathf::DragFloat("Angle", &ps->shape.angle);
			}
		}

		if (!ps->shape.enabled) ImGui::EndDisabled();
	}
	EndModuleHeader();

	if (BeginModuleHeader("Color over Lifetime", &ps->colorOverLifetime.enabled)) {
		DrawMinMaxGradient("Color", ps->colorOverLifetime.color);
		if (!ps->colorOverLifetime.enabled) ImGui::EndDisabled();
	}
	EndModuleHeader();

	if (BeginModuleHeader("Size over Lifetime", &ps->sizeOverLifetime.enabled)) {
		DrawMinMaxCurve("Size", ps->sizeOverLifetime.size);
		if (!ps->sizeOverLifetime.enabled) ImGui::EndDisabled();
	}
	EndModuleHeader();

	if (BeginModuleHeader("Velocity over Lifetime", &ps->velocityOverLifetime.enabled)) {
		DrawMinMaxCurve("Linear X", ps->velocityOverLifetime.x);
		DrawMinMaxCurve("Linear Y", ps->velocityOverLifetime.y);
		DrawMinMaxCurve("Speed Modifier", ps->velocityOverLifetime.speedModifier);
		Editor::ImMathf::InputEnum<SimulationSpace>("Space", &ps->velocityOverLifetime.space);
		if (!ps->velocityOverLifetime.enabled) ImGui::EndDisabled();
	}
	EndModuleHeader();

	bool rendererEnabled = true;
	if (BeginModuleHeader("Renderer", &rendererEnabled)) {
		Editor::ImMathf::InputEnum<ParticleSystemRenderer::RenderMode>("Render Mode", &ps->renderer.renderMode);
		Editor::ImMathf::InputEnum<ParticleSystemRenderer::RenderAlignment>("Render Alignment", &ps->renderer.alignment);
		
		if (ps->renderer.renderMode == ParticleSystemRenderer::RenderMode::StretchedBillboard) {
			Editor::ImMathf::DragFloat("Speed Scale", &ps->renderer.speedScale);
			Editor::ImMathf::DragFloat("Length Scale", &ps->renderer.lengthScale);
		}

		Editor::ImMathf::InputEnum<ParticleSystemRenderer::BlendMode>("Blend Mode", &ps->renderer.blendMode);
		Editor::ImMathf::InputEnum<FlipMode>("Flip Mode", &ps->renderer.flipMode);
		DrawAssetGuidField("Material", ps->renderer.materialGuid, Asset::AssetType::Material);
		DrawAssetGuidField("Mesh", ps->renderer.meshGuid, Asset::AssetType::Mesh);
	}
	EndModuleHeader();

	if (BeginModuleHeader("Texture Sheet Animation", &ps->textureSheetAnimation.enabled)) {
		Editor::ImMathf::DragInt("Tiles X", &ps->textureSheetAnimation.tilesX, 1, 1, 64);
		Editor::ImMathf::DragInt("Tiles Y", &ps->textureSheetAnimation.tilesY, 1, 1, 64);
		Editor::ImMathf::DragFloat("FPS", &ps->textureSheetAnimation.fps, 0.05f, 0.0f, 120.0f);
		if (!ps->textureSheetAnimation.enabled) ImGui::EndDisabled();
	}
	EndModuleHeader();
}
