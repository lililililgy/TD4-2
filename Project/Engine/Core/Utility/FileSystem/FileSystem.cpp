#include "FileSystem.h"

using namespace ONEngine;

/// std
#include <filesystem>
#include <fstream>

/// externals
#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>

/// engine
#include "Engine/Core/Utility/Tools/Log.h"

namespace fs = std::filesystem;


std::vector<File> FileSystem::GetFiles(const std::string& fileDirectory, const std::string& fileExtension) {
	/// ----- 指定されたディレクトリ内のファイルを全て探索 ----- ///

	std::vector<File> result{};
	// ディレクトリが存在するか確認
	if (!fs::exists(fileDirectory) || !fs::is_directory(fileDirectory)) {
		return result; // 空のベクターを返す
	}


	/// 拡張子がある場合とない場合で処理を分ける
	if (fileExtension.empty()) {

		/// ディレクトリ内のファイルを全て探索
		for (const auto& entry : fs::recursive_directory_iterator(fileDirectory)) {
			if (fs::is_regular_file(entry)) {
				result.emplace_back(entry.path().string(), entry.path().filename().string());
			}
		}

	} else {

		/// 指定された拡張子を持つファイルのみを探索
		for (const auto& entry : fs::recursive_directory_iterator(fileDirectory)) {
			if (fs::is_regular_file(entry) && entry.path().extension() == fileExtension) {
				result.emplace_back(entry.path().string(), entry.path().filename().string());
			}
		}
	}

	for (auto& file : result) {
		ReplaceAll(&file.first, "\\", "/");
		ReplaceAll(&file.second, "\\", "/");
	}

	return result;
}

File FileSystem::GetFile(const std::string& fileDirectory, const std::string& filename) {
	// ディレクトリが存在するか確認
	if (!fs::exists(fileDirectory) || !fs::is_directory(fileDirectory)) {
		Console::LogError("Directory does not exist: " + fileDirectory);
		return File(); // 空のFileを返す
	}

	/// ディレクトリ内のファイルを探索
	for (const auto& entry : fs::recursive_directory_iterator(fileDirectory)) {
		if (fs::is_regular_file(entry) && entry.path().filename() == filename) {
			std::string filePath = entry.path().string();
			ReplaceAll(&filePath, "\\", "/"); // パスの区切り文字を統一
			return File(filePath, entry.path().filename().string());
		}
	}

	return File();
}

bool FileSystem::FileExists(const std::string& fileDirectory, const std::string& filename) {
	/// ディレクトリが存在するか確認
	if (!fs::exists(fileDirectory) || !fs::is_directory(fileDirectory)) {
		return false;
	}

	/// ディレクトリ内のファイルを探索、ファイル名が一致したらtrueを返す
	for (const auto& entry : fs::recursive_directory_iterator(fileDirectory)) {
		if (fs::is_regular_file(entry) && entry.path().filename() == filename) {
			return true;
		}
	}

	/// 見つからなかった場合
	return false;
}

bool FileSystem::FileExists(const std::string& path) {
	return std::filesystem::exists(path);
}

void FileSystem::ReplaceAll(std::string* str, const std::string& from, const std::string& to) {
	if (!str) {
		return; // nullptrチェック
	}

	/// 対象が空なら何もしない
	if (from.empty()) {
		return;
	}

	size_t pos = 0;
	while ((pos = str->find(from, pos)) != std::string::npos) {
		str->replace(pos, from.length(), to);
		pos += to.length(); // 次の検索位置を更新
	}
}

std::string ONEngine::FileSystem::ReplaceAll(const std::string& str, const std::string& from, const std::string& to) {
	std::string result = str;
	ReplaceAll(&result, from, to);
	return result;
}

std::string FileSystem::FileNameWithoutExtension(const std::string& filename) {
	size_t lastDot = filename.find_last_of('.');
	if (lastDot == std::string::npos) {
		return filename;  // 拡張子がなければそのまま返す
	}
	return filename.substr(0, lastDot);
}

std::string FileSystem::FileExtension(const std::string& filename) {
	size_t lastDot = filename.find_last_of('.');
	if (lastDot == std::string::npos) {
		return "";  // 拡張子がなければ空文字を返す
	}
	return filename.substr(lastDot); // 拡張子を返す
}

std::vector<std::vector<int>> FileSystem::LoadCSV(const std::string& filePath) {
	/// ----- CSVファイルを読み込む ----- ///

	std::vector<std::vector<int>> data;

	/// ファイルを開く
	std::ifstream file(filePath);
	if (!file.is_open()) {
		Console::LogError("Mathf::LoadCSV: Could not open file " + filePath);
		return data; // 空のベクターを返す
	}

	/// 行ごとに読み込む
	std::string line;
	while (std::getline(file, line)) {
		std::vector<int> row;
		std::stringstream ss(line);
		std::string cell;

		while (std::getline(ss, cell, ',')) {
			try {
				int value = std::stoi(cell);
				row.push_back(value);
			} catch (const std::invalid_argument&) {
				Console::LogError("Mathf::LoadCSV: Invalid integer in file " + filePath + ": " + cell);
			}
		}

		data.push_back(row);
	}

	file.close();
	return data;
}

bool FileSystem::StartsWith(const std::string& str, const std::string& prefix) {
	return str.rfind(prefix, 0) == 0;
}


std::string FileSystem::LoadFile(const std::string& directory, const std::string& filename) {
	/// ----- ファイルを読み込む ----- ///

	if (!FileExists(directory, filename)) {
		return "";
	}

	/// パスをフルパスに変換
	std::filesystem::path dir(directory);
	std::filesystem::path filePath(filename);
	std::filesystem::path fullPath = dir / filePath;

	return LoadFile(fullPath.string());
}

std::string FileSystem::LoadFile(const std::string& path) {
	/// ----- ファイルを読み込む ----- ///

	// ファイルストリームで読み込み
	std::ifstream file(path);
	if (!file.is_open()) {
		return ""; // 開けなかった場合も空文字列
	}

	/// ファイルの中身をテキストに
	std::stringstream buffer;
	buffer << file.rdbuf();
	file.close();

	return buffer.str();
}



MonoString* MonoInternalMethods::LoadFile(MonoString* path) {

	/// スクリプト名をUTF-8に変換
	char* cstr = mono_string_to_utf8(path);
	std::string pathStr(cstr);
	mono_free(cstr);

	std::string fileText = FileSystem::LoadFile(pathStr);
	MonoString* monoStr = mono_string_new(mono_domain_get(), fileText.c_str());

	return monoStr;
}
