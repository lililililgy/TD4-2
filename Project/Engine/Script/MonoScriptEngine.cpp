#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")
#include "MonoScriptEngine.h"
#include "InternalCalls/AddInternalMethods.h"

#include <windows.h>
#include <filesystem>

using namespace ONEngine;

/// std
#include <regex>
#include <thread>
#include <chrono>
#include <fstream>
#include <ctime>
#include <iomanip>
#include <sstream>

/// externals
#include <metadata/mono-config.h>
#include <mono/metadata/object.h>
#include <mono/metadata/class.h>
#include <mono/metadata/tokentype.h>
#include <mono/metadata/blob.h>
#include <mono/metadata/debug-helpers.h>
#include <mono/metadata/threads.h>
#include <mono/metadata/image.h>
#include <mono/metadata/mono-gc.h>


/// engine
#include "Engine/Core/Config/EngineConfig.h"
#include "Engine/Core/Utility/Utility.h"
#include "Engine/Core/Utility/FileSystem/FileSystem.h"
#include "Engine/ECS/EntityComponentSystem/EntityComponentSystem.h"
#include "Engine/ECS/EntityComponentSystem/ComponentApplyFunc.h"
#include "InternalCalls/AddInternalMethods.h"
#include "InternalCalls/EventInternalCalls.h"

#include <ws2tcpip.h>
#include <iphlpapi.h>
#pragma comment(lib, "iphlpapi.lib")

namespace {
// ポート 55555 に対する TCP 接続 (ESTABLISHED) が存在するかどうかを判定
bool IsDebuggerAttachedViaTcp() {
	ULONG size = 0;
	if (GetExtendedTcpTable(NULL, &size, FALSE, AF_INET, TCP_TABLE_OWNER_PID_ALL, 0) != ERROR_INSUFFICIENT_BUFFER) {
		return false;
	}

	std::vector<char> buffer(size);
	PMIB_TCPTABLE_OWNER_PID tcpTable = reinterpret_cast<PMIB_TCPTABLE_OWNER_PID>(buffer.data());

	if (GetExtendedTcpTable(tcpTable, &size, FALSE, AF_INET, TCP_TABLE_OWNER_PID_ALL, 0) == NO_ERROR) {
		for (DWORD i = 0; i < tcpTable->dwNumEntries; ++i) {
			USHORT localPort = ntohs((USHORT)tcpTable->table[i].dwLocalPort);
			if (localPort == 55555) {
				if (tcpTable->table[i].dwState == MIB_TCP_STATE_ESTAB) {
					return true;
				}
			}
		}
	}
	return false;
}

// ポート 55555 に対する TCP コネクション状態を詳細ログ出力する
void LogTcpConnections() {
	ULONG size = 0;
	if (GetExtendedTcpTable(NULL, &size, FALSE, AF_INET, TCP_TABLE_OWNER_PID_ALL, 0) != ERROR_INSUFFICIENT_BUFFER) {
		Console::LogWarning("[MonoDbg] Failed to query size of ExtendedTcpTable", LogCategory::ScriptEngine);
		return;
	}

	std::vector<char> buffer(size);
	PMIB_TCPTABLE_OWNER_PID tcpTable = reinterpret_cast<PMIB_TCPTABLE_OWNER_PID>(buffer.data());

	if (GetExtendedTcpTable(tcpTable, &size, FALSE, AF_INET, TCP_TABLE_OWNER_PID_ALL, 0) == NO_ERROR) {
		Console::Log("[MonoDbg] Dump TCP Connections on Port 55555 (Total entries: " + std::to_string(tcpTable->dwNumEntries) + "):", LogCategory::ScriptEngine);
		bool found = false;
		for (DWORD i = 0; i < tcpTable->dwNumEntries; ++i) {
			USHORT localPort = ntohs((USHORT)tcpTable->table[i].dwLocalPort);
			USHORT remotePort = ntohs((USHORT)tcpTable->table[i].dwRemotePort);
			if (localPort == 55555 || remotePort == 55555) {
				found = true;
				std::string stateStr;
				switch (tcpTable->table[i].dwState) {
					case MIB_TCP_STATE_CLOSED: stateStr = "CLOSED"; break;
					case MIB_TCP_STATE_LISTEN: stateStr = "LISTEN"; break;
					case MIB_TCP_STATE_SYN_SENT: stateStr = "SYN_SENT"; break;
					case MIB_TCP_STATE_SYN_RCVD: stateStr = "SYN_RCVD"; break;
					case MIB_TCP_STATE_ESTAB: stateStr = "ESTABLISHED"; break;
					case MIB_TCP_STATE_FIN_WAIT1: stateStr = "FIN_WAIT1"; break;
					case MIB_TCP_STATE_FIN_WAIT2: stateStr = "FIN_WAIT2"; break;
					case MIB_TCP_STATE_CLOSE_WAIT: stateStr = "CLOSE_WAIT"; break;
					case MIB_TCP_STATE_CLOSING: stateStr = "CLOSING"; break;
					case MIB_TCP_STATE_LAST_ACK: stateStr = "LAST_ACK"; break;
					case MIB_TCP_STATE_TIME_WAIT: stateStr = "TIME_WAIT"; break;
					case MIB_TCP_STATE_DELETE_TCB: stateStr = "DELETE_TCB"; break;
					default: stateStr = "UNKNOWN (" + std::to_string(tcpTable->table[i].dwState) + ")"; break;
				}
				Console::Log("[MonoDbg]   Entry: LocalPort=" + std::to_string(localPort) +
					", RemotePort=" + std::to_string(remotePort) +
					", State=" + stateStr +
					", PID=" + std::to_string(tcpTable->table[i].dwOwningPid),
					LogCategory::ScriptEngine);
			}
		}
		if (!found) {
			Console::Log("[MonoDbg]   No connections found on port 55555.", LogCategory::ScriptEngine);
		}
	} else {
		Console::LogWarning("[MonoDbg] Failed to retrieve ExtendedTcpTable", LogCategory::ScriptEngine);
	}
}

// デバッグ用ポート競合検知関数
bool IsDebugPortInUse(unsigned short port) {
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		return false;
	}
	SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (s == INVALID_SOCKET) {
		WSACleanup();
		return false;
	}

	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	addr.sin_port = htons(port);

