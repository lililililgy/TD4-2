#include "AudioClip.h"

/// engine
#include "Engine/Core/Utility/Utility.h"


namespace ONEngine::Asset {

AudioClip::~AudioClip() {
	/// 音データの解放
	SoundDataUnload(&soundData_);
}

IXAudio2SourceVoice* AudioClip::CreateSourceVoice(IXAudio2* audio) {
	/// 音声ソースを作成し返す
	IXAudio2SourceVoice* sourceVoice = nullptr;
	HRESULT result = audio->CreateSourceVoice(&sourceVoice, &soundData_.wfex);
	Assert(SUCCEEDED(result));
	return sourceVoice;
}

const AudioStructs::SoundData& AudioClip::GetSoundData() {
	return soundData_;
}

void SoundDataUnload(AudioStructs::SoundData* soundData) {
	/// 音データの解放をする
	soundData->buffer.clear();
	soundData->wfex = {};
}

} /// namespace ONEngine::Asset