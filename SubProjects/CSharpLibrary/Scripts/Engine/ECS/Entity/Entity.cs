using System;
using System.Collections.Generic;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Security.Principal;

public class Entity {

	/// =========================================
	/// objects
	/// =========================================

	// transform
	public Transform transform;

	// components, scripts
	private Dictionary<string, Component> components_ = new Dictionary<string, Component>();
	private Dictionary<string, MonoScript> scripts_ = new Dictionary<string, MonoScript>();

	// id
	private int entityId_;
	private int parentId_ = 0; // 親のID

	// ecs group name
	private string ecsGroupName_;
	private ECSGroup ecsGroup_;

	/// =========================================
	/// methods
	/// =========================================

	/// <summary>
	/// コンストラクタ
	/// </summary>
	public Entity(int id, ECSGroup ecsGroup) {
		entityId_ = id;
		ecsGroup_ = ecsGroup;
		ecsGroupName_ = ecsGroup_.groupName;
		transform = AddComponent<Transform>();
	}

	public int Id {
		get {
			return entityId_;
		}
	}

	public ECSGroup ecsGroup {
		get {
			return ecsGroup_;
		}
	}

	public string ecsGroupName {
		get {
			return ecsGroupName_;
		}
	}

	public string name {
		get {
			IntPtr namePtr = InternalGetName(entityId_, ecsGroupName_);
			if (namePtr == IntPtr.Zero) {
				// Debug.Log("[error] Entity name is null for ID: " + entityId_);
				return "UnnamedEntity";
			}
			string name = Marshal.PtrToStringAnsi(namePtr);
			return name;
		}
		set {
			InternalSetName(entityId_, value, ecsGroupName_);
		}
	}

	public Entity parent {
		get {
			int parentId = InternalGetParentId(entityId_, ecsGroupName_);
			Entity parentEntity = ecsGroup_.GetEntity(parentId);
			if (parentEntity) {
				return parentEntity;
			}

			return null;
		}
		set {
			if (value == null) {
				return;
			}
			InternalSetParent(Id, value.Id, ecsGroupName_);
		}
	}

	public bool enable {
		get {
			return InternalGetEnable(entityId_, ecsGroupName_);
		}
		set {
			InternalSetEnable(entityId_, value, ecsGroupName_);
		}
	}


	/// =========================================
	/// methods
	/// =========================================

	public Entity GetChild(uint index) {
		int childId = InternalGetChildId(entityId_, index, ecsGroupName_);
		ECSGroup ecsGroup = EntityComponentSystem.GetECSGroup(ecsGroupName_);
		if (ecsGroup == null) {
			return null;
		}

		return ecsGroup.GetEntity(childId);
	}

	public uint GetChildCount() {
		return (uint)InternalGetChildrenCount(entityId_, ecsGroupName_);
	}


	public void Destroy() {
		/// 子の情報もクリア
		uint childCount = GetChildCount();
		List<Entity> children = new List<Entity>();
		for (uint i = 0; i < childCount; i++) {
			Entity child = GetChild(i);
			if (child) {
				children.Add(child);
			}
		}
		foreach (Entity child in children) {
			Debug.LogInfo("Entity.Destroy - Destroying child entity ID: " + child.Id + " of parent entity ID: " + entityId_);
			child.Destroy();
		}

		/// Entityを削除
		ecsGroup_.DestroyEntity(entityId_);
		entityId_ = 0; // IDを無効化
		transform = null;

		foreach (var comp in components_) {
			ecsGroup_.componentCollection.RemoveComponent(comp.Value);
		}

		components_.Clear();
		scripts_.Clear();
	}


	/// ------------------------------------------
	/// components
	/// ------------------------------------------

	public T AddComponent<T>() where T : Component {
		/// コンポーネントを作る
		string typeName = typeof(T).Name;
		uint compId;
		ulong nativeHandle = InternalAddComponent<T>(entityId_, typeName, ecsGroupName_, out compId);

		T comp = Activator.CreateInstance<T>();
		comp.nativeHandle = nativeHandle;
		comp.entity = this;
		comp.compId = compId;
		components_[typeName] = comp;
		comp.SyncFromNative(ecsGroupName_);

		if (comp == null) {
			Debug.LogError("Failed to create component: " + typeName + " (Entity ID: " + entityId_ + ")");
		}

		Debug.Log("---");
		Debug.Log("--- add component: \n     - component id: " + components_[typeName].compId);
		Debug.Log("---");

		ecsGroup_.componentCollection.AddComponent(comp);
		return comp;
	}

