#include "ProjectWindow.h"

/// std
#include <filesystem>
#include <iostream>
#include <format>
#include <unordered_set>
#include <algorithm>

/// external
#include <imgui.h>
#include <shellapi.h>
#include <fstream>
#include <nlohmann/json.hpp>

/// engine
#include "Engine/Asset/Collection/AssetCollection.h"
#include "Engine/ECS/EntityComponentSystem/EntityComponentSystem.h"
#include "Engine/ECS/Entity/GameEntity/GameEntity.h"

/// editor
#include "Engine/Editor/Manager/EditorManager.h"
#include "Engine/Editor/Manager/HotReloadManager.h"
#include "Engine/Editor/Manager/ImGuiManager.h"
#include "Engine/Editor/Math/AssetPayload.h"
#include "Engine/Editor/Math/ImGuiMath.h"
#include "Engine/Editor/Math/ImGuiSelection.h"
#include "Engine/Asset/AssetType.h"

using namespace Editor;

namespace {

bool IsValidCSharpIdentifier(const std::string& name, std::string& outErrorReason) {
	if (name.empty()) {
		outErrorReason = "クラス名が空です。";
		return false;
	}

	static const std::unordered_set<std::string> keywords = {
		"abstract", "as", "base", "bool", "break", "byte", "case", "catch", "char", "checked",
		"class", "const", "continue", "decimal", "default", "delegate", "do", "double", "else",
		"enum", "event", "explicit", "extern", "false", "finally", "fixed", "float", "for",
		"foreach", "goto", "if", "implicit", "in", "int", "interface", "internal", "is", "lock",
		"long", "namespace", "new", "null", "object", "operator", "out", "override", "params",
		"private", "protected", "public", "readonly", "ref", "return", "sbyte", "sealed",
		"short", "sizeof", "stackalloc", "static", "string", "struct", "switch", "this", "throw",
		"true", "try", "typeof", "uint", "ulong", "unchecked", "unsafe", "ushort", "using",
		"virtual", "void", "volatile", "while"
	};

	if (keywords.find(name) != keywords.end()) {
		outErrorReason = "C#の予約キーワードはクラス名として使用できません。";
		return false;
	}

	for (size_t i = 0; i < name.length(); ) {
		unsigned char c = static_cast<unsigned char>(name[i]);

		if (c < 128) {
			if (i == 0) {
				if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_')) {
					outErrorReason = "クラス名の先頭文字は英字またはアンダースコアである必要があります。";
					return false;
				}
			} else {
				if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_')) {
					outErrorReason = "クラス名に空白や記号（英数字・アンダースコア以外）は使用できません。";
					return false;
				}
			}
			i += 1;
		} else {
			int len = 0;
			if ((c & 0xE0) == 0xC0) len = 2;
			else if ((c & 0xF0) == 0xE0) len = 3;
			else if ((c & 0xF8) == 0xF0) len = 4;
			else {
				outErrorReason = "不正な文字コードが含まれています。";
				return false;
			}

			if (i + len > name.length()) {
				outErrorReason = "文字コードが破損しています。";
				return false;
			}

			// 全角スペース (UTF-8: E3 80 80)
			if (len == 3 && 
				static_cast<unsigned char>(name[i]) == 0xE3 && 
				static_cast<unsigned char>(name[i+1]) == 0x80 && 
				static_cast<unsigned char>(name[i+2]) == 0x80) {
				outErrorReason = "クラス名に全角スペースは使用できません。";
				return false;
			}

			// CJK記号・句読点 (E3 80 81 - E3 80 BF)
			if (len == 3 && 
				static_cast<unsigned char>(name[i]) == 0xE3 && 
				static_cast<unsigned char>(name[i+1]) == 0x80) {
				unsigned char third = static_cast<unsigned char>(name[i+2]);
				if (third >= 0x81 && third <= 0xBF) {
					outErrorReason = "クラス名に記号は使用できません。";
					return false;
				}
			}

			// 全角記号 (EF BC 81 - EF BC 9F)
			if (len == 3 && 
				static_cast<unsigned char>(name[i]) == 0xEF && 
				static_cast<unsigned char>(name[i+1]) == 0xBC) {
				unsigned char third = static_cast<unsigned char>(name[i+2]);
				if (third >= 0x81 && third <= 0x9F) {
					outErrorReason = "クラス名に記号は使用できません。";
					return false;
				}
			}

			i += len;
		}
	}

	return true;
}

/// @brief .slnファイルからの絶対パス
const std::filesystem::path kRootPath = std::filesystem::absolute("./");


