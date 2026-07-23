#include "Transform.h"
#include <nlohmann/json.hpp>

#define NOMINMAX

/// std
#include <limits>
#include <vector>

/// externals
#include <imgui.h>

/// engine
#include "Engine/ECS/EntityComponentSystem/EntityComponentSystem.h"
#include "Engine/Script/MonoScriptEngine.h"

/// editor
#include "Engine/Editor/Commands/ComponentEditCommands/ComponentJsonConverter.h"
#include "Engine/Editor/Commands/ComponentEditCommands/ModifyTransformCommand.h"
#include "Engine/Editor/EditorUtils.h"
#include "Engine/Editor/Manager/EditCommand.h"

using namespace ONEngine;

Transform::Transform() {
	position = Vector3::Zero;
	rotate = Quaternion::kIdentity;
	scale = Vector3::One;
	euler = Vector3::Zero;
	lastSyncedRotate = Quaternion::kIdentity;
}
Transform::~Transform() = default;


void Transform::Update() {
	matWorld = Matrix4x4::MakeScale(scale) * Matrix4x4::MakeRotate(Quaternion::Normalize(rotate)) * Matrix4x4::MakeTranslate(position);
}

void Transform::Reset() {
	position = Vector3::Zero;
	rotate = Quaternion::kIdentity;
	scale = Vector3::One;
	euler = Vector3::Zero;
	lastSyncedRotate = Quaternion::kIdentity;
	matWorld = Matrix4x4::kIdentity;
}

void Transform::SetPosition(const Vector3& v) {
	position = v;
}

void Transform::SetRotate(const Vector3& v) {
	euler = v; // ここでの v は度数法を想定
	SyncQuaternionFromEuler();
}

void Transform::SetRotate(const Quaternion& q) {
	rotate = q;
	SyncEulerFromQuaternion();
}

void Transform::SetScale(const Vector3& v) {
	scale = v;
}

const Vector3& Transform::GetPosition() const {
	return position;
}

const Quaternion& Transform::GetRotate() const {
	return rotate;
}

const Vector3& Transform::GetScale() const {
	return scale;
}

const Matrix4x4& Transform::GetMatWorld() const {
	return matWorld;
}

void Transform::SyncQuaternionFromEuler() {
	Vector3 rad = {
		euler.x * Math::Deg2Rad,
		euler.y * Math::Deg2Rad,
		euler.z * Math::Deg2Rad
	};
	rotate = Quaternion::FromEuler(rad);
	lastSyncedRotate = rotate;
}

void Transform::SyncEulerFromQuaternion() {
	Vector3 rad = Quaternion::ToEuler(rotate);
	euler = {
		rad.x * Math::Rad2Deg,
		rad.y * Math::Rad2Deg,
		rad.z * Math::Rad2Deg
	};
	lastSyncedRotate = rotate;
}


/// ===================================================
/// mono からのTransform取得用関数
/// ===================================================

void ONEngine::UpdateTransform(Transform* transform) {
	if(GameEntity* entity = transform->GetOwner()) {
		entity->UpdateTransform();
	}
}

void ONEngine::InternalGetPosition(uint64_t nativeHandle, float* x, float* y, float* z) {
	Transform* transform = reinterpret_cast<Transform*>(nativeHandle);
	if(!transform) {
		Console::LogError("Transform pointer is null");
		return;
	}

	const Matrix4x4& matWorld = transform->GetMatWorld();
	const Vector3& position = { matWorld.m[3][0], matWorld.m[3][1], matWorld.m[3][2] };

	if(x) { *x = position.x; }
	if(y) { *y = position.y; }
	if(z) { *z = position.z; }
}

void ONEngine::InternalGetLocalPosition(uint64_t nativeHandle, float* x, float* y, float* z) {
	Transform* transform = reinterpret_cast<Transform*>(nativeHandle);
	if(!transform) {
		Console::LogError("Transform pointer is null");
		return;
	}

	if(x) { *x = transform->position.x; }
	if(y) { *y = transform->position.y; }
	if(z) { *z = transform->position.z; }
}