	bool inUse = false;
	if (bind(s, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
		int err = WSAGetLastError();
		if (err == WSAEADDRINUSE) {
			inUse = true;
		}
	}

	closesocket(s);
	WSACleanup();
	return inUse;
}

void LogCallback(const char* log_domain, const char* log_level, const char* message, mono_bool fatal, void*) {
	const char* domain = log_domain ? log_domain : "null";
	const char* level = log_level ? log_level : "null";
	const char* msg = message ? message : "null";

	std::string log = "[" + std::string(domain) + "][" + std::string(level) + "] " + msg;
	if(fatal) log += " (fatal)";

	Console::Log(log, LogCategory::ScriptEngine);
}

void ConsoleLog(MonoString* msg, LogCategory category) {
	// MonoString* -> const char* 変換
	char* cstr = mono_string_to_utf8(msg);
	Console::Log(cstr, category);
	mono_free(cstr);
}

void ApplicationQuit() {
	PostQuitMessage(0);
}

MonoAssembly* LoadAssemblyWithSymbols(MonoDomain* domain, const std::string& dllPath, std::vector<char>& outPdbBuffer) {
	// アセンブリのロード開始時の診断ログを出力
	Console::Log("[Mono] LoadAssemblyWithSymbols started for: " + dllPath, LogCategory::ScriptEngine);
	if (std::filesystem::exists(dllPath)) {
		auto ftime = std::filesystem::last_write_time(dllPath);
		auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(ftime - std::filesystem::file_time_type::clock::now() + std::chrono::system_clock::now());
		std::time_t ctime = std::chrono::system_clock::to_time_t(sctp);
		char timeBuf[100];
		std::tm timeInfo;
		localtime_s(&timeInfo, &ctime);
		std::strftime(timeBuf, sizeof(timeBuf), "%Y-%m-%d %H:%M:%S", &timeInfo);
		Console::Log("[Mono]   DLL last modified: " + std::string(timeBuf), LogCategory::ScriptEngine);
	}

	// デバッガがロードできるように、このアセンブリに対応する DLL と PDB を固定名 "CSharpLibrary.dll" / "CSharpLibrary.pdb" として配置する。
	// アセンブリのロード前に配置することで、デバッガ接続時に最新の DLL と PDB を確実にロードさせます。
	{
		std::string latestDllPath = dllPath;
		std::string latestPdbPath = dllPath;
		size_t extPos = latestPdbPath.find_last_of('.');
		if (extPos != std::string::npos) {
			latestPdbPath = latestPdbPath.substr(0, extPos) + ".pdb";
		} else {
			latestPdbPath += ".pdb";
		}

		std::string targetDllPath = "./Packages/Scripts/CSharpLibrary.dll";
		std::string targetPdbPath = "./Packages/Scripts/CSharpLibrary.pdb";

		if (std::filesystem::exists(latestDllPath) && latestDllPath != targetDllPath) {
			try {
				std::filesystem::copy_file(latestDllPath, targetDllPath, std::filesystem::copy_options::overwrite_existing);
				Console::Log("[Mono] LoadAssembly: Copied DLL to logical path: " + targetDllPath, LogCategory::ScriptEngine);
			} catch (const std::exception& e) {
				Console::LogWarning("[Mono] LoadAssembly: Failed to copy DLL to logical path (it might be locked): " + std::string(e.what()), LogCategory::ScriptEngine);
			}
		}

		if (std::filesystem::exists(latestPdbPath) && latestPdbPath != targetPdbPath) {
			try {
				std::filesystem::copy_file(latestPdbPath, targetPdbPath, std::filesystem::copy_options::overwrite_existing);
				Console::Log("[Mono] LoadAssembly: Copied PDB to logical path: " + targetPdbPath, LogCategory::ScriptEngine);
			} catch (const std::exception& e) {
				Console::LogWarning("[Mono] LoadAssembly: Failed to copy PDB to logical path (it might be locked): " + std::string(e.what()), LogCategory::ScriptEngine);
			}
		}
	}

#if defined(DEBUG_MODE)
	std::string pdbPath = dllPath;
	size_t extPos = pdbPath.find_last_of('.');
	if (extPos != std::string::npos) {
		pdbPath = pdbPath.substr(0, extPos) + ".pdb";
	} else {
		pdbPath += ".pdb";
	}

	std::ifstream dllFile(dllPath, std::ios::binary | std::ios::ate);
	std::ifstream pdbFile(pdbPath, std::ios::binary | std::ios::ate);

	if (dllFile.is_open() && pdbFile.is_open()) {
		std::streamsize dllSize = dllFile.tellg();
		dllFile.seekg(0, std::ios::beg);
		std::vector<char> dllBuffer(dllSize);

		std::streamsize pdbSize = pdbFile.tellg();
		pdbFile.seekg(0, std::ios::beg);
		outPdbBuffer.resize(pdbSize);

		if (dllFile.read(dllBuffer.data(), dllSize) && pdbFile.read(outPdbBuffer.data(), pdbSize)) {
			// 論理アセンブリ名（CSharpLibrary.dll）に置き換えて Mono に報告し、デバッガがソースコードとマッピングできるようにする
			std::string logicalDllPath = dllPath;
			size_t lastSlash = logicalDllPath.find_last_of("/\\");
			std::string dir = (lastSlash != std::string::npos) ? logicalDllPath.substr(0, lastSlash + 1) : "";
			std::string logicalPath = dir + "CSharpLibrary.dll";

			MonoImageOpenStatus status = MONO_IMAGE_OK;
			// DLLデータからMonoImageをオープン (論理パスを報告)
			MonoImage* image = mono_image_open_from_data_with_name(
				dllBuffer.data(), 
				(uint32_t)dllSize, 
				true, // need_copy
				&status, 
				false, // refonly
				logicalPath.c_str()
			);

			if (image && status == MONO_IMAGE_OK) {
				// アセンブリロードの前にデバッグ情報を登録
				mono_debug_open_image_from_memory(image, (const mono_byte*)outPdbBuffer.data(), (int)pdbSize);
				
				// ImageからAssemblyをロード (論理パスを報告)
				MonoAssembly* assembly = mono_assembly_load_from_full(
					image,
					logicalPath.c_str(),
					&status,
					false
				);

				mono_image_close(image);

				if (assembly) {
					Console::Log("[Mono] Successfully loaded assembly and debug symbols from memory: " + dllPath + " (logical: " + logicalPath + ")", LogCategory::ScriptEngine);
					return assembly;
				}
			}
		}
	}
	Console::LogWarning("[Mono] Failed to load assembly from memory with symbols. Falling back to file load: " + dllPath, LogCategory::ScriptEngine);
#endif
	return mono_domain_assembly_open(domain, dllPath.c_str());
}

std::string GetUtf8Path(const std::filesystem::path& path) {
	std::filesystem::path absPath = std::filesystem::absolute(path);
	std::wstring wpath = absPath.wstring();
	int len = WideCharToMultiByte(CP_UTF8, 0, wpath.c_str(), -1, nullptr, 0, nullptr, nullptr);
	if (len > 0) {
		std::string utf8Path(len - 1, '\0');
		WideCharToMultiByte(CP_UTF8, 0, wpath.c_str(), -1, &utf8Path[0], len, nullptr, nullptr);
		return utf8Path;
	}
	return absPath.string();
}

}


MonoScriptEngine::MonoScriptEngine() : domainReloadCounter_(0) {}
MonoScriptEngine::~MonoScriptEngine() = default;

MonoScriptEngine& MonoScriptEngine::GetInstance() {
	static MonoScriptEngine instance;
	return instance;
}