/// @brief 指定した基準パスに対する、与えられた絶対パスの相対パスを計算して文字列で返す。
std::string GetRelativePath(const std::filesystem::path& absolutePath, const std::filesystem::path& basePath = kRootPath) {
	std::filesystem::path relativePath = std::filesystem::relative(absolutePath, basePath);
	std::string relativeStr = relativePath.string();

	if(!relativeStr.empty() && relativeStr[0] != '.') {
		relativeStr = "./" + relativeStr;
	} else if(relativeStr == ".") {
		relativeStr = "./";
	}

	std::replace(relativeStr.begin(), relativeStr.end(), '\\', '/');

	return relativeStr;
}

}

ProjectWindow::ProjectWindow(ONEngine::Asset::AssetCollection* assetCollection)
	: pAssetCollection_(assetCollection) {
	windowName_ = "Project";

	rootPaths_ = { "./Assets", "./Packages", "../SubProjects/CSharpLibrary/Scripts" };
	currentPath_ = rootPaths_[0];

	for(const auto& path : rootPaths_) {
		if(std::filesystem::exists(path)) {
			UpdateDirectoryCache(path);
		}
	}
	UpdateFileCache(currentPath_);
}

ProjectWindow::~ProjectWindow() {}


///	-------------------------------------------------------------------------------------------------------------------------------------------------------------------
/// プロジェクトウィンドウの基礎的な処理
///	-------------------------------------------------------------------------------------------------------------------------------------------------------------------

