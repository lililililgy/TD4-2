#include "Model.h"


namespace ONEngine::Asset {

Model::Model() = default;
Model::~Model() = default;

void Model::AddMesh(std::shared_ptr<ModelMesh>&& mesh) {
	meshes_.push_back(std::move(mesh));
}

Model::ModelMesh* Model::CreateMesh() {
	/// ----- 新規Meshを追加し、返す ----- ///
	meshes_.emplace_back(std::make_shared<ModelMesh>());
	return meshes_.back().get();
}

void Model::SetMeshes(std::vector<std::shared_ptr<ModelMesh>>&& meshes) {
	/// ----- 新しいMeshと今のMeshを入れ替える ----- ///
	if(meshes.size() > meshes_.size()) {
		meshes_.resize(meshes.size());
	}

	for(size_t i = 0; i < meshes.size(); ++i) {
		meshes_[i] = std::move(meshes[i]);
	}
}

const std::vector<std::shared_ptr<Model::ModelMesh>>& Model::GetMeshes() const {
	return meshes_;
}

std::vector<std::shared_ptr<Model::ModelMesh>>& Model::GetMeshes() {
	return meshes_;
}

void Model::SetPath(const std::string& path) {
	path_ = path;
}

void Model::SetRootNode(const Node& node) {
	rootNode_ = node;
}

const std::string& Model::GetPath() const {
	return path_;
}

const Node& Model::GetRootNode() const {
	return rootNode_;
}

const std::vector<std::unordered_map<uint32_t, JointWeightData>>& Model::GetMeshJointWeightData() const {
	return meshJointWeightData_;
}

std::vector<std::unordered_map<uint32_t, JointWeightData>>& Model::GetMeshJointWeightData() {
	return meshJointWeightData_;
}

const std::unordered_map<uint32_t, AnimationClip>& Model::GetAnimationClips() const {
	return animationClips_;
}

std::unordered_map<uint32_t, AnimationClip>& Model::GetAnimationClips() {
	return animationClips_;
}

} // namespace ONEngine::Asset