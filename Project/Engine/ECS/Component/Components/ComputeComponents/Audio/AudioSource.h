#pragma once

/// std
#include <string>
#include <list>
#include <vector>

/// audio
#include <xaudio2.h>

/// external
#include <nlohmann/json.hpp>
#include <mono/jit/jit.h>

/// engine
#include "../../Interface/IComponent.h"

namespace ONEngine::Asset {
class AudioClip;
}


namespace ONEngine {

/// @brief 音の再生状態
enum class AudioState {
	Stopped,
	Playing,
	Paused,
};

/// @brief 一度キリの再生に使う構造体
struct OneShotAudio {
	std::string path;
	float volume;
	float pitch;
};

/// ////////////////////////////////////////////////////////////
/// Audio Source
/// ////////////////////////////////////////////////////////////
class AudioSource : public IComponent {
	friend class AudioPlaybackSystem;
public:
	/// ===================================================
	/// public : methods
	/// ===================================================

	AudioSource();
	~AudioSource();

	/// 再生
	void Play();
	void Stop();
	void PlayOneShot(float volume, float pitch, const std::string& path);

	/// 追加
	void AddSourceVoice(IXAudio2SourceVoice* sourceVoice);

private:
	/// ===================================================
	/// private : objects
	/// ===================================================

	Asset::AudioClip* pAudioClip_;
	std::string path_;
	float volume_;
	float pitch_;

	int state_;
	bool isPlayingRequest_;
	bool isStopRequest_ = false;
	bool isLoop_ = false;

	/// 再生中の音声ソースリスト
	std::list<IXAudio2SourceVoice*> sourceVoices_;

	/// ワンショット再生リクエストリスト
	std::vector<OneShotAudio> oneShotAudioRequests_;

public:
	/// ===================================================
	/// public : methods
	/// ===================================================

	/// ----- setter ----- ///
	void SetVolume(float volume);
	void SetPitch(float pitch);
	void SetAudioPath(const std::string& path);
	void SetAudioClip(Asset::AudioClip* clip);

	/// ----- getter ----- ///
	float GetVolume() const;
	float GetPitch() const;
	bool GetLoop() const { return isLoop_; }
	void SetLoop(bool loop) { isLoop_ = loop; }
	const std::string& GetAudioPath() const;
	Asset::AudioClip* GetAudioClip() const;
	int GetState() const;

};

namespace ComponentDebug {
void AudioSourceDebug(AudioSource* as);
}

namespace MonoInternalMethods {
void InternalGetParams(uint64_t nativeHandle, float* volume, float* pitch);
void InternalSetParams(uint64_t nativeHandle, float volume, float pitch);
void InternalPlay(uint64_t nativeHandle);
void InternalStop(uint64_t nativeHandle);
void InternalPlayOneShot(uint64_t nativeHandle, float volume, float pitch, MonoString* path);
}



void from_json(const nlohmann::json& j, AudioSource& a);
void to_json(nlohmann::json& j, const AudioSource& a);

} /// ONEngine