///
/// プロジェクトウィンドウの表示
///
void ProjectWindow::ShowImGui() {
	// ファイル監視イベントの処理
	for(const auto& ev : HotReloadManager::GetInstance().ConsumeLatestEvents()) {
		// キャッシュの更新
		std::filesystem::path parentPath = std::filesystem::path(ev.path).parent_path();
		UpdateDirectoryCache(parentPath);

		if(currentPath_ == parentPath) {
			UpdateFileCache(currentPath_);
		}
	}

	bool* p_open = canClose_ ? &isOpen_ : nullptr;
	if(ImGui::Begin(windowName_.c_str(), p_open)) {
		ImGui::Columns(2);

		// 左側：フォルダツリー
		if(ImGui::BeginChild("DirectoryTree")) {
			for(const auto& root : rootPaths_) {
				if(std::filesystem::exists(root)) {
					DrawDirectoryTree(root);
				}
			}
			ImGui::Dummy(ImVec2(0.0f, 100.0f)); // 余分にスクロールできるようにするための余白
		}
		ImGui::EndChild();

		ImGui::NextColumn();

		// 空白部分で右クリックしたときに新規作成メニューを表示するための判定
		if(ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows) && ImGui::IsMouseClicked(ImGuiMouseButton_Right) && !ImGui::IsAnyItemHovered()) {
			ImGui::OpenPopup("ProjectWindow_GlobalContextMenu");
		}

		if(ImGui::BeginPopup("ProjectWindow_GlobalContextMenu")) {
			if(ImGui::BeginMenu("Create")) {
				if(ImGui::MenuItem("Folder")) {
					showCreateFolderPopup_ = true;
					targetPath_ = currentPath_;
					inputBuffer_ = "NewFolder";
				}

				// C#スクリプトの作成場所を制限 (../SubProjects/CSharpLibrary/Scripts 以下のみ)
				std::filesystem::path scriptsRoot = "../SubProjects/CSharpLibrary/Scripts";
				std::string scriptsRootStr = std::filesystem::absolute(scriptsRoot).string();
				std::string currentPathStr = std::filesystem::absolute(currentPath_).string();
				bool isValidScriptPath = currentPathStr.find(scriptsRootStr) != std::string::npos;

				if(ImGui::MenuItem("C# Script", nullptr, false, isValidScriptPath)) {
					showCreateScriptPopup_ = true;
					targetPath_ = currentPath_;
					inputBuffer_ = "NewScript";
					errorMessage_.clear();
				}
				if(!isValidScriptPath && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
					ImGui::SetTooltip("C# scripts can only be created in the CSharpLibrary/Scripts folder.");
				}

				ImGui::EndMenu();
			}
			ImGui::Separator();
			if(ImGui::MenuItem("Show in Explorer")) {
				std::filesystem::path absolutePath = std::filesystem::absolute(currentPath_);
				ShellExecuteW(NULL, L"open", absolutePath.wstring().c_str(), NULL, NULL, SW_SHOWNORMAL);
			}
			ImGui::EndPopup();
		}

		// 右側：ファイルビュー
		DrawFileView(currentPath_);

		ImGui::Columns(1);
	}

	// --- モーダルポップアップの描画 ---

	// リネーム用
	if(showRenamePopup_) ImGui::OpenPopup("Rename Item");
	if(ImGui::BeginPopupModal("Rename Item", &showRenamePopup_, ImGuiWindowFlags_AlwaysAutoResize)) {
		ImGui::Text("Enter new name:");
		ImGuiInputText("##newName", &inputBuffer_);
		ImGui::Spacing();
		if(ImGui::Button("OK", ImVec2(120, 0))) {
			try {
				std::filesystem::path newPath = targetPath_.parent_path() / inputBuffer_;
				// 拡張子を保持（ファイルの場合）
				if(!std::filesystem::is_directory(targetPath_)) {
					if(!newPath.has_extension()) {
						newPath.replace_extension(targetPath_.extension());
					}
				}

				if(!std::filesystem::exists(newPath)) {
					std::filesystem::rename(targetPath_, newPath);
					// .metaファイルもあればリネーム
					std::filesystem::path oldMeta = targetPath_.string() + ".meta";
					std::filesystem::path newMeta = newPath.string() + ".meta";
					if(std::filesystem::exists(oldMeta)) {
						std::filesystem::rename(oldMeta, newMeta);
					}

					// Prefabのリネーム時に配置済みEntityの名前とPrefab名を同期
					if(targetPath_.extension() == ".prefab") {
						std::string oldPrefabName = targetPath_.stem().string();
						std::string newPrefabName = newPath.stem().string();

						// 1. プレハブファイル自体の内部データ（JSON）の "name" と "prefabName" を書き換える
						try {
							std::ifstream inFile(newPath);
							if (inFile.is_open()) {
								nlohmann::json prefabJson;
								inFile >> prefabJson;
								inFile.close();

								prefabJson["name"] = newPrefabName;
								prefabJson["prefabName"] = newPrefabName;

								std::ofstream outFile(newPath);
								if (outFile.is_open()) {
									outFile << prefabJson.dump(4);
									outFile.close();
								}
							}
						} catch (...) {
							ONEngine::Console::LogError("Failed to update internal name of prefab file.");
						}

						// 2. シーン内の配置済みEntityの同期およびプレハブキャッシュの再構築
						if(auto* ecs = pImGuiManager_->GetEntityComponentSystem()) {
							if(auto* currentGroup = ecs->GetCurrentGroup()) {
								for(auto& entity : currentGroup->GetEntities()) {
									if(entity && entity->GetPrefabName() == oldPrefabName) {
										entity->SetPrefabName(newPrefabName);
										entity->SetName(newPrefabName);
									}
								}
								if(auto* entityCollection = currentGroup->GetEntityCollection()) {
									entityCollection->LoadPrefabAll();
								}
							}
						}
					}

					UpdateFileCache(currentPath_);
					UpdateDirectoryCache(currentPath_.parent_path());
				}
			} catch(...) {}
			showRenamePopup_ = false;
		}
		ImGui::SameLine();
		if(ImGui::Button("Cancel", ImVec2(120, 0))) { showRenamePopup_ = false; }
		ImGui::EndPopup();
	}

	// フォルダ作成用
	if(showCreateFolderPopup_) ImGui::OpenPopup("Create Folder");
	if(ImGui::BeginPopupModal("Create Folder", &showCreateFolderPopup_, ImGuiWindowFlags_AlwaysAutoResize)) {
		ImGui::Text("Enter folder name:");
		ImGuiInputText("##folderName", &inputBuffer_);
		if(ImGui::Button("OK", ImVec2(120, 0))) {
			try {
				std::filesystem::path newDir = targetPath_ / inputBuffer_;
				std::filesystem::create_directories(newDir);
				UpdateFileCache(targetPath_);
				UpdateDirectoryCache(targetPath_);
			} catch(...) {}
			showCreateFolderPopup_ = false;
		}
		ImGui::SameLine();
		if(ImGui::Button("Cancel", ImVec2(120, 0))) { showCreateFolderPopup_ = false; }
		ImGui::EndPopup();
	}

	// C#スクリプト作成用
	if(showCreateScriptPopup_) ImGui::OpenPopup("Create C# Script");
	if(ImGui::BeginPopupModal("Create C# Script", &showCreateScriptPopup_, ImGuiWindowFlags_AlwaysAutoResize)) {
		ImGui::Text("Enter script name:");
		ImGuiInputText("##scriptName", &inputBuffer_);

		if(!errorMessage_.empty()) {
			ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "%s", errorMessage_.c_str());
		}

		if(ImGui::Button("OK", ImVec2(120, 0))) {
			std::string className = inputBuffer_;
			std::string errorReason;
			if(!IsValidCSharpIdentifier(className, errorReason)) {
				errorMessage_ = errorReason;
			} else {
				try {
					std::filesystem::path newScript = targetPath_ / (className + ".cs");
					if(!std::filesystem::exists(newScript)) {
						std::ofstream ofs(newScript);
						ofs << "using System;\n";
						ofs << "using System.Collections.Generic;\n\n";
						ofs << "public class " << className << " : MonoScript {\n";
						ofs << "\tpublic override void Initialize() {\n\t\t\n\t}\n\n";
						ofs << "\tpublic override void Update() {\n\t\t\n\t}\n";
						ofs << "}\n";
						ofs.close();
						UpdateFileCache(targetPath_);

						// スクリプト作成後にPremakeを実行してプロジェクトを更新 (Projectフォルダからの相対パス)
						//system("powershell.exe -ExecutionPolicy Bypass -File ../SubProjects/CSharpLibrary/GenerateProject_CS.ps1");

						// プロジェクトが更新されたのでホットリロードを要求（ビルドは手動または起動時に行われる想定）
						//HotReloadManager::GetInstance().RequestScriptHotReload();
						
						showCreateScriptPopup_ = false;
					} else {
						errorMessage_ = "同名のファイルが既に存在します。";
					}
				} catch(...) {
					errorMessage_ = "ファイルの作成に失敗しました。";
				}
			}
		}
		ImGui::SameLine();
		if(ImGui::Button("Cancel", ImVec2(120, 0))) { showCreateScriptPopup_ = false; }
		ImGui::EndPopup();
	}

	ImGui::End();
}


