#pragma once

/// std
#include <string>
#include <optional>
#include <vector>

/// externals
#include <jit/jit.h>
#include <metadata/assembly.h>
#include <metadata/mono-debug.h>
#include <metadata/debug-helpers.h>
#include <utils/mono-logger.h>

/// engine
#include "Engine/ECS/Component/Components/ComputeComponents/Script/Script.h"


/// ///////////////////////////////////////////////////
/// monoを使ったC#スクリプトエンジン
/// ///////////////////////////////////////////////////
namespace ONEngine {

class ECSGroup;

class MonoScriptEngine {
private:
	/// ===================================================
	/// private : methods
	/// ===================================================

	MonoScriptEngine();
	~MonoScriptEngine();

	/// C#プロジェクトのビルド
	bool BuildCSharpProject(std::string& outMessage);

	/// 代入演算子の禁止
	MonoScriptEngine(const MonoScriptEngine&) = delete;
	MonoScriptEngine& operator=(const MonoScriptEngine&) = delete;
	MonoScriptEngine(MonoScriptEngine&&) = delete;
	MonoScriptEngine& operator=(MonoScriptEngine&&) = delete;

public:
	/// ===================================================
	/// public : methods
	/// ===================================================

	/// インスタンスの取得
	static MonoScriptEngine& GetInstance();

	/// Monoの初期化
	void Initialize();

	/// @brief monoの終了処理
	void Finalize();

	/// CSの関数を登録
	void RegisterFunctions();

	/// CSのHotReloadを行う
	void HotReload();

	/// ドメインの再ロードバージョンカウンタを取得
	int32_t GetDomainReloadCounter() const { return domainReloadCounter_; }

	/// ホットリロード実行中（コピー中など）フラグを取得
	bool IsReloading() const { return isReloading_; }
	void SetIsReloading(bool reloading) { isReloading_ = reloading; }

	/// C#のログ無視設定を同期・適用
	void ApplyCSharpLogSetting();

	void SetEcsPtr(class EntityComponentSystem* ecs);

	/// DLLのパスを探す
	std::optional<std::string> FindLatestDll(const std::string& dirPath, const std::string& baseName);

	/// C#側のリセット
	void ResetCS();

	/// @brief C#側の特定のECSGroupをクリアする
	void ClearECSGroup(const std::string& name);

	/// @brief C++で初期化したコンポーネントデータをCS側に同期する
	void SyncInitialComponentsToCS(ECSGroup* ecsGroup);

	/// C#側のECSGroupインスタンスを取得
	MonoObject* GetEcsGroupObject(const std::string& groupName);

	/// C#側のEntityを取得
	MonoObject* GetEntityFromCS(const std::string& ecsGroupName, int32_t entityId);
	MonoObject* GetMonoBehaviorFromCS(const std::string& ecsGroupName, int32_t entityId, const std::string& behaviorName);

	/// @brief MonoObjectの所有者であるGameEntityを取得する
	/// @param obj 
	/// @return 
	class GameEntity* GetOwnerEntity(MonoObject* obj);

	/// @brief GuidからGameEntityを取得する
	/// @param guid 
	/// @return 
	class GameEntity* GetOwnerEntity(const struct Guid& guid);

	/// @brief エンティティのGuidから所属しているECSグループ名を取得する
	/// @param guid 
	/// @return 
	std::string GetGroupNameByEntityGuid(const struct Guid& guid);

	/// @brief C#側のメソッドを取得する
	/// @param namespace 名前空間
	/// @param className クラス名
	/// @param methodName 関数名
	/// @param argsCount 引数の数
	/// @return 関数へのポインタ
	MonoMethod* GetMethodFromCS(const std::string& nameSpace, const std::string& className, const std::string& methodName, int argsCount);

	/// @brief Reload用のDomainを作成する
	/// @return 作成したDomainへのポインタ
	MonoDomain* CreateReloadDomain();
	
	/// @brief 保留中の古いDomainをアンロードする
	void ClearPendingDomains();
	
	void UpdateAiIntents(void* data, int count, float deltaTime, const std::string& groupName);

