#pragma once

/// std
#include <string>

/// audio
#include <xaudio2.h>

/// external
#include <nlohmann/json.hpp>

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

/// ////////////////////////////////////////////////////////////
/// BGM Player
/// ////////////////////////////////////////////////////////////
class BGMPlayer : public IComponent {
	friend class AudioPlaybackSystem;
public:
	BGMPlayer();
	~BGMPlayer();

	/// 再生
	void Play();
	void Stop();

private:
	Asset::AudioClip* pAudioClip_ = nullptr;
	std::string path_;
	float volume_;
	float pitch_;

	int state_;
	bool isPlayingRequest_;
	bool isStopRequest_ = false;
	bool isLoop_ = true;

	/// 再生中の音声ソース
	IXAudio2SourceVoice* sourceVoice_ = nullptr;

public:
	/// ----- setter ----- ///
	void SetVolume(float volume);
	void SetPitch(float pitch);
	void SetAudioPath(const std::string& path);
	void SetAudioClip(Asset::AudioClip* clip);
	void SetLoop(bool loop) { isLoop_ = loop; }

	/// ----- getter ----- ///
	float GetVolume() const;
	float GetPitch() const;
	bool GetLoop() const { return isLoop_; }
	const std::string& GetAudioPath() const;
	Asset::AudioClip* GetAudioClip() const;
	int GetState() const;

};

namespace ComponentDebug {
void BGMPlayerDebug(BGMPlayer* bgm);
}

namespace MonoInternalMethods {
void BGMPlayer_GetParams(uint64_t nativeHandle, float* volume, float* pitch, bool* loop);
void BGMPlayer_SetParams(uint64_t nativeHandle, float volume, float pitch, bool loop);
void BGMPlayer_Play(uint64_t nativeHandle);
void BGMPlayer_Stop(uint64_t nativeHandle);
}

void from_json(const nlohmann::json& j, BGMPlayer& a);
void to_json(nlohmann::json& j, const BGMPlayer& a);

} /// ONEngine