///	-------------------------------------------------------------------------------------------------------------------------------------------------------------------
/// 以下、処理ごとの関数実装
///	-------------------------------------------------------------------------------------------------------------------------------------------------------------------

///
/// プロジェクトウィンドウのウィンドウ名を設定
///
void ProjectWindow::SetWindowName(const std::string& windowName) {
	windowName_ = windowName;
}

///
/// プロジェクトのファイル構造をツリーで表示
///
void ProjectWindow::DrawDirectoryTree(const std::filesystem::path& directory) {
	std::string dirStr = directory.string();
	std::string dirName = directory.filename().string();

	// ルートパス自体（./Assets など）の名前が空（.）になる場合の対策
	if(dirName == "." || dirName == "") {
		dirName = directory.stem().string();
	}

	ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
	if(currentPath_ == directory) flags |= ImGuiTreeNodeFlags_Selected;

	// 子ディレクトリがない場合は Leaf にする
	bool hasSubDirs = directoryCache_.contains(dirStr) && !directoryCache_[dirStr].empty();
	if(!hasSubDirs) flags |= ImGuiTreeNodeFlags_Leaf;

	bool isOpen = ImGui::TreeNodeEx(dirName.c_str(), flags);

	// クリックで中身を表示
	if(ImGui::IsItemClicked()) {
		currentPath_ = directory;
		UpdateFileCache(currentPath_);
		searchBuffer_.clear(); // フォルダ選択で検索解除
		isSearching_ = false;
		selectedFileIndex_ = -1;
	}

	if(isOpen) {
		if(directoryCache_.contains(dirStr)) {
			for(const auto& item : directoryCache_[dirStr]) {
				DrawDirectoryTree(item.path);
			}
		}
		ImGui::TreePop();
	}
}

