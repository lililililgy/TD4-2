using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Runtime.CompilerServices;

public class ECSGroup {
	///////////////////////////////////////////////////////////////////////////////////////////
	// objects
	///////////////////////////////////////////////////////////////////////////////////////////

	public string groupName;
	private bool enable_; //!< このGroupの有効/無効フラグ
	private Dictionary<int, Entity> entities_ = new Dictionary<int, Entity>();
	public ComponentCollection componentCollection = new ComponentCollection();

	/// 生成処理、初期化処理の呼び出し用リスト
	private List<Entity> awakeList_ = new List<Entity>();
	private List<Entity> initList_ = new List<Entity>();


	///////////////////////////////////////////////////////////////////////////////////////////
	// methods
	///////////////////////////////////////////////////////////////////////////////////////////

	/// <summary>
	/// コンストラクタ
	/// </summary>
	/// <param name="groupName"></param>
	public ECSGroup(string groupName) {
		ComponentBatchManager.Initialize();
		this.groupName = groupName;
		enable_ = true;
	}

	// ==============================================
	// エンティティの生成
	// ==============================================

	/// <summary>
	/// c/c++側から呼び出すエンティティの追加関数
	/// </summary>
	public void AddEntity(int id) {
		//Debug.LogInfo("ECSGroup.AddEntity - Adding entity with ID: " + id + ", Group Name: " + groupName);
		if(entities_.ContainsKey(id)) {
			//Debug.LogError("ECSGroup.AddEntity - Entity already exists with ID: " + id + ", Group Name: " + groupName);
			return;
		}


		Entity entity = new Entity(id, this);
		entities_.Add(id, entity);

		/// 生成、初期化の呼び出し用リストに追加
		awakeList_.Add(entity);
		initList_.Add(entity);
	}

	/// <summary>
	/// C/C++側から呼び出すコンポーネントの追加関数
	/// </summary>
	public void AddScript(int entityId, MonoScript behavior, bool enable) {
		Entity entity;
		if (entities_.TryGetValue(entityId, out entity)) {
			Debug.LogInfo("ECSGroup.AddScript - Adding script to Entity ID: " + entityId + ", Script Name: " + behavior.GetType().Name);
			behavior.CreateBehavior(entityId, behavior.GetType().Name, this);
			behavior.enable = enable;
			entity.AddScript(behavior);
		} else {
			Debug.LogError("ECSGroup.AddScript - Entity not found with ID: " + entityId);
		}
	}

	/// <summary>
	/// c#側から呼び出すエンティティの生成関数
	/// </summary>
	public Entity CreateEntity(string prefabName) {
		//Debug.LogInfo("ECSGroup.CreateEntity - Creating entity with prefab: " + prefabName + ", Group Name: " + groupName);

		int id = 0;
		InternalCreateEntity(out id, prefabName, groupName);
		Entity entity = new Entity(id, this);
		entities_.Add(id, entity);

		awakeList_.Add(entity); //!< 生成されたエンティティを生成リストに追加
		initList_.Add(entity); //!< 初期化リストにも追加
		//Debug.Log("ECSGroup.CreateEntity - AwakeListCount: " + awakeList_.Count + ", InitListCount: " + initList_.Count);

		return entity;
	}

	// ==============================================
	// 更新
	// ==============================================

	/// <summary>
	/// リストないのエンティティの更新処理を呼ぶ
	/// </summary>
	public void UpdateEntities() {
		//!< 無効なら処理しない
		if (!enable_) {
			return;
		}

		var sw = Stopwatch.StartNew();

		ComponentBatchManager.ReceiveAllBatches(componentCollection, groupName);

		/// 生成、初期化の呼び出しを行う
		CallAwake();
		CallInitialize();

		// 初期化後の座標を即座にC++に送信してリセットを防ぐ
		ComponentBatchManager.SendAllBatches(componentCollection, groupName);

		// 生成・削除によるコレクション変更エラーを避けるため、配列にコピーして反復処理を行う
		foreach (Entity entity in entities_.Values.ToArray()) {
			// 処理中に自身または他のエンティティが削除された場合はスキップ
			if (!entities_.ContainsKey(entity.Id)) {
				continue;
			}

			if (!CheckEnable(entity)) {
				continue;
			}

			foreach (MonoScript script in entity.GetScripts()) {
				if (script.enable) {
					script.Update();
				}
			}
		}

		ComponentBatchManager.SendAllBatches(componentCollection, groupName);

		//Debug.Log("//////////////////////////////////////////////////////////////////////////////////////////////////");
		//Debug.Log("ECSGroup.UpdateEntities - Updating entities in group: " + groupName + ", EntityCount: " + entities_.Count);
		//Debug.Log($"gen0:{GC.CollectionCount(0)} gen1:{GC.CollectionCount(1)} gen2:{GC.CollectionCount(2)}");
		//sw.Stop();
		//double ms = sw.ElapsedTicks * 1000.0 / Stopwatch.Frequency;
		//Debug.Log("Update Time (ms): " + ms);
		//Debug.Log("//////////////////////////////////////////////////////////////////////////////////////////////////");

	}


	/// <summary>
	/// 生成されたエンティティの生成関数の呼び出しを行う
	/// </summary>
	private void CallAwake() {
		if (awakeList_.Count == 0) {
			return;
		}

#if DEBUG
		//Debug.Log("");
		//Debug.Log("//////////////////////////////////////////////////////////////////////////////////////////////////");
		//Debug.Log("ECSGroup.CallAwake - Awakening entities in group: " + groupName + ", Count: " + awakeList_.Count);
#endif

		List<Entity> entitiesToAwake = new List<Entity>(awakeList_);
		awakeList_.Clear(); // 生成リストをクリア
		foreach (Entity entity in entitiesToAwake) {
			foreach (MonoScript script in entity.GetScripts()) {
				script.Awake();
			}
		}

#if DEBUG
		//Debug.Log("//////////////////////////////////////////////////////////////////////////////////////////////////");
		//Debug.Log("");
#endif
	}


