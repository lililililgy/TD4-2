#pragma once

/// std
#include <string>
#include <vector>
#include <unordered_map>
#include <fstream>
#include <filesystem>
#include <cstdint>
#include <shared_mutex> 

/// engine
#include "Engine/Asset/Assets/IAsset.h"
#include "Engine/Asset/Guid/Guid.h"
#include "Engine/Asset/Meta/MetaFile.h"

namespace ONEngine::Asset {

/// ///////////////////////////////////////////////////
/// アセットのインターフェイスクラス
/// ///////////////////////////////////////////////////
class IAssetContainer {
public:
	virtual ~IAssetContainer() = default;
};

/// ///////////////////////////////////////////////////
/// リソースのコンテナクラス
/// ///////////////////////////////////////////////////
template <IsAsset T>
class AssetContainer : public IAssetContainer {
public:
	/// ===================================================
	/// public : methods
	/// ===================================================

	AssetContainer(size_t maxResourceSize);
	~AssetContainer();

	/// 追加
	T* Add(const std::string& key, T t);

	/// 削除
	void Remove(const std::string& key);
	void Remove(int32_t index);


	/// --------------- 取得用 --------------- ///

	T* Get(const std::string& key);
	T* Get(int32_t index);
	T* GetFirst();

	const std::string& GetKey(int32_t index) const;

	int32_t GetIndex(const std::string& key) const;
	int32_t GetIndex(const Guid& guid) const;

	const std::vector<T>& GetValues() const;
	std::vector<T>& GetValues();

	const std::unordered_map<std::string, int32_t>& GetIndexMap() const;

	const Guid& GetGuid(const std::string& key) const;
	const Guid& GetGuid(int32_t index) const;

private:
	/// ===================================================
	/// private : objects
	/// ===================================================

	mutable std::shared_mutex mtx_;

	std::unordered_map<std::string, int32_t> indexMap_;
	std::unordered_map<int32_t, std::string> reverseIndexMap_;

	std::unordered_map<Guid, int32_t> guidToIndexMap_;
	std::unordered_map<int32_t, Guid> indexToGuidMap_;

	std::vector<T> values_;

