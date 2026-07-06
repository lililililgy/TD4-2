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

/// @brief 一度キリの再生に使う構造体
struct OneShotSE {
	std::string path;
	float volume;
	float pitch;
};

/// ////////////////////////////////////////////////////////////
/// SE Player
/// ////////////////////////////////////////////////////////////
class SEPlayer : public IComponent {
	friend class AudioPlaybackSystem;
public:
	SEPlayer();
	~SEPlayer();

	/// 再生
	void Play();
	void Stop();
	void PlayOneShot(float volume, float pitch, const std::string& path);

private:
	Asset::AudioClip* pAudioClip_ = nullptr;
	std::string path_;
	float volume_;
	float pitch_;

	int state_;
	bool isPlayingRequest_;
	bool isStopRequest_ = false;

	/// 再生中の音声ソースリスト
	std::list<IXAudio2SourceVoice*> sourceVoices_;

	/// ワンショット再生リクエストリスト
	std::vector<OneShotSE> oneShotRequests_;

public:
	/// ----- setter ----- ///
	void SetVolume(float volume);
	void SetPitch(float pitch);
	void SetAudioPath(const std::string& path);
	void SetAudioClip(Asset::AudioClip* clip);

	/// ----- getter ----- ///
	float GetVolume() const;
	float GetPitch() const;
	const std::string& GetAudioPath() const;
	Asset::AudioClip* GetAudioClip() const;
	int GetState() const;

};

namespace ComponentDebug {
void SEPlayerDebug(SEPlayer* se);
}

namespace MonoInternalMethods {
void SEPlayer_GetParams(uint64_t nativeHandle, float* volume, float* pitch);
void SEPlayer_SetParams(uint64_t nativeHandle, float volume, float pitch);
void SEPlayer_Play(uint64_t nativeHandle);
void SEPlayer_Stop(uint64_t nativeHandle);
void SEPlayer_PlayOneShot(uint64_t nativeHandle, float volume, float pitch, MonoString* path);
}

void from_json(const nlohmann::json& j, SEPlayer& a);
void to_json(nlohmann::json& j, const SEPlayer& a);

} /// ONEngine