void ONEngine::InternalGetRotate(uint64_t nativeHandle, float* x, float* y, float* z, float* w) {
	Transform* transform = reinterpret_cast<Transform*>(nativeHandle);
	if(!transform) {
		Console::LogError("Transform pointer is null");
		return;
	}

	if(x) { *x = transform->rotate.x; }
	if(y) { *y = transform->rotate.y; }
	if(z) { *z = transform->rotate.z; }
	if(w) { *w = transform->rotate.w; }
}

void ONEngine::InternalGetScale(uint64_t nativeHandle, float* x, float* y, float* z) {
	Transform* transform = reinterpret_cast<Transform*>(nativeHandle);
	if(!transform) {
		Console::LogError("Transform pointer is null");
		return;
	}

	if(x) { *x = transform->scale.x; }
	if(y) { *y = transform->scale.y; }
	if(z) { *z = transform->scale.z; }
}

void ONEngine::InternalSetPosition(uint64_t nativeHandle, float x, float y, float z) {
	Transform* transform = reinterpret_cast<Transform*>(nativeHandle);
	if(!transform) {
		Console::LogError("Transform pointer is null");
		return;
	}

	transform->position.x = x;
	transform->position.y = y;
	transform->position.z = z;
	UpdateTransform(transform); // 更新を呼び出す
}

void ONEngine::InternalSetLocalPosition(uint64_t nativeHandle, float x, float y, float z) {
	Transform* transform = reinterpret_cast<Transform*>(nativeHandle);
	if(!transform) {
		Console::LogError("Transform pointer is null");
		return;
	}

	transform->position.x = x;
	transform->position.y = y;
	transform->position.z = z;
	UpdateTransform(transform); // 更新を呼び出す
}

void ONEngine::InternalSetRotate(uint64_t nativeHandle, float x, float y, float z, float w) {
	Transform* transform = reinterpret_cast<Transform*>(nativeHandle);
	if(!transform) {
		Console::LogError("Transform pointer is null");
		return;
	}

	transform->rotate.x = x;
	transform->rotate.y = y;
	transform->rotate.z = z;
	transform->rotate.w = w;
	transform->SyncEulerFromQuaternion(); // Eulerキャッシュを同期
	UpdateTransform(transform); // 更新を呼び出す
}

void ONEngine::InternalSetScale(uint64_t nativeHandle, float x, float y, float z) {
	Transform* transform = reinterpret_cast<Transform*>(nativeHandle);
	if(!transform) {
		Console::LogError("Transform pointer is null");
		return;
	}

	transform->scale.x = x;
	transform->scale.y = y;
	transform->scale.z = z;
	UpdateTransform(transform); // 更新を呼び出す
}

void ComponentDebug::TransformDebug(Transform* transform) {
	std::vector<Transform*> transforms = { transform };
	TransformDebug(transforms);
}