	int32_t nextIndex_ = 0;
	std::vector<int32_t> freeIndices_;
};

/// ///////////////////////////////////////////////////
/// methods
/// ///////////////////////////////////////////////////

template<IsAsset T>
inline AssetContainer<T>::AssetContainer(size_t maxResourceSize) : nextIndex_(0) {
	values_.resize(maxResourceSize);
}

template<IsAsset T>
inline AssetContainer<T>::~AssetContainer() {}

template<IsAsset T>
inline T* AssetContainer<T>::Add(const std::string& key, T t) {
	std::unique_lock<std::shared_mutex> lock(mtx_);

	if(indexMap_.contains(key)) {
		uint32_t index = indexMap_[key];
		values_[index] = std::move(t);
		return &values_[index];
	}

	uint32_t index;
	if (!freeIndices_.empty()) {
		index = freeIndices_.back();
		freeIndices_.pop_back();
	} else {
		index = static_cast<uint32_t>(nextIndex_++);
	}

	indexMap_[key] = index;
	reverseIndexMap_[index] = key;

	guidToIndexMap_[t.guid] = index;
	indexToGuidMap_[index] = t.guid;

	//if(std::filesystem::exists(key + ".meta")) {
	//	MetaFile metaFile;
	//	metaFile.LoadFromFile(key + ".meta");
	//	Guid& guid = metaFile.guid;
	//	guidToIndexMap_[guid] = index;
	//	indexToGuidMap_[index] = guid;
	//	t.guid = guid;
	//} else {
	//	MetaFile metaFile = GenerateMetaFile(key);
	//	Guid& guid = metaFile.guid;
	//	guidToIndexMap_[guid] = index;
	//	indexToGuidMap_[index] = guid;
	//	t.guid = guid;
	//}

	values_[index] = std::move(t);
	return &values_[index];
}

template<IsAsset T>
inline void AssetContainer<T>::Remove(const std::string& key) {
	std::unique_lock<std::shared_mutex> lock(mtx_);

	if(indexMap_.contains(key)) {
		uint32_t index = indexMap_[key];
		
		if (indexToGuidMap_.contains(index)) {
			guidToIndexMap_.erase(indexToGuidMap_[index]);
			indexToGuidMap_.erase(index);
		}
		
		indexMap_.erase(key);
		reverseIndexMap_.erase(index);
		
		values_[index] = T();
		freeIndices_.push_back(index);
	}
}

template<IsAsset T>
inline void AssetContainer<T>::Remove(int32_t index) {
	std::unique_lock<std::shared_mutex> lock(mtx_);

	if(reverseIndexMap_.contains(index)) {
		std::string key = reverseIndexMap_[index];
		
		if (indexToGuidMap_.contains(index)) {
			guidToIndexMap_.erase(indexToGuidMap_[index]);
			indexToGuidMap_.erase(index);
		}
		
		indexMap_.erase(key);
		reverseIndexMap_.erase(index);
		
		values_[index] = T();
		freeIndices_.push_back(index);
	}
}

template<IsAsset T>
inline T* AssetContainer<T>::Get(const std::string& key) {
	std::shared_lock<std::shared_mutex> lock(mtx_);

	if(indexMap_.contains(key)) {
		uint32_t index = indexMap_[key];
		return &values_[index];
	}
	return nullptr;
}

template<IsAsset T>
inline T* AssetContainer<T>::Get(int32_t index) {
	std::shared_lock<std::shared_mutex> lock(mtx_);

	if(index < values_.size()) {
		return &values_[index];
	}
	return nullptr;
}

template<IsAsset T>
inline T* AssetContainer<T>::GetFirst() {
	std::shared_lock<std::shared_mutex> lock(mtx_);
	return &values_.front();
}

template<IsAsset T>
inline const std::string& AssetContainer<T>::GetKey(int32_t index) const {
	std::shared_lock<std::shared_mutex> lock(mtx_);

	if(reverseIndexMap_.contains(index)) {
		return reverseIndexMap_.at(index);
	}
	static const std::string emptyString;
	return emptyString;
}

template<IsAsset T>
inline int32_t AssetContainer<T>::GetIndex(const std::string& key) const {
	std::shared_lock<std::shared_mutex> lock(mtx_);

	if(indexMap_.contains(key)) {
		return indexMap_.at(key);
	}
	return -1;
}

template<IsAsset T>
inline int32_t AssetContainer<T>::GetIndex(const Guid& guid) const {
	std::shared_lock<std::shared_mutex> lock(mtx_);

	if(guidToIndexMap_.contains(guid)) {
		return guidToIndexMap_.at(guid);
	}
	return -1;
}

template<IsAsset T>
inline const std::vector<T>& AssetContainer<T>::GetValues() const {
	std::shared_lock<std::shared_mutex> lock(mtx_);
	return values_;
}

template<IsAsset T>
inline std::vector<T>& AssetContainer<T>::GetValues() {
	std::shared_lock<std::shared_mutex> lock(mtx_);
	return values_;
}

template<IsAsset T>
inline const std::unordered_map<std::string, int32_t>& AssetContainer<T>::GetIndexMap() const {
	std::shared_lock<std::shared_mutex> lock(mtx_);
	return indexMap_;
}

template<IsAsset T>
inline const Guid& AssetContainer<T>::GetGuid(const std::string& key) const {
	std::shared_lock<std::shared_mutex> lock(mtx_);

	if(indexMap_.contains(key)) {
		int32_t index = indexMap_.at(key);
		if (indexToGuidMap_.contains(index)) {
			return indexToGuidMap_.at(index);
		}
	}
	return Guid::kInvalid;
}

template<IsAsset T>
inline const Guid& AssetContainer<T>::GetGuid(int32_t index) const {
	std::shared_lock<std::shared_mutex> lock(mtx_);
	if (indexToGuidMap_.contains(index)) {
		return indexToGuidMap_.at(index);
	}
	return Guid::kInvalid;
}

} /// namespace ONEngine::Asset