	/// <summary>
	/// 生成されたエンティティの初期化関数の呼び出しを行う
	/// </summary>
	private void CallInitialize() {
		if (initList_.Count == 0) {
			return;
		}

#if DEBUG
		Debug.Log("");
		Debug.Log("//////////////////////////////////////////////////////////////////////////////////////////////////");
		Debug.Log("ECSGroup.CallInitialize - Initializing entities in group: " + groupName + ", Count: "
				  + initList_.Count);
#endif

		List<Entity> entitiesToInitialize = new List<Entity>(initList_);
		initList_.Clear();
		foreach (Entity entity in entitiesToInitialize) {
			foreach (MonoScript script in entity.GetScripts()) {
				script.Initialize();
			}
		}

#if DEBUG
		Debug.Log("//////////////////////////////////////////////////////////////////////////////////////////////////");
		Debug.Log("");
#endif
	}

	/// <summary>
	/// Entityの取得
	/// </summary>
	public Entity GetEntity(int id) {
		if (entities_.TryGetValue(id, out Entity entity)) {
#if DEBUG
			//Debug.Log("ECSGroup.GetEntity - Entity found with ID: " + entity.Id + ", Entity Name: " + entity.name);
#endif
			return entity;
		}

#if DEBUG
		//Debug.LogError("ECSGroup.GetEntity - Entity not found with ID: " + id + ", Group Name: " + groupName);
#endif
		return null;
	}

	/// <summary>
	/// エンティティの削除
	/// </summary>
	public void DestroyEntity(int id) {
		if (entities_.TryGetValue(id, out Entity entity)) {
			entities_.Remove(id);
			InternalDestroyEntity(groupName, id);
#if DEBUG
			Debug.Log("Entity destroyed with ID: " + id);
		} else {
			Debug.LogError("Entity not found with ID: " + id);
#endif
		}
	}

	/// <summary>
	/// すべてのエンティティを削除
	/// </summary>
	public void DeleteEntityAll() {
#if DEBUG
		Debug.Log("ECSGroup.DeleteEntityAll - Deleting all entities in group: " + groupName + ", EntityCount: "
				  + entities_.Count);
#endif

		var entitiesToDestroy = new Dictionary<int, Entity>(entities_);
		foreach (var entity in entitiesToDestroy.Values) {
			entity.Destroy();
		}
	}

	/// <summary>
	/// C++側がシーンを破棄する際に呼ぶ、C#側の保持を捨てるための関数。
	///
	/// C++の EntityCollection::RemoveEntity() はC#へ破棄を通知しないため、これを呼ばないと
	/// entities_ に旧シーンのエンティティが残る。エンティティIDは使い回されるので、
	/// 再読み込みで同じIDが割り当てられると AddEntity() が「既に居る」と判断して早期returnし、
	/// 旧シーンのMonoScriptインスタンスが状態を保ったまま再利用され、Initialize()も呼ばれなくなる。
	///
	/// Entity.Destroy() は使ってはいけない。InternalDestroyEntity でC++へ呼び返してしまい、
	/// 破棄処理中のネイティブ側へ再入するため。ここではC#側の後始末だけを行う。
	/// </summary>
	public void ClearEntitiesFromNative() {
#if DEBUG
		Debug.Log("ECSGroup.ClearEntitiesFromNative - Clearing group: " + groupName + ", EntityCount: "
				  + entities_.Count);
#endif

		/// MessageBus の購読解除などを行わせる。
		/// 1つが例外を投げても残りの後始末は続ける（取りこぼすと購読が残り続けるため）。
		foreach (Entity entity in entities_.Values.ToArray()) {
			foreach (MonoScript script in entity.GetScripts()) {
				try {
					script.OnDestroy();
				} catch (Exception e) {
					Debug.LogError("ECSGroup.ClearEntitiesFromNative - OnDestroy threw: " + e.Message);
				}
			}
		}

		entities_.Clear();
		awakeList_.Clear();
		initList_.Clear();

		/// 古いComponentが残るとバッチ同期に載り続けるので一緒に捨てる
		componentCollection.Clear();
	}

	/// <summary>
	/// エンティティの探索
	/// </summary>
	public Entity FindEntity(string name) {
		foreach (var entity in entities_.Values) {
			if (entity.name == name) {
				return entity;
			}
		}

#if DEBUG
		Debug.LogError("Entity not found with name: " + name);
#endif
		return null;
	}


	bool CheckEnable(Entity entity) {
		if (!entity) {
			return false;
		}

		if (!entity.enable) {
			return false;
		}

		Entity parent = entity.parent;
		if (parent) {
			return CheckEnable(parent);
		}

		return true;
	}


	///////////////////////////////////////////////////////////////////////////////////////////
	// internal methods
	///////////////////////////////////////////////////////////////////////////////////////////

	[MethodImpl(MethodImplOptions.InternalCall)]
	static extern void InternalCreateEntity(out int id, string prefabName, string groupName);

	[MethodImpl(MethodImplOptions.InternalCall)]
	static extern void InternalDestroyEntity(string ecsGroupName, int id);

	[MethodImpl(MethodImplOptions.InternalCall)]
	static extern void InternalSetEnable();

	[MethodImpl(MethodImplOptions.InternalCall)]
	static extern void InternalFindEntity(string ecsGroupName, string entityName);


	///////////////////////////////////////////////////////////////////////////////////////////
	// operators
	///////////////////////////////////////////////////////////////////////////////////////////

	public static implicit operator bool(ECSGroup group) {
		return group != null;
	}
}