void MonoScriptEngine::Initialize() {
	Console::Log("[Mono] Building C# project...", LogCategory::ScriptEngine);
	std::string buildOutput;
	bool buildSuccess = BuildCSharpProject(buildOutput);
	if (buildSuccess) {
		Console::Log("[Mono] C# project built successfully.", LogCategory::ScriptEngine);
	} else {
		Console::LogError("[Mono] C# project build failed!\n" + buildOutput, LogCategory::ScriptEngine);
	}

	// PATH 設定 (OSレベルA/W & CRTレベルA/W) - staticにしてメモリを永続化
	static std::string monoBinA = std::filesystem::absolute("Packages/mono/bin").string();
	static std::wstring monoBinW = std::filesystem::absolute("Packages/mono/bin").wstring();
	static std::string pathEnvA = "PATH=" + monoBinA + ";C:\\Windows\\System32";
	static std::wstring pathEnvW = L"PATH=" + monoBinW + L";C:\\Windows\\System32";
	SetEnvironmentVariableA("PATH", (monoBinA + ";C:\\Windows\\System32").c_str());
	SetEnvironmentVariableW(L"PATH", (monoBinW + L";C:\\Windows\\System32").c_str());
	_putenv(pathEnvA.c_str());
	_wputenv(pathEnvW.c_str());

	// MONO_PATH 設定 (OSレベルA/W & CRTレベルA/W) - staticにしてメモリを永続化
	static std::string monoLibA = std::filesystem::absolute("Packages/mono/lib/4.5").string();
	static std::wstring monoLibW = std::filesystem::absolute("Packages/mono/lib/4.5").wstring();
	static std::string monoPathEnvA = "MONO_PATH=" + monoLibA;
	static std::wstring monoPathEnvW = L"MONO_PATH=" + monoLibW;
	SetEnvironmentVariableA("MONO_PATH", monoLibA.c_str());
	SetEnvironmentVariableW(L"MONO_PATH", monoLibW.c_str());
	_putenv(monoPathEnvA.c_str());
	_wputenv(monoPathEnvW.c_str());

#if defined(DEBUG_MODE)
	// Monoの診断ログ出力を詳細化 - staticにしてメモリを永続化
	static std::string logEnv1 = "MONO_LOG_LEVEL=debug";
	static std::string logEnv2 = "MONO_LOG_MASK=asm,dll,gc,cfg";
	SetEnvironmentVariableA("MONO_LOG_LEVEL", "debug");
	_putenv(logEnv1.c_str());
	SetEnvironmentVariableA("MONO_LOG_MASK", "asm,dll,gc,cfg");
	_putenv(logEnv2.c_str());

	// デバッグシーケンスポイント生成を強化 - staticにしてメモリを永続化
	static std::string debugEnv1 = "MONO_DEBUG=gen-compact-seq-points";
	SetEnvironmentVariableA("MONO_DEBUG", "gen-compact-seq-points");
	_putenv(debugEnv1.c_str());

	/// デバッグモード用のオプション設定
	bool waitDebug = true;
	LPWSTR cmdLine = GetCommandLineW();
	if (cmdLine && wcsstr(cmdLine, L"--test-mode") != nullptr) {
		waitDebug = false;
	}

	// ポートが既に他のプロセスに占有されているか事前に競合検知
	if (waitDebug && IsDebugPortInUse(55555)) {
		Console::LogError("[Mono] WARNING: Debugger port 55555 is ALREADY IN USE by another process! Mono Debugger binding may fail.", LogCategory::ScriptEngine);
	}

	// アドレス解決不具合を防ぐため IPv4 0.0.0.0 でバインド - staticにしてメモリを永続化
	static std::string debugAgentOptA;
	static std::wstring debugAgentOptW;
	debugAgentOptA = waitDebug 
		? "--debugger-agent=transport=dt_socket,address=0.0.0.0:55555,server=y,suspend=y"
		: "--debugger-agent=transport=dt_socket,address=0.0.0.0:55555,server=y,suspend=n";
	debugAgentOptW = waitDebug 
		? L"--debugger-agent=transport=dt_socket,address=0.0.0.0:55555,server=y,suspend=y"
		: L"--debugger-agent=transport=dt_socket,address=0.0.0.0:55555,server=y,suspend=n";

	// MONO_ENV_OPTIONS と MONO_OPTIONS の両方に環境変数を設定 (A/W両対応) - staticにしてメモリを永続化
	static std::string monoEnvOptA = "MONO_ENV_OPTIONS=" + debugAgentOptA;
	static std::wstring monoEnvOptW = L"MONO_ENV_OPTIONS=" + debugAgentOptW;
	SetEnvironmentVariableA("MONO_ENV_OPTIONS", debugAgentOptA.c_str());
	SetEnvironmentVariableW(L"MONO_ENV_OPTIONS", debugAgentOptW.c_str());
	_putenv(monoEnvOptA.c_str());
	_wputenv(monoEnvOptW.c_str());

	static std::string monoOptA = "MONO_OPTIONS=" + debugAgentOptA;
	static std::wstring monoOptW = L"MONO_OPTIONS=" + debugAgentOptW;
	SetEnvironmentVariableA("MONO_OPTIONS", debugAgentOptA.c_str());
	SetEnvironmentVariableW(L"MONO_OPTIONS", debugAgentOptW.c_str());
	_putenv(monoOptA.c_str());
	_wputenv(monoOptW.c_str());

	Console::Log("[Mono] Debug Mode: waitDebug = " + std::string(waitDebug ? "true (suspend=y)" : "false (suspend=n)"), LogCategory::ScriptEngine);

	// mono_jit_parse_options にもデバッガオプションを確実に渡す (ポインタが永続メモリを指すようにする)
	const char* debugOptions[] = {
		"--soft-breakpoints",
		debugAgentOptA.c_str()
	};
	mono_jit_parse_options(sizeof(debugOptions) / sizeof(char*), (char**)debugOptions);
	mono_debug_init(MONO_DEBUG_FORMAT_MONO);

	// デバッガが物理的にアタッチされる前であっても、JIT時に常にデバッグ行情報（シーケンスポイント）を
	// 生成させるために、Monoのデバッガ接続フラグを強制的にONに設定します。
	// これにより、waitDebug=false時の後発アタッチや、デバッガ再アタッチ時にもブレイクポイントが確実に効くようになります。
	mono_set_is_debugger_attached(true);
#else
	Console::Log("[Mono] Non-Debug Mode: Debugger Disabled (suspend=n)", LogCategory::ScriptEngine);
	/// 高速化用オプション
	const char* options[] = {
		"--optimize=all",   // JIT最適化フル
	};
	mono_jit_parse_options(sizeof(options) / sizeof(char*), (char**)options);
#endif

	/// ログ出力(任意、デバッグ時だけでもOK)
	mono_trace_set_level_string("info");
	mono_trace_set_log_handler(LogCallback, nullptr);

	/// versionの出力
	Console::Log("Mono version: " + std::string(mono_get_runtime_build_info()), LogCategory::ScriptEngine);

	/// Monoの検索パス設定
	std::string scriptsLib = GetUtf8Path("Packages/Scripts/lib");
	std::string monoEtc = GetUtf8Path("Externals/mono/etc");
	mono_set_dirs(scriptsLib.c_str(), monoEtc.c_str());
	mono_config_parse(nullptr);

	/// JIT初期化 (v4.x CLRターゲット)
	rootDomain_ = mono_jit_init_version("MyRootDomain", "v4.0.30319");
	if(!rootDomain_) {
		Console::LogError("Failed to initialize Mono JIT", LogCategory::ScriptEngine);
		return;
	}

#if defined(DEBUG_MODE)
	// ルートドメイン用のデバッグ情報を登録
	mono_debug_domain_create(rootDomain_);
#endif

	// デバッグモード時でも、再生ごとのC#状態リセットを確実にするため、
	// AppDomainを常に再作成するように変更
	domain_ = CreateReloadDomain();
	if(!domain_) {
		Console::LogError("Failed to create Mono domain for initialization", LogCategory::ScriptEngine);
		return;
	}
	mono_domain_set(domain_, true);

	auto latestDll = FindLatestDll("./Packages/Scripts", "CSharpLibrary");
	if(!latestDll.has_value()) {
		Console::LogError("Failed to find latest assembly DLL.", LogCategory::ScriptEngine);
		return;
	}

	currentDllPath_ = GetUtf8Path(*latestDll);
	assembly_ = LoadAssemblyWithSymbols(domain_, currentDllPath_, activePdbBuffer_);
	if(!assembly_) {
		Console::LogError("Failed to load CSharpLibrary.dll", LogCategory::ScriptEngine);
		return;
	}

	image_ = mono_assembly_get_image(assembly_);
	if(!image_) {
		Console::LogError("Failed to get image from assembly", LogCategory::ScriptEngine);
		return;
	}

	RegisterFunctions();
}

void MonoScriptEngine::Finalize() {
	if(rootDomain_) {
		ResetCS();
		mono_jit_cleanup(rootDomain_);
		rootDomain_ = nullptr;
		domain_ = nullptr;
	}
}

