#include "Skinning.h"

/// engine
#include "Engine/Core/DirectX12/Manager/DxManager.h"
#include "Engine/Core/Utility/Utility.h"
#include "Engine/Core/Utility/Tools/StringHash.h"
#include "Model.h"
#include "Engine/ECS/Component/Components/ComputeComponents/Animator/Animator.h"

using namespace ONEngine;

Vector3 ANIME_MATH::CalculateValue(const std::vector<KeyFrameVector3>& keyFrames, float time) {
	/// ----- Vector3のキーフレーム配列を計算 ----- ///

	Assert(!keyFrames.empty(), "keyframe empty...");

	/// 最初のキーフレーム以前の場合は最初の値を返す
	if (keyFrames.size() == 1 || time <= keyFrames[0].time) {
		return keyFrames[0].value;
	}

	/// キーフレーム間の補間計算
	for (size_t index = 0; index < keyFrames.size() - 1; ++index) {
		size_t nextIndex = index + 1;

		if (keyFrames[index].time <= time && time <= keyFrames[nextIndex].time) {
			float t = (time - keyFrames[index].time) / (keyFrames[nextIndex].time - keyFrames[index].time);
			return Vector3::Lerp(keyFrames[index].value, keyFrames[nextIndex].value, t);
		}
	}

	return keyFrames.back().value;
}

Quaternion ANIME_MATH::CalculateValue(const std::vector<KeyFrameQuaternion>& keyFrames, float time) {
	/// ----- Quaternionのキーフレーム配列を計算 ----- ///

	Assert(!keyFrames.empty(), "keyframe empty...");


	/// 最初のキーフレーム以前の場合は最初の値を返す
	if (keyFrames.size() == 1 || time <= keyFrames[0].time) {
		return keyFrames[0].value;
	}

	/// キーフレーム間の補間計算
	for (size_t index = 0; index < keyFrames.size() - 1; ++index) {
		size_t nextIndex = index + 1;

		if (keyFrames[index].time <= time && time <= keyFrames[nextIndex].time) {
			float t = (time - keyFrames[index].time) / (keyFrames[nextIndex].time - keyFrames[index].time);
			return Quaternion::Slerp(keyFrames[index].value, keyFrames[nextIndex].value, t);
		}
	}

	return keyFrames.back().value;
}

int32_t ANIME_MATH::CreateJoint(const Node& node, const std::optional<int32_t>& parent, std::vector<Joint>& joints) {
	/// ----- ノードからジョイントを作成 ----- ///

	/// 自身のインデックスを決定し、追加する
	int32_t selfIndex = static_cast<int32_t>(joints.size());
	joints.emplace_back();

	/// 参照を取得 (再確保に注意)
	Joint& joint = joints[selfIndex];
	joint.name = node.name;
	joint.nameHash = StringHash::Get(node.name);
	joint.transform.matWorld = node.transform.matWorld;

	joint.matSkeletonSpace = Matrix4x4::kIdentity;
	joint.index = selfIndex;
	joint.parent = parent;

	/// 子ノードのジョイントを再帰的に作成
	for (const Node& child : node.children) {
		int32_t childIndex = CreateJoint(child, selfIndex, joints);

		/// 再確保されている可能性があるため、再度インデックスでアクセスして子を追加
		joints[selfIndex].children.push_back(childIndex);
	}

	return selfIndex;
}

Skeleton ANIME_MATH::CreateSkeleton(const Node& rootNode) {
	/// ----- ノードからスケルトンを作成 ----- ///
	Console::LogInfo("ANIME_MATH::CreateSkeleton: Starting skeleton creation.");

	Skeleton result;
	result.root = CreateJoint(rootNode, {}, result.joints);

	for (const Joint& joint : result.joints) {
		result.jointMap.emplace(joint.nameHash, joint.index);
	}

	Console::LogInfo(std::format("ANIME_MATH::CreateSkeleton: Created {} joints.", result.joints.size()));
	return result;
}

