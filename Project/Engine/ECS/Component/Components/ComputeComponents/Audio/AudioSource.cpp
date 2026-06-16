#include "AudioSource.h"

/// external
#include <imgui.h>
#include <magic_enum/magic_enum.hpp>

/// engine
#include "Engine/Asset/Assets/AudioClip/AudioClip.h"
#include "Engine/Core/Utility/Utility.h"

/// editor
#include "Engine/Editor/Math/ImGuiMath.h"
#include "Engine/Editor/Math/AssetPayload.h"

using namespace ONEngine;

AudioSource::AudioSource()
	: volume_(1.0f),
	pitch_(1.0f),
	state_(0),
	isPlayingRequest_(false) {
}

AudioSource::~AudioSource() {}

void AudioSource::Play() {
	isPlayingRequest_ = true;
}

void AudioSource::Stop() {
	isStopRequest_ = true;
}

void AudioSource::PlayOneShot(float volume, float pitch, const std::string& path) {
	oneShotAudioRequests_.push_back({ path, volume, pitch });
}

void AudioSource::AddSourceVoice(IXAudio2SourceVoice* sourceVoice) {
	sourceVoices_.push_back(sourceVoice);
}

void AudioSource::SetVolume(float volume) {
	volume_ = volume;
}

void AudioSource::SetPitch(float pitch) {
	pitch_ = pitch;
}

void AudioSource::SetAudioPath(const std::string& path) {
	path_ = path;
}

void AudioSource::SetAudioClip(Asset::AudioClip* clip) {
	pAudioClip_ = clip;
}

float AudioSource::GetVolume() const {
	return volume_;
}

float AudioSource::GetPitch() const {
	return pitch_;
}

const std::string& AudioSource::GetAudioPath() const {
	return path_;
}

Asset::AudioClip* AudioSource::GetAudioClip() const {
	return pAudioClip_;
}

int AudioSource::GetState() const {
	return state_;
}


/// 

void ComponentDebug::AudioSourceDebug(AudioSource* as) {
	if (!as) {
		return;
	}

	std::string audioPath = as->GetAudioPath();

	/// audio clipの編集
	ImGui::Text("Audio Source");
	Editor::ImMathf::InputText("Audio Path", &audioPath, ImGuiInputTextFlags_ReadOnly);
	if (ImGui::BeginDragDropTarget()) {
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("AssetData")) {
			if (payload->Data) {
				Editor::AssetPayload* assetPayload = *static_cast<Editor::AssetPayload**>(payload->Data);
				std::string path = assetPayload->filePath;
				std::string extension = FileSystem::FileExtension(path);

				/// Audioのパスが有効な形式か確認
				if (extension == ".mp3" ||
					extension == ".wav" ||
					extension == ".ogg") {
					as->SetAudioPath(path);

					Console::Log(std::format("Audio path set to: {}", path));
				} else {
					Console::LogError("Invalid audio format. Please use .mp3, .wav, or .ogg.");
				}
			}
		}
		ImGui::EndDragDropTarget();
	}

	ImGui::Spacing();

	/// 音量の編集
	float volume = as->GetVolume();
	ImGui::Text("Volume");
	if (ImGui::SliderFloat("##Volume", &volume, 0.0f, 1.0f, "%.2f")) {
		as->SetVolume(volume);
	}

	/// ピッチの編集
	float pitch = as->GetPitch();
	ImGui::Text("Pitch");
	if (ImGui::SliderFloat("##Pitch", &pitch, 0.0f, 3.0f, "%.2f")) {
		as->SetPitch(pitch);
	}

	ImGui::Spacing();

	/// 再生ボタン
	if (ImGui::Button("Play")) {
		as->Play();
	}

	ImGui::Spacing();

	/// 再生状態の表示
	int state = as->GetState();
	std::string stateStr = static_cast<std::string>(magic_enum::enum_name(static_cast<AudioState>(state)));
	ImGui::Text("State: %s", stateStr.c_str());

}

void MonoInternalMethods::InternalGetParams(uint64_t nativeHandle, float* volume, float* pitch) {
	AudioSource* audioSource = reinterpret_cast<AudioSource*>(nativeHandle);
	if (!audioSource) {
		Console::LogError("AudioSource pointer is null");
		return;
	}

	*volume = audioSource->GetVolume();
	*pitch = audioSource->GetPitch();

}

void ONEngine::MonoInternalMethods::InternalSetParams(uint64_t nativeHandle, float volume, float pitch) {
	AudioSource* audioSource = reinterpret_cast<AudioSource*>(nativeHandle);
	if (!audioSource) {
		Console::LogError("AudioSource pointer is null in InternalSetParams");
		return;
	}

	Console::Log(std::format("[CPP Audio] SetParams - Vol: {}, Pitch: {}", volume, pitch));
	audioSource->SetVolume(volume);
	audioSource->SetPitch(pitch);
}

void ONEngine::MonoInternalMethods::InternalPlay(uint64_t nativeHandle) {
	AudioSource* audioSource = reinterpret_cast<AudioSource*>(nativeHandle);
	if (audioSource) {
		Console::Log("[CPP Audio] Play Requested");
		audioSource->Play();
	}
}

void ONEngine::MonoInternalMethods::InternalStop(uint64_t nativeHandle) {
	AudioSource* audioSource = reinterpret_cast<AudioSource*>(nativeHandle);
	if (audioSource) {
		Console::Log("[CPP Audio] Stop Requested");
		audioSource->Stop();
	}
}

void ONEngine::MonoInternalMethods::InternalPlayOneShot(uint64_t nativeHandle, float volume, float pitch, MonoString* path) {
	/// 音の再生
	AudioSource* audioSource = reinterpret_cast<AudioSource*>(nativeHandle);
	if (!audioSource) {
		Console::LogError("AudioSource pointer is null in InternalPlayOneShot");
		return;
	}

	/// pathの変換
	char* pathCStr = mono_string_to_utf8(path);

	Console::Log(std::format("[CPP Audio] OneShot Requested - Path: {}, Vol: {}, Pitch: {}", pathCStr, volume, pitch));
	audioSource->PlayOneShot(volume, pitch, std::string(pathCStr));

	mono_free(pathCStr);
}


/// json serialize
void ONEngine::from_json(const nlohmann::json& j, AudioSource& a) {
	a.enable = j.value("enable", 1);
	a.SetVolume(j.value("volume", 1.0f));
	a.SetPitch(j.value("pitch", 1.0f));
	a.SetAudioPath(j.value("path", std::string("")));
}

void ONEngine::to_json(nlohmann::json& j, const AudioSource& a) {
	j = nlohmann::json{
		{ "type", "AudioSource" },
		{ "enable", a.enable },
		{ "volume", a.GetVolume() },
		{ "pitch", a.GetPitch() },
		{ "path", a.GetAudioPath() }
	};
}