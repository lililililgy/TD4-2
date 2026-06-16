#pragma once

/// std
#include <string>
#include <optional>
#include <unordered_map>

/// external
#include <mono/jit/jit.h>
#include <nlohmann/json.hpp>

/// engine
#include "../../Interface/IComponent.h"
#include "Engine/Asset/Assets/Mesh/Skinning.h"

/// /////////////////////////////////////////////
/// スキニングアニメーションのRender Component
/// /////////////////////////////////////////////
namespace ONEngine::Asset {
class AssetCollection;
}

namespace ONEngine {

class SkinMeshRenderer : public IRenderComponent {
	friend class SkinMeshUpdateSystem;
	friend class SkinMeshRenderingPipeline;
	friend class AnimatorUpdateSystem;
public:
	/// =========================================
	/// public : methods
	/// =========================================

	SkinMeshRenderer();
	~SkinMeshRenderer() override = default;

private:
	/// =========================================
	/// private : methods
	/// =========================================

	std::string meshPath_;
	std::string texturePath_;
	Vector4 color_;

	bool isPlaying_;
	float animationTime_;
	float duration_;
	float animationScale_;
	float debugJointSize_;
	float debugRectSize_;

	std::unordered_map<uint32_t, NodeAnimation> nodeAnimationMap_;
	std::optional<SkinCluster> skinCluster_; ///< スキンアニメーションのデータ
	Skeleton skeleton_; ///< ボーンデータ
	bool isChangingMesh_;

public:
	/// ==========================================
	/// public : accessors
	/// ==========================================

	void SetMeshPath(const std::string& path);
	void SetTexturePath(const std::string& path);

	void SetColor(const Vector4& color);

	void SetIsPlaying(bool isPlaying);
	void SetAnimationTime(float time);
	void SetDuration(float duration);
	void SetAnimationScale(float scale);
	void SetDebugJointSize(float size);
	void SetDebugRectSize(float size);
	void SetNodeAnimationMap(const std::unordered_map<uint32_t, NodeAnimation>& map);


	const std::string& GetMeshPath() const;
	const std::string& GetTexturePath() const;

	const Vector4& GetColor() const;

	bool GetIsPlaying() const;
	float GetAnimationTime() const;
	float GetDuration() const;
	float GetAnimationScale() const;
	float GetDebugJointSize() const;
	float GetDebugRectSize() const;

	const Skeleton& GetSkeleton() const;

};


namespace ComponentDebug {
	void SkinMeshRendererDebug(SkinMeshRenderer* smr, Asset::AssetCollection* assetCollection);
}


/// ===================================================
/// csで使用するための関数群
/// ===================================================

SkinMeshRenderer* GetSkinMeshRenderer(uint64_t nativeHandle);

MonoString* InternalGetMeshPath(uint64_t nativeHandle);
void InternalSetMeshPath(uint64_t nativeHandle, MonoString* path);

MonoString* InternalGetTexturePath(uint64_t nativeHandle);
void InternalSetTexturePath(uint64_t nativeHandle, MonoString* path);

bool InternalGetIsPlaying(uint64_t nativeHandle);
void InternalSetIsPlaying(uint64_t nativeHandle, bool isPlaying);

float InternalGetAnimationTime(uint64_t nativeHandle);
void InternalSetAnimationTime(uint64_t nativeHandle, float time);

float InternalGetAnimationScale(uint64_t nativeHandle);
void InternalSetAnimationScale(uint64_t nativeHandle, float scale);

void InternalGetJointTransform(uint64_t nativeHandle, MonoString* jointName, Vector3* outScale, Quaternion* outRotation, Vector3* outPosition);



void from_json(const nlohmann::json& j, SkinMeshRenderer& smr);
void to_json(nlohmann::json& j, const SkinMeshRenderer& smr);

} /// ONEngine