///
/// ファイルのビュー表示
///
void ProjectWindow::DrawFileView(const std::filesystem::path& directory) {
	// --- 検索バーとフィルタの描画 (固定部) ---
	float filterWidth = 100.0f;
	float buttonWidth = 24.0f;
	float spacing = 8.0f;
	float totalWidth = ImGui::GetContentRegionAvail().x;

	ImGui::PushItemWidth(totalWidth - filterWidth - buttonWidth - (spacing * 2.0f));
	if(ImGuiInputText("##ProjectSearch", &searchBuffer_, ImGuiInputTextFlags_AutoSelectAll, "search file...")) {
	}
	if(ImGui::IsItemDeactivatedAfterEdit() || !searchBuffer_.empty()) {
		isSearching_ = !searchBuffer_.empty();
	} else {
		isSearching_ = false;
	}
	ImGui::PopItemWidth();

	ImGui::SameLine(0, spacing);

	ImGui::PushItemWidth(filterWidth);
	if(ImGuiInputText("##ProjectFilter", &filterBuffer_, ImGuiInputTextFlags_AutoSelectAll, ".ext")) {
	}
	if(ImGui::IsItemDeactivatedAfterEdit() || !filterBuffer_.empty()) {
		isFiltering_ = !filterBuffer_.empty();
	} else {
		isFiltering_ = false;
	}
	ImGui::PopItemWidth();

	ImGui::SameLine(0, spacing);

	if(ImGui::Button("+", ImVec2(buttonWidth, 0.0f))) {
		if(GetParentContainer()) {
			static int projectWindowCounter = 1;
			int id = ++projectWindowCounter;
			std::string name = std::format("Project ({})##Project_{}", id, id);

			auto newWindow = std::make_unique<ProjectWindow>(pAssetCollection_);
			newWindow->SetWindowName(name);

			GetParentContainer()->AddView(std::move(newWindow));
		}
	}

	// 検索文字列が空でない場合にグローバル検索を実行
	if(isSearching_) {
		searchedFiles_.clear();
		std::string query = searchBuffer_;
		std::transform(query.begin(), query.end(), query.begin(), ::tolower);

		std::string extQuery = filterBuffer_;
		std::transform(extQuery.begin(), extQuery.end(), extQuery.begin(), ::tolower);
		if(!extQuery.empty() && extQuery[0] != '.') extQuery = "." + extQuery;

		for(const auto& root : rootPaths_) {
			if(!std::filesystem::exists(root)) continue;
			for(const auto& entry : std::filesystem::recursive_directory_iterator(root)) {
				if(entry.path().extension() == ".meta") continue;

				std::string filename = entry.path().filename().string();
				std::string filenameLower = filename;
				std::transform(filenameLower.begin(), filenameLower.end(), filenameLower.begin(), ::tolower);

				bool matchName = filenameLower.find(query) != std::string::npos;
				bool matchExt = true;
				if(isFiltering_) {
					std::string ext = entry.path().extension().string();
					std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
					matchExt = (ext == extQuery);
				}

				if(matchName && matchExt) {
					FileItem item;
					item.path = entry.path();
					item.isDirectory = entry.is_directory();
					item.relativePath = GetRelativePath(entry.path());

					// アイコン設定
					std::string ext = item.path.extension().string();
					std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
					if(item.isDirectory) {
						item.displayTexture = pAssetCollection_->GetTexture("./Packages/Textures/ImGui/FileIcons/FolderIcon.png");
						if(!item.displayTexture) item.displayTexture = pAssetCollection_->GetTexture("./Packages/Textures/ImGui/FileIcons/FolderIcon.dds");
					} else if(ext == ".cs") {
						item.displayTexture = pAssetCollection_->GetTexture("./Packages/Textures/ImGui/FileIcons/ph-file-c-sharp-none-256.png");
					} else {
						item.displayTexture = pAssetCollection_->GetTexture("./Packages/Textures/ImGui/FileIcons/FileIcon.png");
					}

					searchedFiles_.push_back(item);
				}
			}
		}
	}
	ImGui::Spacing();

	bool requestChangeDir = false;
	std::filesystem::path nextTargetDir;

	if(isSearching_) {
		ImGui::Text("Search Results for \"%s\"%s", searchBuffer_.c_str(), isFiltering_ ? (std::string(" (Type: ") + filterBuffer_ + ")").c_str() : "");
		ImGui::Separator();
	} else {
		// パンくずリスト（固定部）
		DrawBreadcrumbs(directory, requestChangeDir, nextTargetDir);
	}

	// ここから下をスクロール可能にする
	if(ImGui::BeginChild("FileListScrollArea", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar)) {
		DrawFileList(directory, requestChangeDir, nextTargetDir);
		ImGui::Dummy(ImVec2(0.0f, 100.0f)); // 余分にスクロールできるようにするための余白
	}
	ImGui::EndChild();

	// ディレクトリ移動リクエストの処理
	if(requestChangeDir) {
		currentPath_ = nextTargetDir;
		UpdateFileCache(currentPath_);
		searchBuffer_.clear();
		isSearching_ = false;
		selectedFileIndex_ = -1;
	}
}

///
/// パンくずリスト（階層ナビゲーション）の描画
///
void ProjectWindow::DrawBreadcrumbs(const std::filesystem::path& directory, bool& outRequestChangeDir, std::filesystem::path& outNextTargetDir) {
	// --- パンくずリスト（Breadcrumb）の描画 ---
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0)); // ボタン背景を透明化
	std::filesystem::path cumulativePath;
	bool isFirst = true;

	for(const auto& part : directory) {
		if(!isFirst) {
			ImGui::SameLine(0, 4.0f);
			ImGui::Text("/");
			ImGui::SameLine(0, 4.0f);
		}

		if(cumulativePath.empty()) {
			cumulativePath = part;
		} else {
			cumulativePath /= part;
		}

		// パスの一部をボタンとして描画（クリックでその階層へ移動）
		if(ImGui::Button(part.string().c_str())) {
			outRequestChangeDir = true;
			outNextTargetDir = cumulativePath;
		}
		isFirst = false;
	}
	ImGui::PopStyleColor();
	if(isFiltering_) {
		ImGui::SameLine();
		ImGui::TextDisabled(" (Filter: %s)", filterBuffer_.c_str());
	}
	ImGui::Separator();
	ImGui::Spacing();
	// ----------------------------------------
}