void MonoScriptEngine::RegisterFunctions() {
	/// 関数の登録
	AddComponentInternalCalls();
	AddEntityInternalCalls();
	AddEventInternalCalls();

	/// log
	mono_add_internal_call("Debug::InternalConsoleLog", (void*)ConsoleLog);

	/// application
	mono_add_internal_call("Application::InternalQuit", (void*)ApplicationQuit);

	/// time
	mono_add_internal_call("Time::InternalGetDeltaTime", (void*)Time::DeltaTime);
	mono_add_internal_call("Time::InternalGetTime", (void*)Time::GetTime);
	mono_add_internal_call("Time::InternalGetUnscaledDeltaTime", (void*)Time::UnscaledDeltaTime);
	mono_add_internal_call("Time::InternalGetTimeScale", (void*)Time::TimeScale);
	mono_add_internal_call("Time::InternalSetTimeScale", (void*)Time::SetTimeScale);

	mono_add_internal_call("Mathf::LoadFile", (void*)MonoInternalMethods::LoadFile);

	/// 他のクラスの関数も登録
	AddInputInternalCalls();
	AddSceneInternalCalls();
	AddGizmoInternalCalls();
	AddWindowInternalCalls();
	AddAnimationInternalCalls();
	ComponentApplyFuncs::Initialize(image_);

	// データ同期用のC#メソッドを取得
	{
		// static class ComponentBatchManager
		receiveAllBatchesMethod_ = GetMethodFromCS("", "ComponentBatchManager", "ReceiveAllBatches", 2);

		// static class EntityComponentSystem
		getEcsGroupMethod_ = GetMethodFromCS("", "EntityComponentSystem", "GetECSGroup", 1);
		addEcsGroupMethod_ = GetMethodFromCS("", "EntityComponentSystem", "AddECSGroup", 1);
		clearEcsGroupMethod_ = GetMethodFromCS("", "EntityComponentSystem", "ClearECSGroup", 1);

		// class ECSGroup
		MonoClass* ecsGroupClass = mono_class_from_name(image_, "", "ECSGroup");
		if(ecsGroupClass) {
			getComponentCollectionField_ = MonoScriptEngineUtils::FindFieldRecursive(ecsGroupClass, "componentCollection");
			addEntityMethod_ = mono_class_get_method_from_name(ecsGroupClass, "AddEntity", 1);
		}

		// class Entity
		MonoClass* entityClass = mono_class_from_name(image_, "", "Entity");
		if(entityClass) {
			fetchInitialDataMethod_ = mono_class_get_method_from_name(entityClass, "FetchInitialData", 0);
		}

		// static class SceneManager
		MonoClass* sceneManagerClass = mono_class_from_name(image_, "", "SceneManager");
		if(sceneManagerClass) {
			sceneNameField_ = mono_class_get_field_from_name(sceneManagerClass, "sceneName_");
		}

		// AI
		updateAiIntentsMethod_ = GetMethodFromCS("", "AIUpdater", "UpdateIntents", 4);
		notifyEventCompletedMethod_ = GetMethodFromCS("", "BlackboardManager", "SetBool", 3);
	}

	ApplyCSharpLogSetting();
}

void MonoScriptEngine::HotReload() {
	Console::Log("[MonoDbg] ========================================================", LogCategory::ScriptEngine);
	Console::Log("[MonoDbg] MonoScriptEngine::HotReload() CALLED! Starting C# reload.", LogCategory::ScriptEngine);
	Console::Log("[MonoDbg] ========================================================", LogCategory::ScriptEngine);
	Console::Log("[Mono] HotReload: Rebuilding C# project...", LogCategory::ScriptEngine);
	std::string buildOutput;
	bool buildSuccess = BuildCSharpProject(buildOutput);
	if (!buildSuccess) {
		Console::LogError("[Mono] HotReload: C# project build failed! Reload aborted.\n" + buildOutput, LogCategory::ScriptEngine);
		return;
	}
	Console::Log("[Mono] HotReload: C# project built successfully. Reloading DLL...", LogCategory::ScriptEngine);

	// デバッグモード時でも再生ごとのリセットを優先するため、Hot Reloadを有効化

	MonoDomain* oldDomain = domain_;
	std::string oldDllPath = currentDllPath_;

	// 新しいアセンブリをロードする前に、古いドメインを完全にアンロード（解放）する。
	// これにより、デバッガ（VS Code）は古いアセンブリのアンロードを検知でき、
	// 同じ名前のアセンブリがメモリ上に重複してキャッシュの競合が発生するのを防ぎます。
	if (oldDomain && oldDomain != rootDomain_) {
		domainsToUnload_.push_back(oldDomain);
		ClearPendingDomains();
		domain_ = nullptr;
	}

	domain_ = CreateReloadDomain();
	mono_domain_set(domain_, true);

	if(domain_ != oldDomain) {
		Console::Log("Created new Mono domain for hot reload.", LogCategory::ScriptEngine);
	} else {
		Console::Log("Reusing existing Mono domain for hot reload.", LogCategory::ScriptEngine);
	}

	auto latestDll = FindLatestDll("./Packages/Scripts", "CSharpLibrary");
	if(!latestDll.has_value()) {
		Console::LogError("Failed to find latest assembly DLL.", LogCategory::ScriptEngine);
		domain_ = nullptr;
		return;
	}

	std::string utf8DllPath = GetUtf8Path(*latestDll);

	// コピー中の可能性があるため、ファイルが完全に書き込まれロック解除されるまで待つ
	{
		int retryCount = 0;
		const int maxRetries = 20; // 最大2秒
		bool fileReady = false;
		std::filesystem::path dllPath(*latestDll);
		while (retryCount < maxRetries) {
			std::ifstream file(dllPath, std::ios::binary | std::ios::in);
			if (file.good()) {
				fileReady = true;
				break;
			}
			retryCount++;
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}
		if (!fileReady) {
			Console::LogError("Latest DLL is still locked or inaccessible: " + utf8DllPath, LogCategory::ScriptEngine);
			domain_ = nullptr;
			return;
		}
	}

	// デバッガがロードできるように、最新の PDB を固定名 "CSharpLibrary.pdb" としてコピー配置する
	{
		std::string latestPdbPath = utf8DllPath;
		size_t extPos = latestPdbPath.find_last_of('.');
		if (extPos != std::string::npos) {
			latestPdbPath = latestPdbPath.substr(0, extPos) + ".pdb";
		} else {
			latestPdbPath += ".pdb";
		}

		std::string targetPdbPath = "./Packages/Scripts/CSharpLibrary.pdb";
		if (std::filesystem::exists(latestPdbPath) && latestPdbPath != targetPdbPath) {
			try {
				std::filesystem::copy_file(latestPdbPath, targetPdbPath, std::filesystem::copy_options::overwrite_existing);
				Console::Log("[Mono] HotReload: Copied latest PDB to logical path: " + targetPdbPath, LogCategory::ScriptEngine);
			} catch (const std::exception& e) {
				Console::LogWarning("[Mono] HotReload: Failed to copy PDB to logical path: " + std::string(e.what()), LogCategory::ScriptEngine);
			}
		}
	}

	// 古い PDB バッファを保留リストに退避（クリアはせず、ClearPendingDomains() が実際にアンロードするタイミングまで寿命を維持する）
	if (!activePdbBuffer_.empty()) {
		pendingPdbBuffers_.push_back(std::move(activePdbBuffer_));
	}

	assembly_ = LoadAssemblyWithSymbols(domain_, utf8DllPath, activePdbBuffer_);
	if(!assembly_) {
		Console::LogError("Failed to load assembly in new domain", LogCategory::ScriptEngine);
		domain_ = nullptr;
		return;
	}

	currentDllPath_ = utf8DllPath;
	image_ = mono_assembly_get_image(assembly_);
	RegisterFunctions();

	// ComponentBatchManagerの再初期化
	{
		MonoClass* batchMgrClass = mono_class_from_name(image_, "", "ComponentBatchManager");
		if(batchMgrClass) {
			MonoMethod* initMethod = mono_class_get_method_from_name(batchMgrClass, "Initialize", 0);
			if(initMethod) {
				MonoObject* exc = nullptr;
				MonoScriptEngineUtils::SafeInvoke(initMethod, nullptr, nullptr, &exc);
				if(exc) MonoScriptEngineUtils::HandleException(exc);
			}
		}
	}

	currentDllPath_ = utf8DllPath;

	SetIsHotReloadRequest(true);

	Console::Log("Reloaded assembly successfully in new domain.", LogCategory::ScriptEngine);
}

