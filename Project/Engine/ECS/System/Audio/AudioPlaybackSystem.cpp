#include "AudioPlaybackSystem.h"


/// engine
#include "Engine/ECS/EntityComponentSystem/ECSGroup.h"
#include "Engine/ECS/Component/Components/ComputeComponents/Audio/AudioSource.h"
#include "Engine/Asset/Collection/AssetCollection.h"
#include "Engine/Asset/Assets/AudioClip/AudioClip.h"


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

AudioPlaybackSystem::~AudioPlaybackSystem() {}


void AudioPlaybackSystem::OutsideOfRuntimeUpdate(ECSGroup* /*ecs*/) {}

void AudioPlaybackSystem::RuntimeUpdate(ECSGroup* ecs) {
	/// AudioSourceコンポーネントの配列を取得、有効かチェック
	ComponentArray<AudioSource>* asArray = ecs->GetComponentArray<AudioSource>();
	if(!asArray || asArray->GetUsedComponents().empty()) {
		return;
	}

	for(auto& as : asArray->GetUsedComponents()) {
		/// 有効なAudioSourceのみ処理する
		if(!as || !as->enable) {
			continue;
		}

		/// 音のクリップを設定
		SetAudioClip(as);

		/// 音の再生リクエストチェック
		if(as->isPlayingRequest_) {
			/// 再生状態ではなければ再生する
			if(as->state_ != static_cast<int>(AudioState::Playing)) {
				PlayAudio(as);
			}
		}

		/// 音の停止リクエストチェック
		if(as->isStopRequest_) {
			as->isStopRequest_ = false;
			for(auto& voice : as->sourceVoices_) {
				if(voice) {
					voice->Stop();
					voice->FlushSourceBuffers();
				}
			}
			as->sourceVoices_.clear();
			as->state_ = static_cast<int>(AudioState::Stopped);
		}

		/// 音の状態を取得
		int state = GetAudioState(as);
		if(state != as->state_) {
			/// 状態が変わった場合は更新する
			as->state_ = state;
		}

		/// 再生中のボイスのパラメータを更新
		for(auto& voice : as->sourceVoices_) {
			if(voice) {
				voice->SetVolume(as->volume_);
				voice->SetFrequencyRatio(as->pitch_);
			}
		}


		/// OneShotAudioの再生リクエストチェック
		for(auto& req : as->oneShotAudioRequests_) {
			/// ワンショット再生
			Asset::AudioClip* clip = pAssetCollection_->GetAudioClip(req.path);
			if(clip) {
				PlayOneShot(clip, req.volume, req.pitch, req.path);
			}
		}

		/// ワンショット再生が終わった音声ソースを削除
		as->oneShotAudioRequests_.clear();

	}

}

void AudioPlaybackSystem::SetAudioClip(AudioSource* audioSource) {
	if(audioSource->path_.empty()) return;

	Asset::AudioClip* clip = pAssetCollection_->GetAudioClip(audioSource->path_);
	if(clip) {
		audioSource->pAudioClip_ = clip;
	} else {
		Console::LogError(std::format("[CPP Audio] Failed to load clip from path: {}", audioSource->path_));
	}
}

void AudioPlaybackSystem::PlayAudio(AudioSource* audioSource) {
	if(!audioSource->pAudioClip_) {
		Console::LogError("[CPP Audio] Cannot play - AudioClip is null");
		return;
	}

	Console::Log(std::format("[CPP Audio] Playing Sustained Sound: {}", audioSource->path_));
	if(audioSource->path_ == "") {
		return;
	}

	/// stateをPlayingに変更
	audioSource->state_ = static_cast<int>(AudioState::Playing);
	audioSource->isPlayingRequest_ = false;

	IXAudio2SourceVoice* sourceVoice = nullptr;
	sourceVoice = audioSource->pAudioClip_->CreateSourceVoice(xAudio2_.Get());

	/// 再生する波形データの設定
	const Asset::AudioStructs::SoundData& soundData = audioSource->pAudioClip_->GetSoundData();
	XAUDIO2_BUFFER buffer{};
	buffer.pAudioData = soundData.buffer.data();
	buffer.AudioBytes = static_cast<UINT32>(soundData.buffer.size());
	buffer.Flags = XAUDIO2_END_OF_STREAM;

	/// 波形データの再生
	sourceVoice->SubmitSourceBuffer(&buffer);
	sourceVoice->SetVolume(audioSource->volume_);
	sourceVoice->SetFrequencyRatio(audioSource->pitch_);
	sourceVoice->Start();

	/// 音声ソースをAudioSourceに追加
	audioSource->sourceVoices_.push_back(sourceVoice);
}

void AudioPlaybackSystem::PlayOneShot(Asset::AudioClip* audioClip, float volume, float pitch, const std::string& /*path*/) {
	IXAudio2SourceVoice* sourceVoice = nullptr;
	sourceVoice = audioClip->CreateSourceVoice(xAudio2_.Get());

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

	/// 音声ソースをAudioSourceに追加
	oneShotAudios_.push_back(sourceVoice);
}

int AudioPlaybackSystem::GetAudioState(AudioSource* audioSource) {
	Asset::AudioClip* clip = audioSource->pAudioClip_;
	if(!clip) {
		// クリップが設定されていない場合は停止状態
		return static_cast<int>(AudioState::Stopped);
	}


	std::list<IXAudio2SourceVoice*>& sourceVoices = audioSource->sourceVoices_;
	for(auto itr = sourceVoices.begin(); itr != sourceVoices.end();) {
		IXAudio2SourceVoice* sourceVoice = *itr;
		if(!sourceVoice) {
			itr = sourceVoices.erase(itr);
			continue;
		}

		XAUDIO2_VOICE_STATE state;
		sourceVoice->GetState(&state);
		if(state.BuffersQueued == 0) {
			itr = sourceVoices.erase(itr);
			continue;
		}

		itr++;
	}

	if(sourceVoices.size() > 0) {
		/// 再生中の音声ソースがある場合は再生中
		return static_cast<int>(AudioState::Playing);
	}

	/// 再生中の音声ソースがない場合は停止
	return static_cast<int>(AudioState::Stopped);
}

}