///
/// ファイル一覧の描画（ClipperとTableによる最適化版）
///
void ProjectWindow::DrawFileList(const std::filesystem::path& directory, bool& outRequestChangeDir, std::filesystem::path& outNextTargetDir) {
	std::string dirStr = directory.string();

	// 検索中なら searchedFiles_ を、そうでなければキャッシュを使用
	std::vector<FileItem> filteredFiles;
	std::vector<FileItem>* pFiles = nullptr;

	if(isSearching_) {
		pFiles = &searchedFiles_;
	} else {
		if(!fileCache_.contains(dirStr) || fileCache_[dirStr].empty()) return;

		if(isFiltering_) {
			std::string extQuery = filterBuffer_;
			std::transform(extQuery.begin(), extQuery.end(), extQuery.begin(), ::tolower);
			if(!extQuery.empty() && extQuery[0] != '.') extQuery = "." + extQuery;

			for(const auto& file : fileCache_[dirStr]) {
				if(file.isDirectory) {
					filteredFiles.push_back(file);
					continue;
				}
				std::string ext = file.path.extension().string();
				std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
				if(ext == extQuery) {
					filteredFiles.push_back(file);
				}
			}
			pFiles = &filteredFiles;
		} else {
			pFiles = &fileCache_[dirStr];
		}
	}

	auto& files = *pFiles;
	if(files.empty()) {
		if(isSearching_ || isFiltering_) ImGui::Text("No files match your criteria.");
		return;
	}

	// 削除予約用の変数
	std::filesystem::path pendingDeletePath;

	// --- ファイル一覧の描画 ---
	float iconSize = 64.0f;
	float padding = 16.0f;
	float cellSize = iconSize + padding;
	float panelWidth = ImGui::GetContentRegionAvail().x;

	// パネル幅から列数を計算
	int columnCount = static_cast<int>(panelWidth / cellSize);
	if(columnCount < 1) columnCount = 1;

	// 総行数を計算（切り上げ）
	int rowCount = (static_cast<int>(files.size()) + columnCount - 1) / columnCount;

	if(ImGui::BeginTable("FileGrid", columnCount)) {
		ImGuiListClipper clipper;
		clipper.Begin(rowCount);

		while(clipper.Step()) {
			for(int row = clipper.DisplayStart; row < clipper.DisplayEnd; ++row) {
				ImGui::TableNextRow();

				for(int col = 0; col < columnCount; ++col) {
					size_t index = static_cast<size_t>(row * columnCount + col);
					if(index >= files.size()) break; // 最後の半端な列は抜ける

					ImGui::TableSetColumnIndex(col);

					auto& file = files[index];
					std::string name = file.path.filename().string();

					ImGui::PushID(static_cast<int>(index));

					bool isSelected = (selectedFileIndex_ == static_cast<int>(index));
					ImVec2 pos = ImGui::GetCursorScreenPos();
					float itemHeight = cellSize + 20.0f;

					// Selectableを配置して背景ハイライトと選択イベントを処理
					if(ImGui::Selectable("##Selectable", isSelected, ImGuiSelectableFlags_AllowDoubleClick, ImVec2(cellSize, itemHeight))) {
						selectedFileIndex_ = static_cast<int>(index);
						if(ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
							if(file.isDirectory) {
								outRequestChangeDir = true;
								outNextTargetDir = file.path;
							} else {
								const ONEngine::Guid& guid = pAssetCollection_->GetAssetGuidFromPath(file.relativePath);
								ImGuiSelection::SetSelectedObject(guid, SelectionType::Asset);
							}
						}
					}

					// 右クリックでも選択する
					if(ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
						selectedFileIndex_ = static_cast<int>(index);
						ImGui::OpenPopup("FileContextMenu");
					}

					// セルの終端カーソル位置を記録
					ImVec2 nextCursorPos = ImGui::GetCursorScreenPos();

					// D&D処理（Selectableをドラッグ元にする）
					if(ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
						static AssetPayload payload;
						payload.filePath = file.relativePath;
						payload.guid = pAssetCollection_->GetAssetGuidFromPath(payload.filePath);

						const AssetPayload* assetPtr = &payload;
						ImGui::SetDragDropPayload("AssetData", &assetPtr, sizeof(AssetPayload*));

						if(file.displayTexture) {
							ImGui::Image((ImTextureID)(uintptr_t)file.displayTexture->GetSRVGPUHandle().ptr, { 32.0f, 32.0f });
							ImGui::SameLine();
						}
						ImGui::Text("%s", name.c_str());
						ImGui::EndDragDropSource();
					}

					// 重ねて描画するために位置を上に戻す
					ImGui::SetCursorScreenPos(pos);
					ImGui::BeginGroup();

					// アイコン描画
					ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4.0f, 4.0f));
					if(file.displayTexture) {
						// Selectableの上にあるためImageを使用
						ImGui::Image((ImTextureID)(uintptr_t)file.displayTexture->GetSRVGPUHandle().ptr, { iconSize, iconSize });
					} else {
						ImGui::Button("Icon", { iconSize, iconSize });
					}
					ImGui::PopStyleVar();

					// 名前描画
					ImGui::SetCursorScreenPos(ImVec2(pos.x + 4.0f, pos.y + iconSize + 12.0f));
					ImGui::PushTextWrapPos(pos.x + cellSize - 4.0f);
					ImGui::TextWrapped("%s", name.c_str());
					ImGui::PopTextWrapPos();

					ImGui::EndGroup();

					// カーソル位置をSelectableの直後に戻す
					ImGui::SetCursorScreenPos(nextCursorPos);

					// コンテキストメニューの呼び出し（削除予約パスを渡す）
					PopupContextMenu(file.path, pendingDeletePath);

					ImGui::PopID();
				}
			}
		}
		ImGui::EndTable();
	}

	// 空スペースクリックでの選択解除
	if(ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows) && ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !ImGui::IsAnyItemHovered()) {
		selectedFileIndex_ = -1;
	}

	// --- キーボード操作の処理 ---
	if(ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) {
		// Backspace: 親フォルダに戻る
		if(ImGui::IsKeyPressed(ImGuiKey_Backspace)) {
			bool isAtRoot = false;
			try {
				for(const auto& root : rootPaths_) {
					if(std::filesystem::exists(directory) && std::filesystem::exists(root) && std::filesystem::equivalent(directory, root)) {
						isAtRoot = true;
						break;
					}
				}
			} catch(...) {
				isAtRoot = true;
			}
			if(!isAtRoot) {
				outRequestChangeDir = true;
				outNextTargetDir = directory.parent_path();
			}
		}

		// Enter: フォルダに入る、またはファイルを開く
		if(ImGui::IsKeyPressed(ImGuiKey_Enter) || ImGui::IsKeyPressed(ImGuiKey_KeypadEnter)) {
			if(selectedFileIndex_ >= 0 && selectedFileIndex_ < static_cast<int>(files.size())) {
				auto& file = files[selectedFileIndex_];
				if(file.isDirectory) {
					outRequestChangeDir = true;
					outNextTargetDir = file.path;
				} else {
					const ONEngine::Guid& guid = pAssetCollection_->GetAssetGuidFromPath(file.relativePath);
					ImGuiSelection::SetSelectedObject(guid, SelectionType::Asset);
				}
			}
		}

		// Delete: 削除
		if(ImGui::IsKeyPressed(ImGuiKey_Delete)) {
			if(selectedFileIndex_ >= 0 && selectedFileIndex_ < static_cast<int>(files.size())) {
				pendingDeletePath = files[selectedFileIndex_].path;
				selectedFileIndex_ = -1;
			}
		}

		// F2: リネームポップアップを開く
		if(ImGui::IsKeyPressed(ImGuiKey_F2)) {
			if(selectedFileIndex_ >= 0 && selectedFileIndex_ < static_cast<int>(files.size())) {
				auto& file = files[selectedFileIndex_];
				showRenamePopup_ = true;
				targetPath_ = file.path;
				inputBuffer_ = file.path.stem().string();
			}
		}

		// 矢印キーでの選択移動
		if(ImGui::IsKeyPressed(ImGuiKey_LeftArrow)) {
			if(selectedFileIndex_ > 0) {
				selectedFileIndex_--;
			} else if(selectedFileIndex_ == -1 && !files.empty()) {
				selectedFileIndex_ = 0;
			}
		}
		if(ImGui::IsKeyPressed(ImGuiKey_RightArrow)) {
			if(selectedFileIndex_ < static_cast<int>(files.size()) - 1) {
				selectedFileIndex_++;
			} else if(selectedFileIndex_ == -1 && !files.empty()) {
				selectedFileIndex_ = 0;
			}
		}
		if(ImGui::IsKeyPressed(ImGuiKey_UpArrow)) {
			if(selectedFileIndex_ >= columnCount) {
				selectedFileIndex_ -= columnCount;
			} else if(selectedFileIndex_ == -1 && !files.empty()) {
				selectedFileIndex_ = 0;
			}
		}
		if(ImGui::IsKeyPressed(ImGuiKey_DownArrow)) {
			if(selectedFileIndex_ + columnCount < static_cast<int>(files.size())) {
				selectedFileIndex_ += columnCount;
			} else if(selectedFileIndex_ == -1 && !files.empty()) {
				selectedFileIndex_ = 0;
			}
		}
	}

	// 描画ループを完全に抜けた後で、安全に削除処理とキャッシュ更新を行う
	if(!pendingDeletePath.empty()) {
		try {
			std::filesystem::remove_all(pendingDeletePath);

			// 元のファイルパスの末尾に ".meta" を付けたパスを作成
			std::filesystem::path metaPath = pendingDeletePath.string() + ".meta";
			if(std::filesystem::exists(metaPath)) {
				std::filesystem::remove(metaPath);
			}

			// 削除後、キャッシュを更新してUIに反映
			UpdateFileCache(directory);
			UpdateDirectoryCache(directory);
		} catch(const std::exception& e) {
			std::cerr << "Failed to delete file/folder: " << e.what() << std::endl;
		}
	}
}