void MonoScriptEngine::ApplyCSharpLogSetting() {
	if (!image_ || !domain_) return;

	MonoClass* debugClass = mono_class_from_name(image_, "", "Debug");
	if (debugClass) {
		MonoClassField* ignoreLogField = mono_class_get_field_from_name(debugClass, "IgnoreLog");
		if (ignoreLogField) {
			mono_bool value = EngineConfig::ignoreCSharpLog ? 1 : 0;
			mono_field_static_set_value(mono_class_vtable(domain_, debugClass), ignoreLogField, &value);
		}
	}
}

void MonoScriptEngine::SetEcsPtr(EntityComponentSystem* ecs) {
	pEcs_ = ecs;
}

std::optional<std::string> MonoScriptEngine::FindLatestDll(const std::string& dirPath, const std::string& baseName) {
	// デバッグモード時でもホットリロード可能にするため、タイムスタンプ付きの最新DLLをロードする

	std::regex pattern(baseName + R"(_.*\.dll)"); // タイムスタンプ付きの全てのDLL
	std::optional<std::string> latestFile;
	std::filesystem::file_time_type latestTime;

	if(!std::filesystem::exists(dirPath)) {
		return std::nullopt;
	}

	for(const auto& entry : std::filesystem::directory_iterator(dirPath)) {
		if(!entry.is_regular_file()) {
			continue;
		}

		std::string filename = entry.path().filename().string();
		if(!std::regex_match(filename, pattern)) {
			continue;
		}

		auto currentTime = std::filesystem::last_write_time(entry.path());

		if(!latestFile || currentTime > latestTime) {
			latestFile = entry.path().string();
			latestTime = currentTime;
		}
	}

	// タイムスタンプ付きが見つからなかった場合のフォールバック (CSharpLibrary.dll)
	if(!latestFile) {
		std::regex fallbackPattern(baseName + R"(\.dll)");
		for(const auto& entry : std::filesystem::directory_iterator(dirPath)) {
			if(!entry.is_regular_file()) {
				continue;
			}

			std::string filename = entry.path().filename().string();
			if(std::regex_match(filename, fallbackPattern)) {
				latestFile = entry.path().string();
				break;
			}
		}
	}

	if(latestFile) {
		Console::Log("Latest DLL found: " + *latestFile, LogCategory::ScriptEngine);
	}

	return latestFile;
}

void MonoScriptEngine::ResetCS() {
	if (!image_ || !domain_) {
		return;
	}

	MonoClass* monoClass = mono_class_from_name(image_, "", "EntityComponentSystem");
	if(!monoClass) {
		Console::LogError("Failed to find class: EntityComponentSystem", LogCategory::ScriptEngine);
		return;
	}

	MonoMethod* method = mono_class_get_method_from_name(monoClass, "DeleteEntityAll", 0);
	if(!method) {
		Console::LogError("Failed to find method: DeleteEntityAll", LogCategory::ScriptEngine);
		return;
	}

	MonoObject* exc = nullptr;
	MonoScriptEngineUtils::SafeInvoke(method, nullptr, nullptr, &exc);

	if(exc) {
		MonoScriptEngineUtils::HandleException(exc);
		return;
	}
}

MonoObject* MonoScriptEngine::GetEntityFromCS(const std::string& ecsGroupName, int32_t entityId) {
	mono_thread_attach(domain_);

	MonoClass* monoClass = mono_class_from_name(image_, "", "EntityComponentSystem");
	if(!monoClass) {
		Console::LogError("Failed to find class: EntityComponentSystem", LogCategory::ScriptEngine);
		return nullptr;
	}

	MonoMethod* method = mono_class_get_method_from_name(monoClass, "GetEntity", 2);
	if(!method) {
		Console::LogError("Failed to find method: GetEntity", LogCategory::ScriptEngine);
		return nullptr;
	}

	void* args[2];
	args[0] = mono_string_new(domain_, ecsGroupName.c_str());
	args[1] = &entityId;

	MonoObject* exc = nullptr;
	MonoObject* result = MonoScriptEngineUtils::SafeInvoke(method, nullptr, args, &exc);
	if(exc) {
		MonoScriptEngineUtils::HandleException(exc);
		return nullptr;
	}

	return result;
}

MonoObject* MonoScriptEngine::GetMonoBehaviorFromCS(const std::string& ecsGroupName, int32_t entityId, const std::string& behaviorName) {
	mono_thread_attach(domain_);

	MonoClass* monoClass = mono_class_from_name(image_, "", "EntityComponentSystem");
	if(!monoClass) {
		Console::LogError("Failed to find class: EntityComponentSystem", LogCategory::ScriptEngine);
		return nullptr;
	}

	MonoMethod* method = mono_class_get_method_from_name(monoClass, "GetMonoBehavior", 3);
	if(!method) {
		Console::LogError("Failed to find method: GetMonoBehavior", LogCategory::ScriptEngine);
		return nullptr;
	}

	void* args[3];
	args[0] = mono_string_new(domain_, ecsGroupName.c_str());
	args[1] = &entityId;
	args[2] = mono_string_new(domain_, behaviorName.c_str());

	MonoObject* exc = nullptr;
	MonoObject* result = MonoScriptEngineUtils::SafeInvoke(method, nullptr, args, &exc);
	if(exc) {
		MonoScriptEngineUtils::HandleException(exc);
		return nullptr;
	}

	return result;
}

MonoObject* MonoScriptEngine::GetEcsGroupObject(const std::string& groupName) {
	mono_thread_attach(domain_);

	if(!getEcsGroupMethod_) {
		Console::LogError("getEcsGroupMethod_ is null", LogCategory::ScriptEngine);
		return nullptr;
	}

	void* args[1];
	args[0] = mono_string_new(domain_, groupName.c_str());

	MonoObject* exc = nullptr;
	MonoObject* result = MonoScriptEngineUtils::SafeInvoke(getEcsGroupMethod_, nullptr, args, &exc);
	if(exc) {
		MonoScriptEngineUtils::HandleException(exc);
		return nullptr;
	}

	return result;
}

GameEntity* MonoScriptEngine::GetOwnerEntity(MonoObject* obj) {
	if(!obj || !image_ || !pEcs_) return nullptr;

	MonoClass* klass = mono_object_get_class(obj);
	if(!klass) return nullptr;

	// 'entity' プロパティを取得 (MonoScriptに定義されている)
	MonoProperty* entityProp = mono_class_get_property_from_name(klass, "entity");
	MonoObject* entityObj = nullptr;
	if(entityProp) {
		entityObj = mono_property_get_value(entityProp, obj, nullptr, nullptr);
	} else {
		// プロパティがない場合はフィールドを探す (互換性のため)
		MonoClassField* entityField = mono_class_get_field_from_name(klass, "entity");
		if(entityField) {
			mono_field_get_value(obj, entityField, &entityObj);
		}
	}

	if(!entityObj) {
		// obj 自身が Entity クラスのインスタンスである可能性を考慮
		MonoClass* entityClass = mono_class_from_name(image_, "", "Entity");
		if(mono_class_is_assignable_from(entityClass, klass)) {
			entityObj = obj;
		}
	}

	if(!entityObj) return nullptr;

	// Entity オブジェクトから 'entityId_' フィールドを取得
	MonoClass* entityKlass = mono_object_get_class(entityObj);
	MonoClassField* idField = mono_class_get_field_from_name(entityKlass, "entityId_");
	if(!idField) {
		idField = MonoScriptEngineUtils::FindFieldRecursive(entityKlass, "entityId_");
	}

	if(!idField) return nullptr;

	int32_t entityId = 0;
	mono_field_get_value(entityObj, idField, &entityId);

	// 全グループから検索
	for(auto& pair : pEcs_->GetECSGroups()) {
		GameEntity* entity = pair.second->GetEntityCollection()->GetEntity(entityId);
		if(entity) return entity;
	}

	return nullptr;
}

GameEntity* MonoScriptEngine::GetOwnerEntity(const Guid& guid) {
	if(!pEcs_) return nullptr;

	for(auto& pair : pEcs_->GetECSGroups()) {
		GameEntity* entity = pair.second->GetEntityFromGuid(guid);
		if(entity) return entity;
	}

	return nullptr;
}

