#include "SkinMeshUpdateSystem.h"

using namespace ONEngine;

/// engine
#include "Engine/ECS/EntityComponentSystem/EntityComponentSystem.h"
#include "Engine/ECS/Component/Array/ComponentArray.h"
#include "Engine/Asset/Collection/AssetCollection.h"

SkinMeshUpdateSystem::SkinMeshUpdateSystem(DxManager* dxm, Asset::AssetCollection* assetCollection)
	: pDxManager_(dxm), pAssetCollection_(assetCollection) {
}

void SkinMeshUpdateSystem::RuntimeUpdate(ECSGroup* ecs) {

	/// Compの配列を取得＆使用中のCompがなければ終了
	ComponentArray<SkinMeshRenderer>* skinMeshArray = ecs->GetComponentArray<SkinMeshRenderer>();
	if (!skinMeshArray || skinMeshArray->GetUsedComponents().empty()) {
		return;
	}

	for (auto& skinMesh : skinMeshArray->GetUsedComponents()) {
		/// 以降の処理を無視する条件
		if (!CheckComponentEnable(skinMesh)) {
			continue;
		}

		/// skin clusterが存在しないなら生成する
		if (skinMesh->isChangingMesh_) {
			Console::LogInfo(std::format("SkinMeshUpdateSystem: Changing mesh for entity '{}' to path: '{}'", 
				skinMesh->GetOwner()->GetName(), skinMesh->GetMeshPath()));

			Asset::Model* model = pAssetCollection_->GetModel(skinMesh->GetMeshPath());
			if (!model) {
				static std::unordered_map<std::string, bool> loggedMissing;
				if (!loggedMissing[skinMesh->GetMeshPath()]) {
					Console::LogError(std::format("SkinMeshUpdateSystem: Failed to find model at path: '{}'. Registered models may not include this path.", skinMesh->GetMeshPath()));
					loggedMissing[skinMesh->GetMeshPath()] = true;
				}
				continue; 
			}

			Console::LogInfo(std::format("SkinMeshUpdateSystem: Found model '{}'. Creating Skeleton and SkinCluster...", skinMesh->GetMeshPath()));
			Skeleton skeleton = ANIME_MATH::CreateSkeleton(model->GetRootNode());
			SkinCluster skinCluster = ANIME_MATH::CreateSkinCluster(skeleton, model, pDxManager_);

			skinMesh->skinCluster_ = std::move(skinCluster);
			skinMesh->skeleton_ = std::move(skeleton);
			Console::LogInfo(std::format("SkinMeshUpdateSystem: Skeleton created with {} joints.", skinMesh->skeleton_.joints.size()));

			skinMesh->animationTime_ = 0.0f;
			if (!model->GetAnimationClips().empty()) {
				const auto& clips = model->GetAnimationClips();
				const auto& clip = clips.begin()->second;
				skinMesh->duration_ = clip.duration;
				skinMesh->nodeAnimationMap_ = clip.nodeAnimationMap;
				Console::LogInfo(std::format("SkinMeshUpdateSystem: Initialized with clip '{}' (duration: {:.2f}s). Total clips available: {}", clip.name, clip.duration, clips.size()));
				
				// List all clips for debugging
				for(const auto& [hash, c] : clips) {
					Console::LogInfo(std::format("  Available Clip: '{}' (hash: {})", c.name, hash));
				}
			} else {
				skinMesh->duration_ = 0.0f;
				skinMesh->nodeAnimationMap_.clear();
				Console::LogWarning(std::format("SkinMeshUpdateSystem: Model '{}' has no animation clips.", skinMesh->GetMeshPath()));
			}

			skinMesh->isChangingMesh_ = false;

			UpdateSkeleton(skinMesh);
			UpdateSkinCluster(skinMesh);
			Console::LogInfo("SkinMeshUpdateSystem: Initialization complete.");
		}


		/// skin clusterがあるかチェック
		if (!skinMesh->skinCluster_) {
			continue;
		}

		/// アニメーションが再生されていないならスキップ
		if (!skinMesh->isPlaying_) {
			continue;
		}


		skinMesh->animationTime_ += Time::DeltaTime();
		skinMesh->animationTime_ = std::fmod(skinMesh->animationTime_, skinMesh->GetDuration());

		UpdateSkeleton(skinMesh);
		UpdateSkinCluster(skinMesh);

	}


}

void SkinMeshUpdateSystem::UpdateSkeleton(SkinMeshRenderer* smr) {
	/// ------------------------------------
	/// スケルトンの更新
	/// ------------------------------------
	UpdateSkeletonRecursive(smr, smr->skeleton_.root, std::nullopt);
}

void SkinMeshUpdateSystem::UpdateSkeletonRecursive(SkinMeshRenderer* smr, int32_t jointIndex, const std::optional<int32_t>& parentIndex) {
	Skeleton& skeleton = smr->skeleton_;
	Joint& joint = skeleton.joints[jointIndex];

	/// アニメーションの適用
	auto it = smr->nodeAnimationMap_.find(joint.nameHash);
	if (it != smr->nodeAnimationMap_.end()) {
		NodeAnimation& animation = it->second;
		if (!animation.translate.empty()) { joint.transform.position = ANIME_MATH::CalculateValue(animation.translate, smr->animationTime_); }
		if (!animation.rotate.empty()) { joint.transform.rotate = ANIME_MATH::CalculateValue(animation.rotate, smr->animationTime_); }
		if (!animation.scale.empty()) { joint.transform.scale = ANIME_MATH::CalculateValue(animation.scale, smr->animationTime_); }
	}
	joint.transform.Update();

	/// スケルトン空間行列の計算 (Local * Parent)
	if (parentIndex) {
		joint.matSkeletonSpace = joint.transform.matWorld * skeleton.joints[*parentIndex].matSkeletonSpace;
	} else {
		joint.matSkeletonSpace = joint.transform.matWorld;
	}

	/// ワールド行列の計算
	joint.matWorld = joint.matSkeletonSpace * smr->GetOwner()->GetTransform()->matWorld;

	/// 子の更新
	for (int32_t childIndex : joint.children) {
		UpdateSkeletonRecursive(smr, childIndex, jointIndex);
	}
}

void SkinMeshUpdateSystem::UpdateSkinCluster(SkinMeshRenderer* smr) {
	Skeleton& skeleton = smr->skeleton_;
	SkinCluster& skinCluster = smr->skinCluster_.value();


	/// ------------------------------------
	/// スキンクラスターの更新
	/// ------------------------------------
	for (size_t jointIndex = 0; jointIndex < skeleton.joints.size(); ++jointIndex) {

		if (jointIndex >= skinCluster.matBindPoseInverseArray.size()) {
			Console::Log("[warring] SkinMeshUpdateSystem::Update: jointIndex out of range for matBindPoseInverseArray");
			continue; ///< 範囲外の場合はスキップ
		}

		/// スキニング行列 = 逆バインドポーズ行列 * スケルトン空間行列
		skinCluster.mappedPalette[jointIndex].matSkeletonSpace =
			skinCluster.matBindPoseInverseArray[jointIndex] * skeleton.joints[jointIndex].matSkeletonSpace;

		skinCluster.mappedPalette[jointIndex].matSkeletonSpaceInverseTranspose =
			Matrix4x4::MakeTranspose(Matrix4x4::MakeInverse(skinCluster.mappedPalette[jointIndex].matSkeletonSpace));

	}

}