///
/// ファイルを右クリックしたときの処理
///
void ProjectWindow::PopupContextMenu(const std::filesystem::path& filepath, std::filesystem::path& outDeletedPath) {
	if(ImGui::BeginPopup("FileContextMenu")) {
		if(ImGui::MenuItem("Reload")) {
			std::string path = GetRelativePath(filepath);
			HotReloadManager::GetInstance().RequestAssetReload(path);

			// スクリプトならホットリロードも要求
			if(path.ends_with(".cs")) {
				HotReloadManager::GetInstance().RequestScriptHotReload();
			}
		}

		if(ImGui::MenuItem("Rename")) {
			showRenamePopup_ = true;
			targetPath_ = filepath;
			inputBuffer_ = filepath.stem().string();
		}

		ImGui::Separator();

		if(ImGui::MenuItem("Show in Explorer")) {
			std::filesystem::path absolutePath = std::filesystem::absolute(filepath);
			std::wstring params = L"/select,\"" + absolutePath.wstring() + L"\"";
			ShellExecuteW(NULL, L"open", L"explorer.exe", params.c_str(), NULL, SW_SHOWNORMAL);
		}

		if(ImGui::MenuItem("Copy Path")) {
			std::string pathStr = filepath.string();
			std::replace(pathStr.begin(), pathStr.end(), '\\', '/');
			ImGui::SetClipboardText(pathStr.c_str());
		}

		// --- 削除機能の追加 ---
		ImGui::Separator();
		// 赤色で少し危険な操作であることをアピール
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.4f, 0.4f, 1.0f));
		if(ImGui::MenuItem("Delete")) {
			// ここでは削除せず、削除対象のパスを呼び出し元に伝えるだけ
			outDeletedPath = filepath;
		}
		ImGui::PopStyleColor();
		// ----------------------

		ImGui::EndPopup();
	}
}

