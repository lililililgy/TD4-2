

using System.Collections.Generic;

static public class EntityComponentSystem {
	
	///////////////////////////////////////////////////////////////////////////////////////////
	// objects
	///////////////////////////////////////////////////////////////////////////////////////////
	
	static private Dictionary<string, ECSGroup> groups = new Dictionary<string, ECSGroup>();
	
	///////////////////////////////////////////////////////////////////////////////////////////
	// methods
	///////////////////////////////////////////////////////////////////////////////////////////

	/// <summary>
	/// 新規グループの追加
	/// </summary>
	static public ECSGroup AddECSGroup(string name) {
		if (groups.TryGetValue(name, out ECSGroup existingGroup)) {
			return existingGroup;
		}
		ECSGroup group = new ECSGroup(name);
		groups.Add(name, group);
		Debug.LogInfo("EntityComponentSystem.AddECSGroup - added: " + group.groupName + "  GroupCount " + groups.Count);
		return group;
	}

	/// <summary>
	/// 指定したグループをクリアする
	/// </summary>
	static public void ClearECSGroup(string name) {
		if (groups.TryGetValue(name, out ECSGroup group)) {
			group.DeleteEntityAll();
			groups.Remove(name);
			Debug.LogInfo("EntityComponentSystem.ClearECSGroup - cleared: " + name);
		}
	}

	/// <summary>
	/// ECSGroupの取得
	/// </summary>
	static public ECSGroup GetECSGroup(string name) {
#if DEBUG
		Debug.LogInfo("EntityComponentSystem.GetECSGroup - Getting ECSGroup: " + name + "  GroupCount " + groups.Count);
#endif

		if (groups.TryGetValue(name, out ECSGroup group)) {
			return group;
		} else {
#if DEBUG
			Debug.LogError("EntityComponentSystem.GetECSGroup - ECSGroup not found: " + name + "  GroupCount " + groups.Count);
			foreach (var ecsGroup in groups) {
				Debug.LogError("Available ECSGroups: " + ecsGroup.Key);
			}
#endif
			return null;
		}
	}

	
	/// <summary>
	/// すべてのGroupのエンティティを削除する
	/// </summary>
	static public void DeleteEntityAll() {
#if DEBUG
		Debug.Log("//////////////////////////////////////////////////////////////////////////////////////////////////////////////////");
		Debug.Log("EntityComponentSystem.DeleteEntityAll - Deleting all entities from all ECSGroups. GroupCount: " + groups.Count);
		Debug.Log("//////////////////////////////////////////////////////////////////////////////////////////////////////////////////");
		Debug.LogInfo("All entities deleted from all ECSGroups.");
#endif

		foreach (var group in groups.Values) {
			group.DeleteEntityAll();
		}
	}
	

	static public Entity GetEntity(string groupName, int id) {
#if DEBUG
		Debug.LogInfo("EntityComponentSystem.GetEntity - Getting Entity from group: " + groupName + ", ID: " + id);
		Debug.LogInfo("EntityComponentSystem.GetEntity - GroupCount: " + groups.Count);
		foreach (var g in groups) {
			Debug.LogInfo("EntityComponentSystem.GetEntity - Available ECSGroup: " + g.Key);
		}
#endif


		if (groups.TryGetValue(groupName, out ECSGroup group)) {
#if DEBUG
			Debug.LogInfo("EntityComponentSystem.GetEntity - ECSGroup found: " + group.groupName);
#endif
			return group.GetEntity(id);
		} else {
#if DEBUG
			Debug.LogError("EntityComponentSystem.GetEntity - ECSGroup not found: " + groupName);
#endif
			return null;
		}
	}

	static public MonoScript GetMonoBehavior(string groupName, int entityId, string scriptName) {
#if DEBUG
		Debug.LogInfo("EntityComponentSystem.GetMonoBehavior - Getting MonoBehavior from group: " + groupName + ", Entity ID: " + entityId + ", Script Name: " + scriptName);
		Debug.LogInfo("EntityComponentSystem.GetEntity - GroupCount: " + groups.Count);
#endif
		
		if (groups.TryGetValue(groupName, out ECSGroup group)) {
			Entity entity = group.GetEntity(entityId);
			if (entity != null) {
				return entity.GetScript(scriptName);
			} else {
#if DEBUG
				Debug.LogError("EntityComponentSystem.GetMonoBehavior - Entity not found with ID: " + entityId);
#endif
				return null;
			}
		} else {
#if DEBUG
			Debug.LogError("EntityComponentSystem.GetMonoBehavior - ECSGroup not found: " + groupName);
#endif
			return null;
		}
	}

}