	public T GetComponent<T>() where T : Component {
		if (components_.ContainsKey(typeof(T).Name)) {
			return (T)components_[typeof(T).Name];
		}

		/// コンポーネントを得る
		string typeName = typeof(T).Name;
		uint compId;
		ulong nativeHandle = InternalGetComponent<T>(entityId_, typeName, ecsGroupName_, out compId);

		if (nativeHandle == 0) {
			return null;
		}

		T comp = Activator.CreateInstance<T>();
		comp.nativeHandle = nativeHandle;
		comp.compId = compId;
		comp.entity = this;
		components_[typeName] = comp;
		comp.SyncFromNative(ecsGroupName_);

		ecsGroup_.componentCollection.AddComponent(comp);

		return comp;
	}

	public List<Component> GetComponents() {
		List<Component> result = new List<Component>();
		foreach (var keyValuePair in components_) {
			result.Add(keyValuePair.Value);
		}
		return result;
	}

	public T GetScript<T>() where T : MonoScript {
		/// スクリプトを得る
		string typeName = typeof(T).Name;
		if (scripts_.ContainsKey(typeName)) {
			return (T)scripts_[typeName];
		}
		if (InternalGetScript(entityId_, typeName, ecsGroupName_)) {
			return AddScript<T>();
		}
		return null;
	}

	public MonoScript GetScript(string scriptName) {
		/// スクリプトを得る
		if (scripts_.ContainsKey(scriptName)) {
			return scripts_[scriptName];
		}
		return null;
	}

	public List<MonoScript> GetScripts() {
		List<MonoScript> result = new List<MonoScript>();
		foreach (var keyValuePair in scripts_) {
			result.Add(keyValuePair.Value);
		}
		return result;
	}

	public T AddScript<T>() where T : MonoScript {
		/// スクリプトを得る
		string typeName = typeof(T).Name;
		if (scripts_.ContainsKey(typeName)) {
			/// あったので返す
			return (T)scripts_[typeName];
		}

		/// なかったので新しく作る
		T script = Activator.CreateInstance<T>();
		script.CreateBehavior(entityId_, typeName, ecsGroup_);
		scripts_[typeName] = script;

		/// c++側でもスクリプトを追加
		InternalAddScript(entityId_, typeName, ecsGroupName_);

		return (T)scripts_[typeName];
	}

	public MonoScript AddScript(MonoScript mb) {
		string scriptName = mb.GetType().Name;

		/// スクリプトを得る
		if (scripts_.ContainsKey(scriptName)) {
			return scripts_[scriptName];
		}

		/// なかったので新しく作る
		scripts_[scriptName] = mb;
		/// c++側でもスクリプトを追加
		InternalAddScript(entityId_, scriptName, ecsGroupName_);
		return mb;
	}


	public static implicit operator bool(Entity entity) {
		return entity != null;
	}

	/// ------------------------------------------
	/// internal methods
	/// ------------------------------------------

	[MethodImpl(MethodImplOptions.InternalCall)]
	static extern ulong InternalAddComponent<T>(int entityId, string compTypeName, string groupName, out uint compId);

	[MethodImpl(MethodImplOptions.InternalCall)]
	static extern ulong InternalGetComponent<T>(int entityId, string compTypeName, string groupName, out uint compId);

	[MethodImpl(MethodImplOptions.InternalCall)]
	static extern IntPtr InternalGetName(int entityId, string groupName);

	[MethodImpl(MethodImplOptions.InternalCall)]
	static extern void InternalSetName(int entityId, string name, string groupName);

	[MethodImpl(MethodImplOptions.InternalCall)]
	static extern int InternalGetChildId(int entityId, uint childIndex, string groupName);

	[MethodImpl(MethodImplOptions.InternalCall)]
	static extern int InternalGetChildrenCount(int entityId, string groupName);

	[MethodImpl(MethodImplOptions.InternalCall)]
	static extern int InternalGetParentId(int entityId, string groupName);

	[MethodImpl(MethodImplOptions.InternalCall)]
	static extern void InternalSetParent(int entityId, int parentId, string groupName);

	[MethodImpl(MethodImplOptions.InternalCall)]
	static extern void InternalAddScript(int entityId, string scriptName, string groupName);

	[MethodImpl(MethodImplOptions.InternalCall)]
	static extern bool InternalGetScript(int entityId, string scriptName, string groupName);

	[MethodImpl(MethodImplOptions.InternalCall)]
	static extern bool InternalGetEnable(int entityId, string groupName);

	[MethodImpl(MethodImplOptions.InternalCall)]
	static extern void InternalSetEnable(int entityId, bool enable, string groupName);

}
