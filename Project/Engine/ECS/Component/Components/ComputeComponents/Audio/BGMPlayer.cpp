#include "BGMPlayer.h"

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

BGMPlayer::BGMPlayer()
	: volume_(1.0f),
	pitch_(1.0f),
	state_(0),
	isPlayingRequest_(false) {
}

BGMPlayer::~BGMPlayer() {}

void BGMPlayer::Play() {
	isPlayingRequest_ = true;
}

void BGMPlayer::Stop() {
	isStopRequest_ = true;
}

void BGMPlayer::SetVolume(float volume) {
	volume_ = volume;
}

void BGMPlayer::SetPitch(float pitch) {
	pitch_ = pitch;
}

void BGMPlayer::SetAudioPath(const std::string& path) {
	path_ = path;
}

void BGMPlayer::SetAudioClip(Asset::AudioClip* clip) {
	pAudioClip_ = clip;
}

float BGMPlayer::GetVolume() const {
	return volume_;
}

float BGMPlayer::GetPitch() const {
	return pitch_;
}

const std::string& BGMPlayer::GetAudioPath() const {
	return path_;
}

Asset::AudioClip* BGMPlayer::GetAudioClip() const {
	return pAudioClip_;
}

int BGMPlayer::GetState() const {
	return state_;
}

/// 

void ComponentDebug::BGMPlayerDebug(BGMPlayer* bgm) {
	if (!bgm) {
		return;
	}

	std::string audioPath = bgm->GetAudioPath();

	/// audio clipの編集
	ImGui::Text("BGM Player");
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
					std::string oldPath = bgm->GetAudioPath();
					std::string newPath = path;
					Editor::EditCommand::Execute<Editor::LambdaCommand>(
						[bgm, newPath]() { bgm->SetAudioPath(newPath); },
						[bgm, oldPath]() { bgm->SetAudioPath(oldPath); }
					);

					Console::Log(std::format("BGM audio path set to: {}", path));
				} else {
					Console::LogError("Invalid audio format. Please use .mp3, .wav, or .ogg.");
				}
			}
		}
		ImGui::EndDragDropTarget();
	}

	ImGui::Spacing();

	/// 音量の編集
	float volume = bgm->GetVolume();
	ImGui::Text("Volume");
	if (ImGui::SliderFloat("##Volume", &volume, 0.0f, 1.0f, "%.2f")) {
		bgm->SetVolume(volume);
	}

	/// ピッチの編集
	float pitch = bgm->GetPitch();
	ImGui::Text("Pitch");
	if (ImGui::SliderFloat("##Pitch", &pitch, 0.0f, 3.0f, "%.2f")) {
		bgm->SetPitch(pitch);
	}

	/// ループ
	bool loop = bgm->GetLoop();
	if (ImGui::Checkbox("Loop", &loop)) {
		bgm->SetLoop(loop);
	}

	ImGui::Spacing();

	/// 再生ボタン
	if (ImGui::Button("Play")) {
		bgm->Play();
	}
	ImGui::SameLine();
	if (ImGui::Button("Stop")) {
		bgm->Stop();
	}

	ImGui::Spacing();

	/// 再生状態の表示
	int state = bgm->GetState();
	std::string stateStr = static_cast<std::string>(magic_enum::enum_name(static_cast<AudioState>(state)));
	ImGui::Text("State: %s", stateStr.c_str());

}

void MonoInternalMethods::BGMPlayer_GetParams(uint64_t nativeHandle, float* volume, float* pitch, bool* loop) {
	BGMPlayer* bgm = reinterpret_cast<BGMPlayer*>(nativeHandle);
	if (!bgm) {
		Console::LogError("BGMPlayer pointer is null");
		return;
	}

	*volume = bgm->GetVolume();
	*pitch = bgm->GetPitch();
	*loop = bgm->GetLoop();
}

void MonoInternalMethods::BGMPlayer_SetParams(uint64_t nativeHandle, float volume, float pitch, bool loop) {
	BGMPlayer* bgm = reinterpret_cast<BGMPlayer*>(nativeHandle);
	if (!bgm) {
		Console::LogError("BGMPlayer pointer is null in BGMPlayer_SetParams");
		return;
	}

	bgm->SetVolume(volume);
	bgm->SetPitch(pitch);
	bgm->SetLoop(loop);
}

void MonoInternalMethods::BGMPlayer_Play(uint64_t nativeHandle) {
	BGMPlayer* bgm = reinterpret_cast<BGMPlayer*>(nativeHandle);
	if (bgm) {
		bgm->Play();
	}
}

void MonoInternalMethods::BGMPlayer_Stop(uint64_t nativeHandle) {
	BGMPlayer* bgm = reinterpret_cast<BGMPlayer*>(nativeHandle);
	if (bgm) {
		bgm->Stop();
	}
}

/// json serialize
void ONEngine::from_json(const nlohmann::json& j, BGMPlayer& a) {
	a.enable = j.value("enable", 1);
	a.SetVolume(j.value("volume", 1.0f));
	a.SetPitch(j.value("pitch", 1.0f));
	a.SetAudioPath(j.value("path", std::string("")));
	a.SetLoop(j.value("loop", true));
}

void ONEngine::to_json(nlohmann::json& j, const BGMPlayer& a) {
	j = nlohmann::json{
		{ "type", "BGMPlayer" },
		{ "enable", a.enable },
		{ "volume", a.GetVolume() },
		{ "pitch", a.GetPitch() },
		{ "path", a.GetAudioPath() },
		{ "loop", a.GetLoop() }
	};
}
