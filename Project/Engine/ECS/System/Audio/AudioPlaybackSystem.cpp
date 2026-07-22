#include "AudioPlaybackSystem.h"

/// engine
#include "Engine/Core/Config/EngineConfig.h"
#include "Engine/ECS/EntityComponentSystem/ECSGroup.h"
#include "Engine/ECS/Component/Components/ComputeComponents/Audio/BGMPlayer.h"
#include "Engine/ECS/Component/Components/ComputeComponents/Audio/SEPlayer.h"
#include "Engine/Asset/Collection/AssetCollection.h"
#include "Engine/Asset/Assets/AudioClip/AudioClip.h"
#include "Engine/ECS/EntityComponentSystem/EntityComponentSystem.h"

namespace ONEngine {

AudioPlaybackSystem::AudioPlaybackSystem(Asset::AssetCollection* assetCollection)
	: pAssetCollection_(assetCollection) {

	HRESULT hr = S_FALSE;

	/// xAudioインスタンスの生成
	hr = XAudio2Create(&xAudio2_, 0, XAUDIO2_DEFAULT_PROCESSOR);
	Assert(SUCCEEDED(hr));

	/// MasterVoiceの生成
	hr = xAudio2_->CreateMasteringVoice(&masterVoice_);
	Assert(SUCCEEDED(hr));
}

AudioPlaybackSystem::~AudioPlaybackSystem() {
	// 1. すべてのソースボイスを確実に停止・破棄する
	StopAllAudio();

	// 2. マスタリングボイスを破棄
	if (masterVoice_) {
		masterVoice_->DestroyVoice();
		masterVoice_ = nullptr;
	}

	// 3. XAudio2 インスタンスの解放
	xAudio2_.Reset();
}

void AudioPlaybackSystem::StopAllAudio() {
	// すべてのソースボイスを停止・破棄する
	for (auto voice : allSourceVoices_) {
		if (voice) {
			voice->Stop();
			voice->DestroyVoice();
		}
	}
	allSourceVoices_.clear();

	activeBGMVoice_ = nullptr;
	activeBGMPlayer_ = nullptr;
	oneShotSEVoices_.clear();
	bgmLoopExited_ = false;
}

void AudioPlaybackSystem::PauseAllAudio() {
	for (auto voice : allSourceVoices_) {
		if (voice) {
			voice->Stop(0);
		}
	}
}

void AudioPlaybackSystem::ResumeAllAudio() {
	for (auto voice : allSourceVoices_) {
		if (voice) {
			voice->Start(0);
		}
	}
}


void AudioPlaybackSystem::OutsideOfRuntimeUpdate(ECSGroup* ecs) {
#ifdef DEBUG_MODE
	// シーン再生中は、停止時用のクリーンアップやhasPlayedOnAwake_のリセットを行わない
	if (DebugConfig::isDebugging) {
		return;
	}
#else
	// Releaseビルドでは常に実行状態のため、Outsideでのクリーンアップやリセットはスキップする
	return;
#endif

	// 再生中だったBGMのクリーンアップ
	if (activeBGMVoice_) {
		activeBGMVoice_->Stop();
		allSourceVoices_.remove(activeBGMVoice_);
		activeBGMVoice_->DestroyVoice();
		activeBGMVoice_ = nullptr;
	}
	if (activeBGMPlayer_) {
		activeBGMPlayer_->state_ = static_cast<int>(AudioState::Stopped);
		activeBGMPlayer_->sourceVoice_ = nullptr;
		activeBGMPlayer_ = nullptr;
	}
	bgmLoopExited_ = false;

	// OneShotSEのクリーンアップ
	for (auto voice : oneShotSEVoices_) {
		if (voice) {
			voice->Stop();
			allSourceVoices_.remove(voice);
			voice->DestroyVoice();
		}
	}
	oneShotSEVoices_.clear();

	// BGMPlayerの再生フラグ・状態のリセット
	ComponentArray<BGMPlayer>* bgmArray = ecs->GetComponentArray<BGMPlayer>();
	if (bgmArray && !bgmArray->GetUsedComponents().empty()) {
		for (auto& bgm : bgmArray->GetUsedComponents()) {
			if (bgm) {
				bgm->hasPlayedOnAwake_ = false;
				bgm->state_ = static_cast<int>(AudioState::Stopped);
				bgm->sourceVoice_ = nullptr;
			}
		}
	}

	// SEPlayerのボイスのクリーンアップと状態のリセット
	ComponentArray<SEPlayer>* seArray = ecs->GetComponentArray<SEPlayer>();
	if (seArray && !seArray->GetUsedComponents().empty()) {
		for (auto& se : seArray->GetUsedComponents()) {
			if (se) {
				for (auto& voice : se->sourceVoices_) {
					if (voice) {
						voice->Stop();
						allSourceVoices_.remove(voice);
						voice->DestroyVoice();
					}
				}
				se->sourceVoices_.clear();
				se->state_ = static_cast<int>(AudioState::Stopped);
			}
		}
	}
}

void AudioPlaybackSystem::RuntimeUpdate(ECSGroup* ecs) {
	// ==========================================
	// 0. アクティブなBGMプレイヤーの生存確認（シーン遷移や破棄の検知）
	// ==========================================
	if (activeBGMPlayer_ && activeBGMVoice_) {
		bool exists = false;
		ComponentArray<BGMPlayer>* bgmArray = ecs->GetComponentArray<BGMPlayer>();
		if (bgmArray) {
			for (auto& bgm : bgmArray->GetUsedComponents()) {
				if (bgm == activeBGMPlayer_) {
					exists = true;
					break;
				}
			}
		}

		if (!exists) {
			activeBGMVoice_->Stop();
			allSourceVoices_.remove(activeBGMVoice_);
			activeBGMVoice_->DestroyVoice();
			activeBGMVoice_ = nullptr;
			activeBGMPlayer_ = nullptr;
			bgmLoopExited_ = false;
			Console::Log("[CPP Audio] BGM stopped because the source BGMPlayer was destroyed.");
		}
	}

	// ==========================================
	// 1. BGMPlayer の処理
	// ==========================================
	ComponentArray<BGMPlayer>* bgmArray = ecs->GetComponentArray<BGMPlayer>();
	if (bgmArray && !bgmArray->GetUsedComponents().empty()) {
		for (auto& bgm : bgmArray->GetUsedComponents()) {
			if (!CheckComponentEnable(bgm)) {
				continue;
			}

			// Play On Awakeの判定
			if (bgm->playOnAwake_ && !bgm->hasPlayedOnAwake_) {
				bgm->hasPlayedOnAwake_ = true;
				bgm->Play();
			}

			/// 音のクリップを設定
			SetBGMAudioClip(bgm);

			/// 再生リクエスト
			if (bgm->isPlayingRequest_) {
				bgm->isPlayingRequest_ = false;

				// 他のBGMが再生中なら停止して破棄
				if (activeBGMVoice_) {
					activeBGMVoice_->Stop();
					allSourceVoices_.remove(activeBGMVoice_);
					activeBGMVoice_->DestroyVoice();
					activeBGMVoice_ = nullptr;
				}
				if (activeBGMPlayer_) {
					activeBGMPlayer_->state_ = static_cast<int>(AudioState::Stopped);
					activeBGMPlayer_->sourceVoice_ = nullptr;
					activeBGMPlayer_ = nullptr;
				}

				PlayBGM(bgm);
			}

			/// 停止リクエスト
			if (bgm->isStopRequest_) {
				bgm->isStopRequest_ = false;
				if (bgm == activeBGMPlayer_ && activeBGMVoice_) {
					activeBGMVoice_->Stop();
					allSourceVoices_.remove(activeBGMVoice_);
					activeBGMVoice_->DestroyVoice();
					activeBGMVoice_ = nullptr;
					bgm->sourceVoice_ = nullptr;
					activeBGMPlayer_ = nullptr;
					bgmLoopExited_ = false;
				}
				bgm->state_ = static_cast<int>(AudioState::Stopped);
			}

			/// 状態の取得
			int state = GetBGMState(bgm);
			if (state != bgm->state_) {
				bgm->state_ = state;
			}

			/// パラメータの更新
			if (bgm == activeBGMPlayer_ && activeBGMVoice_) {
				activeBGMVoice_->SetVolume(bgm->volume_);
				activeBGMVoice_->SetFrequencyRatio(bgm->pitch_);

				// ループが途中で解除された場合はExitLoopを呼び出す
				if (!bgm->isLoop_ && !bgmLoopExited_) {
					activeBGMVoice_->ExitLoop();
					bgmLoopExited_ = true;
					Console::Log("[CPP Audio] BGM Loop exited dynamically.");
				}
			}
		}
	}

	// ==========================================
	// 2. SEPlayer の処理
	// ==========================================
	ComponentArray<SEPlayer>* seArray = ecs->GetComponentArray<SEPlayer>();
	if (seArray && !seArray->GetUsedComponents().empty()) {
		for (auto& se : seArray->GetUsedComponents()) {
			if (!CheckComponentEnable(se)) {
				continue;
			}

			/// 音のクリップを設定
			SetSEAudioClip(se);

			/// 再生リクエスト
			if (se->isPlayingRequest_) {
				se->isPlayingRequest_ = false;
				PlaySE(se);
			}

			/// 停止リクエスト
			if (se->isStopRequest_) {
				se->isStopRequest_ = false;
				for (auto& voice : se->sourceVoices_) {
					if (voice) {
						voice->Stop();
						allSourceVoices_.remove(voice);
						voice->DestroyVoice();
					}
				}
				se->sourceVoices_.clear();
				se->state_ = static_cast<int>(AudioState::Stopped);
			}

			/// 状態の取得と完了したボイスの破棄
			int state = GetSEState(se);
			if (state != se->state_) {
				se->state_ = state;
			}

			/// 再生中のボイスのパラメータ更新
			for (auto& voice : se->sourceVoices_) {
				if (voice) {
					voice->SetVolume(se->volume_);
					voice->SetFrequencyRatio(se->pitch_);
				}
			}

			/// OneShotSEの再生リクエスト
			for (auto& req : se->oneShotRequests_) {
				Asset::AudioClip* clip = pAssetCollection_->GetAudioClip(req.path);
				if (clip) {
					PlayOneShotSE(clip, req.volume, req.pitch, req.path);
				}
			}
			se->oneShotRequests_.clear();
		}
	}

	// ==========================================
	// 3. ワンショットSEのクリーンアップ
	// ==========================================
	for (auto itr = oneShotSEVoices_.begin(); itr != oneShotSEVoices_.end(); ) {
		IXAudio2SourceVoice* voice = *itr;
		if (!voice) {
			itr = oneShotSEVoices_.erase(itr);
			continue;
		}

		XAUDIO2_VOICE_STATE state;
		voice->GetState(&state);
		if (state.BuffersQueued == 0) {
			allSourceVoices_.remove(voice);
			voice->DestroyVoice();
			itr = oneShotSEVoices_.erase(itr);
			continue;
		}
		itr++;
	}
}

void AudioPlaybackSystem::SetBGMAudioClip(BGMPlayer* bgm) {
	if (bgm->path_.empty()) return;

	Asset::AudioClip* clip = pAssetCollection_->GetAudioClip(bgm->path_);
	if (clip) {
		bgm->pAudioClip_ = clip;
	} else {
		Console::LogError(std::format("[CPP Audio] Failed to load BGM clip from path: {}", bgm->path_));
	}
}

void AudioPlaybackSystem::SetSEAudioClip(SEPlayer* se) {
	if (se->path_.empty()) return;

	Asset::AudioClip* clip = pAssetCollection_->GetAudioClip(se->path_);
	if (clip) {
		se->pAudioClip_ = clip;
	} else {
		Console::LogError(std::format("[CPP Audio] Failed to load SE clip from path: {}", se->path_));
	}
}

void AudioPlaybackSystem::PlayBGM(BGMPlayer* bgm) {
	if (!bgm->pAudioClip_) {
		Console::LogError("[CPP Audio] Cannot play BGM - AudioClip is null");
		return;
	}

	if (bgm->path_ == "") {
		return;
	}

	bgm->state_ = static_cast<int>(AudioState::Playing);
	bgm->isPlayingRequest_ = false;

	IXAudio2SourceVoice* sourceVoice = nullptr;
	sourceVoice = bgm->pAudioClip_->CreateSourceVoice(xAudio2_.Get());
	if (!sourceVoice) return;

	allSourceVoices_.push_back(sourceVoice);

	/// 再生する波形データの設定
	const Asset::AudioStructs::SoundData& soundData = bgm->pAudioClip_->GetSoundData();
	XAUDIO2_BUFFER buffer{};
	buffer.pAudioData = soundData.buffer.data();
	buffer.AudioBytes = static_cast<UINT32>(soundData.buffer.size());
	buffer.Flags = XAUDIO2_END_OF_STREAM;
	if (bgm->isLoop_) {
		buffer.LoopCount = XAUDIO2_LOOP_INFINITE;
	}

	/// 波形データの再生
	sourceVoice->SubmitSourceBuffer(&buffer);
	sourceVoice->SetVolume(bgm->volume_);
	sourceVoice->SetFrequencyRatio(bgm->pitch_);
	sourceVoice->Start();

	bgm->sourceVoice_ = sourceVoice;
	activeBGMVoice_ = sourceVoice;
	activeBGMPlayer_ = bgm;
	bgmLoopExited_ = !bgm->isLoop_;
}

void AudioPlaybackSystem::PlaySE(SEPlayer* se) {
	if (!se->pAudioClip_) {
		Console::LogError("[CPP Audio] Cannot play SE - AudioClip is null");
		return;
	}

	if (se->path_ == "") {
		return;
	}

	se->state_ = static_cast<int>(AudioState::Playing);
	se->isPlayingRequest_ = false;

	IXAudio2SourceVoice* sourceVoice = nullptr;
	sourceVoice = se->pAudioClip_->CreateSourceVoice(xAudio2_.Get());
	if (!sourceVoice) return;

	allSourceVoices_.push_back(sourceVoice);

	/// 再生する波形データの設定
	const Asset::AudioStructs::SoundData& soundData = se->pAudioClip_->GetSoundData();
	XAUDIO2_BUFFER buffer{};
	buffer.pAudioData = soundData.buffer.data();
	buffer.AudioBytes = static_cast<UINT32>(soundData.buffer.size());
	buffer.Flags = XAUDIO2_END_OF_STREAM;

	/// 波形データの再生
	sourceVoice->SubmitSourceBuffer(&buffer);
	sourceVoice->SetVolume(se->volume_);
	sourceVoice->SetFrequencyRatio(se->pitch_);
	sourceVoice->Start();

	se->sourceVoices_.push_back(sourceVoice);
}

void AudioPlaybackSystem::PlayOneShotSE(Asset::AudioClip* audioClip, float volume, float pitch, const std::string& /*path*/) {
	IXAudio2SourceVoice* sourceVoice = nullptr;
	sourceVoice = audioClip->CreateSourceVoice(xAudio2_.Get());
	if (!sourceVoice) return;

	allSourceVoices_.push_back(sourceVoice);

	/// 再生する波形データの設定
	const Asset::AudioStructs::SoundData& soundData = audioClip->GetSoundData();
	XAUDIO2_BUFFER buffer{};
	buffer.pAudioData = soundData.buffer.data();
	buffer.AudioBytes = static_cast<UINT32>(soundData.buffer.size());
	buffer.Flags = XAUDIO2_END_OF_STREAM;

	/// 波形データの再生
	sourceVoice->SubmitSourceBuffer(&buffer);
	sourceVoice->SetVolume(volume);
	sourceVoice->SetFrequencyRatio(pitch);
	sourceVoice->Start();

	oneShotSEVoices_.push_back(sourceVoice);
}

int AudioPlaybackSystem::GetBGMState(BGMPlayer* bgm) {
	if (!bgm->pAudioClip_ || !bgm->sourceVoice_) {
		return static_cast<int>(AudioState::Stopped);
	}

	XAUDIO2_VOICE_STATE state;
	bgm->sourceVoice_->GetState(&state);
	if (state.BuffersQueued == 0) {
		// ループなしで再生終了した場合など
		allSourceVoices_.remove(bgm->sourceVoice_);
		bgm->sourceVoice_->DestroyVoice();
		bgm->sourceVoice_ = nullptr;
		if (bgm == activeBGMPlayer_) {
			activeBGMVoice_ = nullptr;
			activeBGMPlayer_ = nullptr;
			bgmLoopExited_ = false;
		}
		return static_cast<int>(AudioState::Stopped);
	}

	return static_cast<int>(AudioState::Playing);
}

int AudioPlaybackSystem::GetSEState(SEPlayer* se) {
	std::list<IXAudio2SourceVoice*>& sourceVoices = se->sourceVoices_;
	for (auto itr = sourceVoices.begin(); itr != sourceVoices.end(); ) {
		IXAudio2SourceVoice* sourceVoice = *itr;
		if (!sourceVoice) {
			itr = sourceVoices.erase(itr);
			continue;
		}

		XAUDIO2_VOICE_STATE state;
		sourceVoice->GetState(&state);
		if (state.BuffersQueued == 0) {
			allSourceVoices_.remove(sourceVoice);
			sourceVoice->DestroyVoice(); // クリーンアップ
			itr = sourceVoices.erase(itr);
			continue;
		}

		itr++;
	}

	if (!sourceVoices.empty()) {
		return static_cast<int>(AudioState::Playing);
	}

	return static_cast<int>(AudioState::Stopped);
}

}