SkinCluster ANIME_MATH::CreateSkinCluster(const Skeleton& skeleton, Asset::Model* model, DxManager* dxm) {
	/// ----- スキンクラスターを作成 ----- ///
	Console::LogInfo(std::format("ANIME_MATH::CreateSkinCluster: Starting (Joints: {}, Meshes: {})", 
		skeleton.joints.size(), model->GetMeshes().size()));

	SkinCluster result{};

	DxDevice* dxDevice = dxm->GetDxDevice();
	ID3D12Device* device = dxDevice->GetDevice();

	/// matrix paletteの作成 (ボーンパレットは全メッシュ共通)
	result.paletteResource.CreateResource(dxDevice, sizeof(WellForGPU) * skeleton.joints.size());
	WellForGPU* mappedPalette = nullptr;
	result.paletteResource.Get()->Map(0, nullptr, reinterpret_cast<void**>(&mappedPalette));
	std::memset(mappedPalette, 0, sizeof(WellForGPU) * skeleton.joints.size());
	result.mappedPalette = { mappedPalette, skeleton.joints.size() };

	DxSRVHeap* pSRVHeap = dxm->GetDxSRVHeap();

	/// cpu,gpu handle get
	result.srvDescriptorIndex = pSRVHeap->AllocateBuffer();
	result.paletteSRVHandle.first = pSRVHeap->GetCPUDescriptorHandel(result.srvDescriptorIndex);
	result.paletteSRVHandle.second = pSRVHeap->GetGPUDescriptorHandel(result.srvDescriptorIndex);

	D3D12_SHADER_RESOURCE_VIEW_DESC paletteSRVDesc{};
	paletteSRVDesc.Format = DXGI_FORMAT_UNKNOWN;
	paletteSRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	paletteSRVDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	paletteSRVDesc.Buffer.FirstElement = 0;
	paletteSRVDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
	paletteSRVDesc.Buffer.NumElements = static_cast<UINT>(skeleton.joints.size());
	paletteSRVDesc.Buffer.StructureByteStride = sizeof(WellForGPU);

	device->CreateShaderResourceView(result.paletteResource.Get(), &paletteSRVDesc, result.paletteSRVHandle.first);


	/// resource create
	if (model->GetMeshes().empty()) {
		Console::LogError("ANIME_MATH::CreateSkinCluster: Model has no meshes.");
		return result;
	}

	size_t meshCount = model->GetMeshes().size();
	result.meshClusters.resize(meshCount);

	/// すべて単位行列で初期化
	result.matBindPoseInverseArray.resize(skeleton.joints.size());
	std::generate(
		result.matBindPoseInverseArray.begin(), result.matBindPoseInverseArray.end(),
		[]() {return Matrix4x4::kIdentity; }
	);

	// 1. 各メッシュのインフルエンスバッファ作成
	for (size_t meshIndex = 0; meshIndex < meshCount; ++meshIndex) {
		Asset::Model::ModelMesh* mesh = model->GetMeshes()[meshIndex].get();
		size_t vertexCount = mesh->GetVertices().size();
		auto& meshCluster = result.meshClusters[meshIndex];

		meshCluster.influenceResource.CreateResource(dxDevice, sizeof(VertexInfluence) * vertexCount);

		/// mapping
		VertexInfluence* mappedInfluence = nullptr;
		meshCluster.influenceResource.Get()->Map(0, nullptr, reinterpret_cast<void**>(&mappedInfluence));
		std::memset(mappedInfluence, 0, sizeof(VertexInfluence) * vertexCount);
		meshCluster.mappedInfluence = { mappedInfluence, vertexCount };

		/// vbvの作成
		meshCluster.vbv.BufferLocation = meshCluster.influenceResource.Get()->GetGPUVirtualAddress();
		meshCluster.vbv.SizeInBytes = static_cast<UINT>(sizeof(VertexInfluence) * vertexCount);
		meshCluster.vbv.StrideInBytes = sizeof(VertexInfluence);

		// 2. そのメッシュのウェイトデータを流し込む
		const auto& jointWeights = model->GetMeshJointWeightData()[meshIndex];

		for (const auto& [jointHash, weightData] : jointWeights) {
			auto itr = skeleton.jointMap.find(jointHash);
			if (itr == skeleton.jointMap.end()) continue;

			uint32_t skeletonJointIndex = itr->second;
			result.matBindPoseInverseArray[skeletonJointIndex] = weightData.matBindPoseInverse;

			for (const auto& vertexWeight : weightData.vertexWeights) {
				if (vertexWeight.vertexIndex >= vertexCount) {
					// Console::LogWarning(std::format("ANIME_MATH::CreateSkinCluster: Vertex index {} is out of bounds for mesh {} (size: {}).", vertexWeight.vertexIndex, meshIndex, vertexCount));
					continue;
				}

				VertexInfluence& influence = meshCluster.mappedInfluence[vertexWeight.vertexIndex];
				for (uint32_t i = 0u; i < kMaxInfluenceNumber; ++i) {
					if (influence.weights[i] == 0.0f) {
						influence.weights[i] = vertexWeight.weight;
						influence.jointIndices[i] = static_cast<int32_t>(skeletonJointIndex);
						break;
					}
				}
			}
		}
	}

	Console::LogInfo("ANIME_MATH::CreateSkinCluster: Finished.");

	return result;
}

void ANIME_MATH::SampleAnimation(const std::unordered_map<uint32_t, AnimationClip>& clips, const AnimationState& state, uint32_t jointNameHash, SampledTransform& outTransform) {
    auto itClip = clips.find(state.clipId);
    if (itClip == clips.end()) return;

    const AnimationClip& clip = itClip->second;

    auto itAnim = clip.nodeAnimationMap.find(jointNameHash);
    if (itAnim == clip.nodeAnimationMap.end()) return;

    const NodeAnimation& anim = itAnim->second;

    if (!anim.translate.empty()) outTransform.translate = CalculateValue(anim.translate, state.time);
    if (!anim.rotate.empty()) outTransform.rotate = CalculateValue(anim.rotate, state.time);
    if (!anim.scale.empty()) outTransform.scale = CalculateValue(anim.scale, state.time);
}
