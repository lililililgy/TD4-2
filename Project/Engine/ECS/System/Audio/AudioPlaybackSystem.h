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
class BGMPlayer;
class SEPlayer;
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
	void SetBGMAudioClip(BGMPlayer* bgm);
	void SetSEAudioClip(SEPlayer* se);

	/// 再生
	void PlayBGM(BGMPlayer* bgm);
	void PlaySE(SEPlayer* se);
	void PlayOneShotSE(Asset::AudioClip* audioClip, float volume, float pitch, const std::string& path);

	/// 状態の取得
	int GetBGMState(BGMPlayer* bgm);
	int GetSEState(SEPlayer* se);

private:
	/// ===================================================
	/// private : objects
	/// ===================================================

	/// other classes
	Asset::AssetCollection* pAssetCollection_ = nullptr;

	/// xAudio
	ComPtr<IXAudio2> xAudio2_ = nullptr;
	IXAudio2MasteringVoice* masterVoice_ = nullptr;

	/// BGM (システム全体でアクティブなBGMは常に1つ)
	IXAudio2SourceVoice* activeBGMVoice_ = nullptr;
	BGMPlayer* activeBGMPlayer_ = nullptr;

	/// SE (OneShotSEで再生されたボイス)
	std::list<IXAudio2SourceVoice*> oneShotSEVoices_;

	float masterVolume_;

};


} /// ONEngine
