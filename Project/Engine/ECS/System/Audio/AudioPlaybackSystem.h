#pragma once

/// std
#include <list>
#include <string>

/// audio
#include <xaudio2.h>

/// engine
#include "../Interface/ECSISystem.h"
#include "Engine/Core/DirectX12/ComPtr/ComPtr.h"

namespace ONEngine {
class ECSGroup;
class AudioSource;
}

namespace ONEngine::Asset {
class AssetCollection;
class AudioClip;
}


/// ////////////////////////////////////////////////////////////
/// 音の再生を行うクラス
/// ////////////////////////////////////////////////////////////
namespace ONEngine {

class AudioPlaybackSystem : public ECSISystem {
public:
	/// ===================================================
	/// public : methods
	/// ===================================================

	AudioPlaybackSystem(Asset::AssetCollection* assetCollection);
	~AudioPlaybackSystem();

	void OutsideOfRuntimeUpdate(ECSGroup* ecs) override;
	void RuntimeUpdate(ECSGroup* ecs) override;

private:
	/// ==================================================
	/// private : methods
	/// ==================================================


	/// 設定
	void SetAudioClip(AudioSource* audioSource);

	/// 再生
	void PlayAudio(AudioSource* audioSource);
	void PlayOneShot(Asset::AudioClip* audioClip, float volume, float pitch, const std::string& path);

	/// 状態の取得
	int GetAudioState(AudioSource* audioSource);

private:
	/// ===================================================
	/// private : methods
	/// ===================================================

	/// other classes
	Asset::AssetCollection* pAssetCollection_ = nullptr;

	/// xAudio
	ComPtr<IXAudio2> xAudio2_ = nullptr;
	IXAudio2MasteringVoice* masterVoice_ = nullptr;

	/// one shot audios
	std::list<IXAudio2SourceVoice*> oneShotAudios_;

	float masterVolume_;

};


} /// ONEngine