std::string MonoScriptEngine::GetGroupNameByEntityGuid(const Guid& guid) {
	if(!pEcs_) return "";

	for(auto& pair : pEcs_->GetECSGroups()) {
		if(pair.second->GetEntityFromGuid(guid)) {
			return pair.first;
		}
	}

	return "";
}

MonoMethod* MonoScriptEngine::GetMethodFromCS(const std::string& nameSpace, const std::string& className, const std::string& methodName, int argsCount) {
	/// MonoClassを取得
	MonoClass* monoClass = mono_class_from_name(image_, nameSpace.c_str(), className.c_str());
	if(!monoClass) {
		Console::LogError("Failed to find class: " + (nameSpace.empty() ? "" : nameSpace + ".") + className, LogCategory::ScriptEngine);
		return nullptr;
	}

	for(MonoClass* current = monoClass; current != nullptr; current = mono_class_get_parent(current)) {
		MonoMethod* method = mono_class_get_method_from_name(current, methodName.c_str(), argsCount);
		if(method) {
			return method;
		}
	}

	Console::LogError("Failed to find method: " + (nameSpace.empty() ? "" : nameSpace + ".") + className + "::" + methodName, LogCategory::ScriptEngine);
	return nullptr;
}

MonoDomain* MonoScriptEngine::CreateReloadDomain() {
	std::string domainName = "ReloadedDomain_" + std::to_string(++domainReloadCounter_);

	MonoDomain* domain = mono_domain_create_appdomain((char*)domainName.c_str(), nullptr);
	if(!domain) {
		Console::LogError("Failed to create Mono domain for hot reload: " + domainName, LogCategory::ScriptEngine);
		return nullptr;
	}

#if defined(DEBUG_MODE)
	// ホットリロード時の新しいドメイン用のデバッグ情報を登録
	mono_debug_domain_create(domain);
#endif

	return domain;
}

void MonoScriptEngine::ClearPendingDomains() {
#if defined(DEBUG_MODE)
	// デバッガが接続（アタッチ）されている間は、アンロードに伴うスレッド競合やデッドロッククラッシュを防ぐため、
	// ドメインのアンロードを一切行わず、リストに保留（蓄積）したままにします。
	// デバッガが切断された直後のフレームから安全に一括アンロードが実行されます。
	// （※毎フレームの GetExtendedTcpTable 呼び出しを避けるため、キャッシュされた変数 wasDebuggerAttached_ を使用します）
	if (wasDebuggerAttached_) {
		return;
	}
#endif

	if (domainsToUnload_.empty()) {
		return;
	}

	for(auto* domain : domainsToUnload_) {
		mono_domain_unload(domain);
	}
	domainsToUnload_.clear();
	pendingPdbBuffers_.clear();

	// 古いドメインに紐づいていたアセンブリ情報やメタデータをメモリから完全に解放するため、
	// ガベージコレクションを強制実行します。
	// これにより、再アタッチ時に古いアセンブリにブレイクポイントがバインドされる問題を防ぎます。
	mono_gc_collect(mono_gc_max_generation());
	Console::Log("[Mono] GC collected after domain unloading to clean up assembly remnants.", LogCategory::ScriptEngine);
}

MonoDomain* MonoScriptEngine::Domain() const {
	return domain_;
}

MonoImage* MonoScriptEngine::Image() const {
	return image_;
}

MonoAssembly* MonoScriptEngine::Assembly() const {
	return assembly_;
}

void MonoScriptEngine::SetIsHotReloadRequest(bool request) {
	isHotReloadRequest_ = request;
}

bool MonoScriptEngine::GetIsHotReloadRequest() const {
	return isHotReloadRequest_;
}

std::vector<MonoScriptEngine::NodeClassInfo> MonoScriptEngine::GetBehaviorNodeClasses() {
	std::vector<NodeClassInfo> nodeClasses;
	if(!image_) return nodeClasses;

	MonoClass* baseClass = mono_class_from_name(image_, "", "BehaviorNode");
	if(!baseClass) {
		Console::LogError("BehaviorNode class not found in C# assembly.", LogCategory::ScriptEngine);
		return nodeClasses;
	}

	MonoClass* decoratorAttrClass = mono_class_from_name(image_, "", "DecoratorAttribute");

	const MonoTableInfo* tableInfo = mono_image_get_table_info(image_, MONO_TABLE_TYPEDEF);
	int rows = mono_table_info_get_rows(tableInfo);

	for(int i = 0; i < rows; i++) {
		MonoClass* klass = mono_class_get(image_, (i + 1) | MONO_TOKEN_TYPE_DEF);
		if(!klass) continue;

		// 抽象クラスやインターフェースは除外
		uint32_t flags = mono_class_get_flags(klass);
		if(flags & (0x00000080 /* TYPE_ATTRIBUTE_ABSTRACT */ | 0x00000020 /* TYPE_ATTRIBUTE_INTERFACE */)) {
			continue;
		}

		if(mono_class_is_subclass_of(klass, baseClass, false)) {
			const char* className = mono_class_get_name(klass);
			const char* nameSpace = mono_class_get_namespace(klass);

			NodeClassInfo info;
			info.fullName = (nameSpace && strlen(nameSpace) > 0)
				? std::string(nameSpace) + "." + className
				: std::string(className);

			// Decorator属性のチェック
			if(decoratorAttrClass) {
				MonoCustomAttrInfo* attrs = mono_custom_attrs_from_class(klass);
				if(attrs) {
					if(mono_custom_attrs_has_attr(attrs, decoratorAttrClass)) {
						info.isDecorator = true;
					}
					mono_custom_attrs_free(attrs);
				}
			}

			nodeClasses.push_back(info);
		}
	}

	return nodeClasses;
}

std::vector<MonoScriptEngine::FieldInfo> MonoScriptEngine::GetClassFields(const std::string& className) {
	std::vector<FieldInfo> fields;
	if(!image_) return fields;

	MonoClass* klass = mono_class_from_name(image_, "", className.c_str());
	if(!klass) {
		// 名前空間ありの場合
		size_t dotPos = className.find_last_of('.');
		if(dotPos != std::string::npos) {
			std::string ns = className.substr(0, dotPos);
			std::string name = className.substr(dotPos + 1);
			klass = mono_class_from_name(image_, ns.c_str(), name.c_str());
		}
	}

	if(!klass) return fields;

	void* iter = nullptr;
	MonoClassField* field;
	while((field = mono_class_get_fields(klass, &iter))) {
		uint32_t flags = mono_field_get_flags(field);
		if(!(flags & 0x0006 /* FIELD_ATTRIBUTE_PUBLIC */)) continue;

		FieldInfo info;
		info.name = mono_field_get_name(field);

		MonoType* type = mono_field_get_type(field);
		char* typeName = mono_type_get_name(type);
		info.typeName = typeName;
		mono_free(typeName);

		// 属性のチェック (BlackboardKeyAttribute)
		MonoCustomAttrInfo* attrs = mono_custom_attrs_from_field(klass, field);
		if(attrs) {
			MonoClass* attrClass = mono_class_from_name(image_, "", "BlackboardKeyAttribute");
			if(attrClass && mono_custom_attrs_has_attr(attrs, attrClass)) {
				info.isBBKey = true;
			}
			mono_custom_attrs_free(attrs);
		}

		fields.push_back(info);
	}

	return fields;
}

