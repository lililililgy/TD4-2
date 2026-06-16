#pragma once

/// std
#include <memory>
#include <future>

/// engine
#include "Engine/Asset/Assets/IAsset.h"
#include "Engine/Asset/Assets/IAssetLoader.h"
#include "Engine/Asset/Collection/Container/AssetContainer.h"
#include "Engine/Core/Threading/ThreadPool.h" 

namespace ONEngine::Asset {

class IAssetBundle {
public:
	virtual ~IAssetBundle() = default;

	virtual void Load(const std::string& filepath) = 0;
	virtual std::future<void> LoadAsync(const std::string& filepath) = 0;
	virtual void Reload(const std::string& filepath) = 0;
	virtual const Guid& GetGuid(const std::string& filepath) const = 0;
	virtual void Remove(const std::string& filepath) = 0;
	virtual bool Contains(const Guid& guid) const = 0;
	virtual bool Contains(const std::string& filepath) const = 0;
};

template <IsAsset T>
class AssetBundle : public IAssetBundle {
public:

	AssetBundle() = default;
	~AssetBundle() override = default;

	std::unique_ptr<AssetLoader<T>> loader;
	std::unique_ptr<AssetContainer<T>> container;

	void Load(const std::string& filepath) override {

		/// キャッシュ確認
		if(container->GetIndex(filepath) == -1) {

			/// Metaファイル読み込み
			Meta<T::MetaData> meta = loader->GetMetaData(filepath);

			/// ロード&追加
			auto asset = loader->Load(filepath, meta);
			if(asset.has_value()) {
				container->Add(filepath, std::move(asset.value()));
			}
		}
	}

	std::future<void> LoadAsync(const std::string& filepath) override {
		return ThreadPool::Instance().Enqueue([this, filepath]() {
			if(container->GetIndex(filepath) == -1) {
				Meta<T::MetaData> meta = loader->GetMetaData(filepath);
				auto asset = loader->Load(filepath, meta);
				if(asset.has_value()) {
					container->Add(filepath, std::move(asset.value()));
				}
			}
		});
	}

	void Reload(const std::string& filepath) override {
		int32_t index = container->GetIndex(filepath);
		if(index != -1) {
			T* src = container->Get(index);
			Meta<T::MetaData> meta = loader->GetMetaData(filepath);
			auto reloadedAsset = loader->Reload(filepath, src, meta);
			if(reloadedAsset.has_value()) {
				container->Add(filepath, std::move(reloadedAsset.value()));
			}
		}
	}

	const Guid& GetGuid(const std::string& filepath) const override {
		return container->GetGuid(filepath);
	}

	void Remove(const std::string& filepath) override {
		container->Remove(filepath);
	}

	bool Contains(const Guid& guid) const override {
		return container->GetIndex(guid) != -1;
	}

	bool Contains(const std::string& filepath) const override {
		return container->GetIndex(filepath) != -1;
	}

};

} /// namespace ONEngine