///
/// ディレクトリのキャッシュを更新する
///
void ProjectWindow::UpdateDirectoryCache(const std::filesystem::path& directory) {
	if(!std::filesystem::exists(directory)) {
		directoryCache_.erase(directory.string());
		return;
	}

	std::vector<FileItem> subdirectories;
	try {
		for(const auto& entry : std::filesystem::directory_iterator(directory)) {
			if(!entry.is_directory()) continue;

			FileItem item;
			item.path = entry.path();
			item.isDirectory = true;
			subdirectories.push_back(item);

			UpdateDirectoryCache(entry.path());
		}
	} catch(...) {}

	directoryCache_[directory.string()] = std::move(subdirectories);
}

void ProjectWindow::UpdateFileCache(const std::filesystem::path& directory) {
	if(!std::filesystem::exists(directory)) {
		fileCache_.erase(directory.string());
		return;
	}

	std::vector<FileItem> files;
	try {
		for(const auto& entry : std::filesystem::directory_iterator(directory)) {
			if(entry.path().extension() == ".meta") continue;

			FileItem item;
			item.path = entry.path();
			item.isDirectory = entry.is_directory();
			item.relativePath = GetRelativePath(entry.path());

			// 拡張子を取得して小文字化
			std::string ext = item.path.extension().string();
			std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

			// --- アイコン決定ロジック ---
			if(item.isDirectory) {
				item.displayTexture = pAssetCollection_->GetTexture("./Packages/Textures/ImGui/FileIcons/FolderIcon.png");
				if(!item.displayTexture) item.displayTexture = pAssetCollection_->GetTexture("./Packages/Textures/ImGui/FileIcons/FolderIcon.dds");
			} else if(ONEngine::Asset::CheckAssetType(ext, ONEngine::Asset::AssetType::Texture)) {
				item.displayTexture = pAssetCollection_->GetTexture(item.relativePath);

				// TextureCubeやTexture3Dの場合はプレビューを表示するとシェーダー側でクラッシュするため、
				// プレビュー用テクスチャを無効にしてデフォルトアイコンを表示させる
				if(item.displayTexture && !item.displayTexture->IsStandard2D()) {
					item.displayTexture = nullptr;
				}
			} else if(ONEngine::Asset::CheckAssetType(ext, ONEngine::Asset::AssetType::Audio)) {
				item.displayTexture = pAssetCollection_->GetTexture("./Packages/Textures/ImGui/FileIcons/lets-icons-sound-none-256.png");
			} else if(ext == ".cs") {
				item.displayTexture = pAssetCollection_->GetTexture("./Packages/Textures/ImGui/FileIcons/ph-file-c-sharp-none-256.png");
			}

			// いずれにも当てはまらない、または読み込み失敗時のデフォルト
			if(!item.displayTexture) {
				item.displayTexture = pAssetCollection_->GetTexture("./Packages/Textures/ImGui/FileIcons/FileIcon.png");
			}

			files.push_back(item);
		}
	} catch(...) {}

	fileCache_[directory.string()] = std::move(files);
}