	/// @brief C#のBlackboardManagerにイベント完了を通知する
	/// @param entityId 対象のエンティティID
	/// @param eventName 完了したイベント名
	void NotifyEventCompleted(int32_t entityId, const std::string& eventName);

	struct NodeClassInfo {
		std::string fullName;
		bool isDecorator = false;
	};
	/// @brief BehaviorNodeを継承する全クラス情報を取得する
	std::vector<NodeClassInfo> GetBehaviorNodeClasses();

	/// @brief BehaviorDecorator/Serviceを継承する全クラス情報を取得する
	std::vector<NodeClassInfo> GetBehaviorModuleClasses();

	struct FieldInfo {
		std::string name;
		std::string typeName;
		bool isBBKey = false;
	};
	/// @brief 指定したクラスの公開フィールド情報を取得する
	std::vector<FieldInfo> GetClassFields(const std::string& className);

private:
	/// ===================================================
	/// private : objects
	/// ===================================================

	std::string currentDllPath_;

	MonoDomain* rootDomain_ = nullptr;
	MonoDomain* domain_ = nullptr;
	MonoImage* image_;
	MonoAssembly* assembly_ = nullptr;
	std::vector<MonoDomain*> domainsToUnload_;

	bool isHotReloadRequest_;
	int32_t domainReloadCounter_; /// domainのリロード回数

	/// C#側のメソッドポインタ
	MonoMethod* receiveAllBatchesMethod_ = nullptr;
	MonoMethod* getEcsGroupMethod_ = nullptr;
	MonoMethod* addEcsGroupMethod_ = nullptr;
	MonoMethod* clearEcsGroupMethod_ = nullptr;
	MonoMethod* addEntityMethod_ = nullptr;
	MonoMethod* fetchInitialDataMethod_ = nullptr;
	MonoClassField* getComponentCollectionField_ = nullptr;
	MonoMethod* updateAiIntentsMethod_ = nullptr;
	MonoMethod* notifyEventCompletedMethod_ = nullptr;

	/// SceneManager
	MonoClassField* sceneNameField_ = nullptr;

	EntityComponentSystem* pEcs_ = nullptr;

public:
	/// ===================================================
	/// public : accessors
	/// ===================================================

	MonoDomain* Domain() const;
	MonoImage* Image() const;
	MonoAssembly* Assembly() const;

	void SetIsHotReloadRequest(bool request);
	bool GetIsHotReloadRequest() const;

	/// デバッガ接続状態の監視と自動リロード
	void UpdateDebuggerStatus();

	bool GetShowAttachedPopup() const { return showAttachedPopup_; }
	bool IsDebuggerSyncSuccess() const { return isDebuggerSyncSuccess_; }
	void ClearShowAttachedPopup() { showAttachedPopup_ = false; }

private:
	std::vector<char> activePdbBuffer_;
	std::vector<std::vector<char>> pendingPdbBuffers_;
	bool wasDebuggerAttached_ = false;
	bool showAttachedPopup_ = false;
	bool isDebuggerSyncSuccess_ = false;
	int debuggerAttachFrameCounter_ = 0;
	bool isReloading_ = false;

};


namespace MonoScriptEngineUtils {
	MonoMethod* FindMethodInClassOrParents(MonoClass* monoClass, const char* methodName, int paramCount);
	MonoClassField* FindFieldRecursive(MonoClass* monoClass, const char* name);
	
	/// @brief Monoの例外を処理し、ログに出力する
	/// @param exc 例外オブジェクト
	void HandleException(MonoObject* exc);

	/// @brief 安全にC#のメソッドを呼び出す（C++レベルの例外も捕捉する）
	/// @param method メソッドポインタ
	/// @param obj インスタンス（staticならnullptr）
	/// @param params 引数配列
	/// @param outExc 発生した例外を受け取るポインタ
	/// @return 実行結果（エラー時はnullptr）
	MonoObject* SafeInvoke(MonoMethod* method, void* obj, void** params, MonoObject** outExc);
} // namespace MonoScriptEngineUtils

} /// ONEngine