std::vector<MonoScriptEngine::NodeClassInfo> MonoScriptEngine::GetBehaviorModuleClasses() {
	std::vector<NodeClassInfo> moduleClasses;
	if(!image_) return moduleClasses;

	MonoClass* decoratorBase = mono_class_from_name(image_, "", "BehaviorDecorator");
	MonoClass* serviceBase = mono_class_from_name(image_, "", "BehaviorService");

	const MonoTableInfo* tableInfo = mono_image_get_table_info(image_, MONO_TABLE_TYPEDEF);
	int rows = mono_table_info_get_rows(tableInfo);

	for(int i = 0; i < rows; i++) {
		MonoClass* klass = mono_class_get(image_, (i + 1) | MONO_TOKEN_TYPE_DEF);
		if(!klass) continue;

		uint32_t flags = mono_class_get_flags(klass);
		if(flags & (0x00000080 | 0x00000020)) continue;

		bool isDecorator = decoratorBase && mono_class_is_subclass_of(klass, decoratorBase, false);
		bool isService = serviceBase && mono_class_is_subclass_of(klass, serviceBase, false);

		if(isDecorator || isService) {
			const char* className = mono_class_get_name(klass);
			const char* nameSpace = mono_class_get_namespace(klass);

			NodeClassInfo info;
			info.fullName = (nameSpace && strlen(nameSpace) > 0)
				? std::string(nameSpace) + "." + className
				: std::string(className);
			info.isDecorator = isDecorator; // true: Decorator, false: Service
			moduleClasses.push_back(info);
		}
	}
	return moduleClasses;
}

void MonoScriptEngine::UpdateAiIntents(void* data, int count, float deltaTime, const std::string& groupName) {
	if(!updateAiIntentsMethod_) {
		Console::LogWarning("AIUpdater.UpdateIntents method not found in C#.", LogCategory::ScriptEngine);
		return;
	}

	void* args[4];
	args[0] = data;
	args[1] = &count;
	args[2] = &deltaTime;
	args[3] = mono_string_new(domain_, groupName.c_str());

	MonoObject* exc = nullptr;
	MonoScriptEngineUtils::SafeInvoke(updateAiIntentsMethod_, nullptr, args, &exc);

	if(exc) {
		MonoScriptEngineUtils::HandleException(exc);
	}
}

void MonoScriptEngine::NotifyEventCompleted(int32_t entityId, const std::string& eventName) {
	if(!notifyEventCompletedMethod_) {
		return;
	}

	std::string key = "EventComplete_" + eventName;
	MonoString* keyStr = mono_string_new(domain_, key.c_str());
	bool value = true;

	void* args[3];
	args[0] = &entityId;
	args[1] = keyStr;
	args[2] = &value;

	MonoObject* exc = nullptr;
	MonoScriptEngineUtils::SafeInvoke(notifyEventCompletedMethod_, nullptr, args, &exc);

	if(exc) {
		MonoScriptEngineUtils::HandleException(exc);
	}
}

void MonoScriptEngine::ClearECSGroup(const std::string& name) {
	if(!clearEcsGroupMethod_) {
		return;
	}

	void* args[1];
	args[0] = mono_string_new(domain_, name.c_str());

	MonoObject* exc = nullptr;
	MonoScriptEngineUtils::SafeInvoke(clearEcsGroupMethod_, nullptr, args, &exc);

	if(exc) {
		MonoScriptEngineUtils::HandleException(exc);
	}
}

void MonoScriptEngine::SyncInitialComponentsToCS(ECSGroup* ecsGroup) {
	if(!ecsGroup) {
		return;
	}

	const std::string& ecsGroupName = ecsGroup->GetGroupName();

	if(!addEcsGroupMethod_ || !getComponentCollectionField_ || !receiveAllBatchesMethod_) {
		Console::LogError("One or more methods for SyncInitialComponentsToCS are not found.", LogCategory::ScriptEngine);
		return;
	}

	MonoObject* exc = nullptr;

	// C#側のシーン名を更新
	if(sceneNameField_) {
		MonoString* nameStr = mono_string_new(domain_, ecsGroupName.c_str());
		MonoClass* parentClass = mono_field_get_parent(sceneNameField_);
		MonoVTable* vtable = mono_class_vtable(domain_, parentClass);
		mono_field_static_set_value(vtable, sceneNameField_, nameStr);
	}

	void* getGroupArgs[1];
	getGroupArgs[0] = mono_string_new(domain_, ecsGroupName.c_str());
	MonoObject* ecsGroupObject = MonoScriptEngineUtils::SafeInvoke(addEcsGroupMethod_, nullptr, getGroupArgs, &exc);
	if(exc) {
		MonoScriptEngineUtils::HandleException(exc);
		return;
	}
	if(!ecsGroupObject) {
		Console::LogError("C# ECSGroup object is null for group: " + ecsGroupName, LogCategory::ScriptEngine);
		return;
	}

	if(addEntityMethod_) {
		for(const auto& entity : ecsGroup->GetEntities()) {
			int32_t id = entity->GetId();
			void* addArgs[1];
			addArgs[0] = &id;
			MonoScriptEngineUtils::SafeInvoke(addEntityMethod_, ecsGroupObject, addArgs, &exc);
			if(exc) {
				MonoScriptEngineUtils::HandleException(exc);
				exc = nullptr;
				continue;
			}
		}
	}

	MonoObject* collectionObject = mono_field_get_value_object(domain_, getComponentCollectionField_, ecsGroupObject);
	if(!collectionObject) {
		Console::LogError("C# ComponentCollection object is null for group: " + ecsGroupName, LogCategory::ScriptEngine);
		return;
	}

	exc = nullptr;
	void* receiveArgs[2];
	receiveArgs[0] = collectionObject;
	receiveArgs[1] = mono_string_new(domain_, ecsGroupName.c_str());
	MonoScriptEngineUtils::SafeInvoke(receiveAllBatchesMethod_, nullptr, receiveArgs, &exc);
	if(exc) {
		MonoScriptEngineUtils::HandleException(exc);
		return;
	}

	Console::Log("Successfully synced initial components to C# for group: " + ecsGroupName, LogCategory::ScriptEngine);
}

MonoMethod* MonoScriptEngineUtils::FindMethodInClassOrParents(MonoClass* monoClass, const char* methodName, int paramCount) {
	while(monoClass) {
		MonoMethod* method = mono_class_get_method_from_name(monoClass, methodName, paramCount);
		if(method)
			return method;
		monoClass = mono_class_get_parent(monoClass);
	}
	return nullptr;
}

MonoClassField* ONEngine::MonoScriptEngineUtils::FindFieldRecursive(MonoClass* monoClass, const char* name) {
	while(monoClass) {
		MonoClassField* field = mono_class_get_field_from_name(monoClass, name);
		if(field) {
			return field;
		}
		monoClass = mono_class_get_parent(monoClass);
	}
	return nullptr;
}

void ONEngine::MonoScriptEngineUtils::HandleException(MonoObject* exc) {
	if(!exc) return;

	MonoClass* excClass = mono_object_get_class(exc);
	MonoMethod* toStringMethod = mono_class_get_method_from_name(excClass, "ToString", 0);

	MonoObject* excStr = MonoScriptEngineUtils::SafeInvoke(toStringMethod, exc, nullptr, nullptr);
	char* err = mono_string_to_utf8((MonoString*)excStr);
	if(err) {


		Console::LogError("----------------------------------------------------------------", LogCategory::ScriptEngine);
		Console::LogError("[C# Exception] An unhandled exception occurred in the scripting engine:", LogCategory::ScriptEngine);
		Console::LogError(err, LogCategory::ScriptEngine);
		Console::LogError("----------------------------------------------------------------", LogCategory::ScriptEngine);

		mono_free(err);
	}
}

MonoObject* ONEngine::MonoScriptEngineUtils::SafeInvoke(MonoMethod* method, void* obj, void** params, MonoObject** outExc) {
	if(!method) {
		return nullptr;
	}

	// 現在のスレッドを確実に Mono にアタッチする (デバッガとGCの正常動作のため)
	mono_thread_attach(MonoScriptEngine::GetInstance().Domain());

	try {
		return mono_runtime_invoke(method, obj, params, outExc);
	} catch(const std::exception& e) {
		Console::LogError(std::string("[C++ Exception in Mono Invoke] ") + e.what(), LogCategory::ScriptEngine);
	} catch(...) {
		Console::LogError("[C++ Unknown Exception in Mono Invoke] A critical error occurred inside the Mono runtime.", LogCategory::ScriptEngine);
	}

	return nullptr;
}