void ComponentDebug::TransformDebug(const std::vector<Transform*>& transforms) {
	Transform* first = transforms.empty() ? nullptr : transforms[0];
	if (!first) return;

	// アニメーションなどで外部から回転が書き換えられていないかチェック
	if (!(first->rotate == first->lastSyncedRotate)) {
		first->SyncEulerFromQuaternion();
	}

	Vector3   pos = first->position;
	Vector3   euler = first->euler; 
	Vector3   scale = first->scale;
	int       flags = first->matrixCalcFlags;

	static std::vector<Vector3> s_startPos, s_startEuler, s_startScale;

	static bool isUnifieds[3] = { false, false, true };
	constexpr float minValue = (std::numeric_limits<float>::lowest)();
	constexpr float maxValue = (std::numeric_limits<float>::max)();

	// --- Position ---
	bool posActivated = false;
	bool posDeactivated = false;
	bool posChanged = Editor::DrawVec3Control("position", pos, 0.1f, minValue, maxValue, 100.0f, &isUnifieds[0], false, &posActivated, &posDeactivated);

	if (posActivated) {
		s_startPos.clear();
		for (auto t : transforms) s_startPos.push_back(t->position);
	}

	if (posChanged) {
		Vector3 delta = pos - first->position;
		for (auto t : transforms) { t->position += delta; t->Update(); }
	}

	if (posDeactivated) {
		if (s_startPos.size() == transforms.size()) {
			bool changed = false;
			for (size_t i = 0; i < transforms.size(); ++i) {
				if (s_startPos[i] != transforms[i]->position) { changed = true; break; }
			}

			if (changed) {
				std::vector<Editor::ModifyTransformCommand::Data> data;
				for (size_t i = 0; i < transforms.size(); ++i) {
					data.push_back({ transforms[i], s_startPos[i], transforms[i]->position });
				}
				Editor::EditCommand::Execute<Editor::ModifyTransformCommand>(Editor::ModifyTransformCommand::Target::Position, data);
			}
		}
	}

	// --- Rotation ---
	bool rotActivated = false;
	bool rotDeactivated = false;
	bool rotChanged = Editor::DrawVec3Control("rotation", euler, 0.5f, minValue, maxValue, 100.0f, &isUnifieds[1], false, &rotActivated, &rotDeactivated);

	if (rotActivated) {
		s_startEuler.clear();
		for (auto t : transforms) s_startEuler.push_back(t->euler);
	}

	if (rotChanged) {
		Vector3 delta = euler - first->euler;
		for (auto t : transforms) { 
			t->euler += delta; 
			t->SyncQuaternionFromEuler(); 
			t->Update(); 
		}
	}

	if (rotDeactivated) {
		if (s_startEuler.size() == transforms.size()) {
			bool changed = false;
			for (size_t i = 0; i < transforms.size(); ++i) {
				if (s_startEuler[i] != transforms[i]->euler) { changed = true; break; }
			}

			if (changed) {
				std::vector<Editor::ModifyTransformCommand::Data> data;
				for (size_t i = 0; i < transforms.size(); ++i) {
					data.push_back({ transforms[i], s_startEuler[i], transforms[i]->euler });
				}
				Editor::EditCommand::Execute<Editor::ModifyTransformCommand>(Editor::ModifyTransformCommand::Target::Rotation, data);
			}
		}
	}

	// --- Scale ---
	bool scaleActivated = false;
	bool scaleDeactivated = false;
	bool scaleChanged = Editor::DrawVec3Control("scale", scale, 0.1f, minValue, maxValue, 100.0f, &isUnifieds[2], false, &scaleActivated, &scaleDeactivated);

	if (scaleActivated) {
		s_startScale.clear();
		for (auto t : transforms) s_startScale.push_back(t->scale);
	}

	if (scaleChanged) {
		Vector3 delta = scale - first->scale;
		for (auto t : transforms) { t->scale += delta; t->Update(); }
	}

	if (scaleDeactivated) {
		if (s_startScale.size() == transforms.size()) {
			bool changed = false;
			for (size_t i = 0; i < transforms.size(); ++i) {
				if (s_startScale[i] != transforms[i]->scale) { changed = true; break; }
			}

			if (changed) {
				std::vector<Editor::ModifyTransformCommand::Data> data;
				for (size_t i = 0; i < transforms.size(); ++i) {
					data.push_back({ transforms[i], s_startScale[i], transforms[i]->scale });
				}
				Editor::EditCommand::Execute<Editor::ModifyTransformCommand>(Editor::ModifyTransformCommand::Target::Scale, data);
			}
		}
	}

	bool flagsChanged = false;
	flagsChanged |= ImGui::CheckboxFlags("matrixCalcFlags: position", &flags, Transform::kPosition);
	flagsChanged |= ImGui::CheckboxFlags("matrixCalcFlags: rotate", &flags, Transform::kRotate);
	flagsChanged |= ImGui::CheckboxFlags("matrixCalcFlags: scale", &flags, Transform::kScale);
	
	if (flagsChanged) {
		for (auto t : transforms) { t->matrixCalcFlags = flags; t->Update(); }
	}

	first->lastSyncedRotate = first->rotate;
}


void ONEngine::from_json(const nlohmann::json& j, Transform& t) {
	t.enable = j.at("enable").get<int>();
	t.position = j.at("position").get<Vector3>();
	t.rotate = j.at("rotate").get<Quaternion>();
	t.scale = j.at("scale").get<Vector3>();
	t.matrixCalcFlags = j.value("matrixCalcFlags", Transform::kAll);
	t.SyncEulerFromQuaternion(); 
	t.Update();
}

void ONEngine::to_json(nlohmann::json& j, const Transform& t) {
	j = nlohmann::json{
		{ "type", "Transform" },
		{ "enable", t.enable },
		{ "position", t.position },
		{ "rotate", t.rotate },
		{ "scale", t.scale },
		{ "matrixCalcFlags", t.matrixCalcFlags }
	};
}
