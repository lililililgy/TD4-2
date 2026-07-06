#include "SEPlayer.h"
#include "BGMPlayer.h" // AudioState の定義を取得するため

/// external
#include <imgui.h>
#include <magic_enum/magic_enum.hpp>

/// engine
#include "Engine/Asset/Assets/AudioClip/AudioClip.h"
#include "Engine/Core/Utility/Utility.h"

/// editor
#include "Engine/Editor/Math/ImGuiMath.h"
#include "Engine/Editor/Math/AssetPayload.h"
#include "Engine/Editor/Commands/LambdaCommand.h"
#include "Engine/Editor/Manager/EditCommand.h"

using namespace ONEngine;

SEPlayer::SEPlayer()
	: volume_(1.0f),
	pitch_(1.0f),
	state_(0),
	isPlayingRequest_(false) {
}

SEPlayer::~SEPlayer() {}

void SEPlayer::Play() {
	isPlayingRequest_ = true;
}

void SEPlayer::Stop() {
	isStopRequest_ = true;
}

void SEPlayer::PlayOneShot(float volume, float pitch, const std::string& path) {
	oneShotRequests_.push_back({ path, volume, pitch });
}

void SEPlayer::SetVolume(float volume) {
	volume_ = volume;
}

void SEPlayer::SetPitch(float pitch) {
	pitch_ = pitch;
}

void SEPlayer::SetAudioPath(const std::string& path) {
	path_ = path;
}

void SEPlayer::SetAudioClip(Asset::AudioClip* clip) {
	pAudioClip_ = clip;
}

float SEPlayer::GetVolume() const {
	return volume_;
}

float SEPlayer::GetPitch() const {
	return pitch_;
}

const std::string& SEPlayer::GetAudioPath() const {
	return path_;
}

Asset::AudioClip* SEPlayer::GetAudioClip() const {
	return pAudioClip_;
}

int SEPlayer::GetState() const {
	return state_;
}


/// 

void ComponentDebug::SEPlayerDebug(SEPlayer* se) {
	if (!se) {
		return;
	}

	std::string audioPath = se->GetAudioPath();

	/// audio clipの編集
	ImGui::Text("SE Player");
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
					std::string oldPath = se->GetAudioPath();
					std::string newPath = path;
					Editor::EditCommand::Execute<Editor::LambdaCommand>(
						[se, newPath]() { se->SetAudioPath(newPath); },
						[se, oldPath]() { se->SetAudioPath(oldPath); }
					);

					Console::Log(std::format("SE audio path set to: {}", path));
				} else {
					Console::LogError("Invalid audio format. Please use .mp3, .wav, or .ogg.");
				}
			}
		}
		ImGui::EndDragDropTarget();
	}

	ImGui::Spacing();

	/// 音量の編集
	float volume = se->GetVolume();
	ImGui::Text("Volume");
	if (ImGui::SliderFloat("##Volume", &volume, 0.0f, 1.0f, "%.2f")) {
		se->SetVolume(volume);
	}

	/// ピッチの編集
	float pitch = se->GetPitch();
	ImGui::Text("Pitch");
	if (ImGui::SliderFloat("##Pitch", &pitch, 0.0f, 3.0f, "%.2f")) {
		se->SetPitch(pitch);
	}

	ImGui::Spacing();

	/// 再生ボタン
	if (ImGui::Button("Play")) {
		se->Play();
	}
	ImGui::SameLine();
	if (ImGui::Button("Stop")) {
		se->Stop();
	}

	ImGui::Spacing();

	/// 再生状態の表示
	int state = se->GetState();
	std::string stateStr = static_cast<std::string>(magic_enum::enum_name(static_cast<AudioState>(state)));
	ImGui::Text("State: %s", stateStr.c_str());

}

void MonoInternalMethods::SEPlayer_GetParams(uint64_t nativeHandle, float* volume, float* pitch) {
	SEPlayer* se = reinterpret_cast<SEPlayer*>(nativeHandle);
	if (!se) {
		Console::LogError("SEPlayer pointer is null");
		return;
	}

	*volume = se->GetVolume();
	*pitch = se->GetPitch();
}

void MonoInternalMethods::SEPlayer_SetParams(uint64_t nativeHandle, float volume, float pitch) {
	SEPlayer* se = reinterpret_cast<SEPlayer*>(nativeHandle);
	if (!se) {
		Console::LogError("SEPlayer pointer is null in SEPlayer_SetParams");
		return;
	}

	se->SetVolume(volume);
	se->SetPitch(pitch);
}

void MonoInternalMethods::SEPlayer_Play(uint64_t nativeHandle) {
	SEPlayer* se = reinterpret_cast<SEPlayer*>(nativeHandle);
	if (se) {
		se->Play();
	}
}

void MonoInternalMethods::SEPlayer_Stop(uint64_t nativeHandle) {
	SEPlayer* se = reinterpret_cast<SEPlayer*>(nativeHandle);
	if (se) {
		se->Stop();
	}
}

void MonoInternalMethods::SEPlayer_PlayOneShot(uint64_t nativeHandle, float volume, float pitch, MonoString* path) {
	SEPlayer* se = reinterpret_cast<SEPlayer*>(nativeHandle);
	if (!se) {
		Console::LogError("SEPlayer pointer is null in SEPlayer_PlayOneShot");
		return;
	}

	char* pathCStr = mono_string_to_utf8(path);
	se->PlayOneShot(volume, pitch, std::string(pathCStr));
	mono_free(pathCStr);
}


/// json serialize
void ONEngine::from_json(const nlohmann::json& j, SEPlayer& a) {
	a.enable = j.value("enable", 1);
	a.SetVolume(j.value("volume", 1.0f));
	a.SetPitch(j.value("pitch", 1.0f));
	a.SetAudioPath(j.value("path", std::string("")));
}

void ONEngine::to_json(nlohmann::json& j, const SEPlayer& a) {
	j = nlohmann::json{
		{ "type", "SEPlayer" },
		{ "enable", a.enable },
		{ "volume", a.GetVolume() },
		{ "pitch", a.GetPitch() },
		{ "path", a.GetAudioPath() }
	};
}