bool MonoScriptEngine::BuildCSharpProject(std::string& outMessage) {
	std::wstring config = L"Debug";
#if !defined(DEBUG_MODE)
	config = L"Release";
#endif

	// dotnet.exeの絶対パスを探索
	std::wstring dotnetPath = L"dotnet";
	if (std::filesystem::exists(L"C:\\Program Files\\dotnet\\dotnet.exe")) {
		dotnetPath = L"\"C:\\Program Files\\dotnet\\dotnet.exe\"";
	} else if (std::filesystem::exists(L"C:\\Program Files (x86)\\dotnet\\dotnet.exe")) {
		dotnetPath = L"\"C:\\Program Files (x86)\\dotnet\\dotnet.exe\"";
	}

	std::wstring cmd = dotnetPath + L" build \"../SubProjects/CSharpLibrary/CSharpLibrary.csproj\" -c " + config + L" -p:Platform=x64";
	std::wstring currentPathW = std::filesystem::current_path().wstring();

	HANDLE hReadPipe, hWritePipe;
	SECURITY_ATTRIBUTES saAttr;
	saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
	saAttr.bInheritHandle = TRUE;
	saAttr.lpSecurityDescriptor = NULL;

	if (!CreatePipe(&hReadPipe, &hWritePipe, &saAttr, 0)) {
		outMessage = "Failed to create pipe for dotnet build.";
		return false;
	}

	if (!SetHandleInformation(hReadPipe, HANDLE_FLAG_INHERIT, 0)) {
		outMessage = "Failed to set handle information.";
		CloseHandle(hReadPipe);
		CloseHandle(hWritePipe);
		return false;
	}

	STARTUPINFOW si;
	PROCESS_INFORMATION pi;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	si.hStdError = hWritePipe;
	si.hStdOutput = hWritePipe;
	si.dwFlags |= STARTF_USESTDHANDLES;

	ZeroMemory(&pi, sizeof(pi));

	std::vector<wchar_t> cmdBuffer(cmd.begin(), cmd.end());
	cmdBuffer.push_back(L'\0');

	BOOL success = CreateProcessW(
		NULL,
		cmdBuffer.data(),
		NULL,
		NULL,
		TRUE,
		CREATE_NO_WINDOW,
		NULL,
		currentPathW.c_str(), // 作業ディレクトリを明示的に指定
		&si,
		&pi
	);

	CloseHandle(hWritePipe);

	if (!success) {
		DWORD err = GetLastError();
		outMessage = "dotnet command not found or failed to execute. Error Code: " + std::to_string(err) + ". Please ensure .NET SDK is installed.";
		CloseHandle(hReadPipe);
		return false;
	}

	std::string output;
	char buffer[4096];
	DWORD bytesRead;
	while (ReadFile(hReadPipe, buffer, sizeof(buffer) - 1, &bytesRead, NULL) && bytesRead > 0) {
		buffer[bytesRead] = '\0';
		output += buffer;
	}

	WaitForSingleObject(pi.hProcess, INFINITE);

	DWORD exitCode = 0;
	GetExitCodeProcess(pi.hProcess, &exitCode);

	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
	CloseHandle(hReadPipe);

	outMessage = output;
	return (exitCode == 0);
}

void MonoScriptEngine::UpdateDebuggerStatus() {
#if defined(DEBUG_MODE)
	if (!domain_) return;
	MonoThread* monoThread = mono_thread_attach(domain_);

	bool currentAttached = IsDebuggerAttachedViaTcp();

	// 接続状態に変化があった場合、詳細なステータスログを出す
	static bool lastCurrentAttached = false;
	static bool lastWasDebuggerAttached = false;
	if (currentAttached != lastCurrentAttached || wasDebuggerAttached_ != lastWasDebuggerAttached) {
		Console::Log("[MonoDbg] State Change -> currentAttached: " + std::string(currentAttached ? "TRUE" : "FALSE") + 
			", wasDebuggerAttached_: " + std::string(wasDebuggerAttached_ ? "TRUE" : "FALSE") + 
			", monoThread: " + std::string(monoThread ? "YES" : "NO") + 
			", isGameplayRunning: " + std::string(DebugConfig::isDebugging ? "YES" : "NO"),
			LogCategory::ScriptEngine);
		
		// TCP接続の詳細情報を出力
		LogTcpConnections();

		lastCurrentAttached = currentAttached;
		lastWasDebuggerAttached = wasDebuggerAttached_;
	}

	if (currentAttached && !wasDebuggerAttached_) {
		Console::Log("[Mono] Debugger newly attached! Syncing breakpoints...", LogCategory::ScriptEngine);
		showAttachedPopup_ = true;

		// Mono ランタイムにデバッガの物理接続を同期
		mono_set_is_debugger_attached(true);

		// CSharpLibrary.dll / PDB のタイムスタンプを詳細出力
		std::string dllPath = "./Packages/Scripts/CSharpLibrary.dll";
		std::string pdbPath = "./Packages/Scripts/CSharpLibrary.pdb";
		if (std::filesystem::exists(dllPath)) {
			auto ftime = std::filesystem::last_write_time(dllPath);
			auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(ftime - std::filesystem::file_time_type::clock::now() + std::chrono::system_clock::now());
			std::time_t ctime = std::chrono::system_clock::to_time_t(sctp);
			char timeBuf[100];
			std::tm timeInfo;
			localtime_s(&timeInfo, &ctime);
			std::strftime(timeBuf, sizeof(timeBuf), "%Y-%m-%d %H:%M:%S", &timeInfo);
			Console::Log("[MonoDbg] CSharpLibrary.dll last modified: " + std::string(timeBuf), LogCategory::ScriptEngine);
		}
		if (std::filesystem::exists(pdbPath)) {
			auto ftime = std::filesystem::last_write_time(pdbPath);
			auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(ftime - std::filesystem::file_time_type::clock::now() + std::chrono::system_clock::now());
			std::time_t ctime = std::chrono::system_clock::to_time_t(sctp);
			char timeBuf[100];
			std::tm timeInfo;
			localtime_s(&timeInfo, &ctime);
			std::strftime(timeBuf, sizeof(timeBuf), "%Y-%m-%d %H:%M:%S", &timeInfo);
			Console::Log("[MonoDbg] CSharpLibrary.pdb last modified: " + std::string(timeBuf), LogCategory::ScriptEngine);
		}

		// デバッガが安全にブレイクポイントをバインド（同期）できるように、
		// アタッチ直後に即座にポーズするのではなく、30フレームだけマネージドコードを通常実行させ、
		// その後一時的にゲームの実行をポーズ（一時停止）状態にします。
		debuggerAttachFrameCounter_ = 30;
		isDebuggerSyncSuccess_ = false; // Sync Skip ポップアップを出し、ポーズ状態の解除を促す
		Console::Log("[Mono] Debugger newly attached! Delaying game suspension for debugger synchronization (30 frames)...", LogCategory::ScriptEngine);
	}

	// アタッチ時の同期猶予フレームカウント処理
	if (debuggerAttachFrameCounter_ > 0) {
		debuggerAttachFrameCounter_--;
		if (debuggerAttachFrameCounter_ == 0) {
			DebugConfig::isPause = true;
			Console::Log("[Mono] Debugger synchronization delay finished. Game execution suspended.", LogCategory::ScriptEngine);
		}
	}

	// デバッガ切断時の状態同期処理
	if (!currentAttached && wasDebuggerAttached_) {
		Console::Log("[Mono] Debugger disconnected. Clearing attached state in Mono.", LogCategory::ScriptEngine);
		mono_set_is_debugger_attached(false);
	}

	wasDebuggerAttached_ = currentAttached;
#endif
}



