

using System.Collections.Generic;

static public class EntityComponentSystem {

	[System.Runtime.InteropServices.DllImport("ONEngine.exe", CallingConvention = System.Runtime.InteropServices.CallingConvention.Cdecl)]
	private static extern void Variables_RegisterSerializeField(string className, string fieldName);

	static public void InitializeSerializeFieldCache() {
		try {
			var asm = System.Reflection.Assembly.GetExecutingAssembly();
			foreach (var type in asm.GetTypes()) {
				foreach (var field in type.GetFields(System.Reflection.BindingFlags.Public | System.Reflection.BindingFlags.NonPublic | System.Reflection.BindingFlags.Instance)) {
					bool shouldSerialize = false;
					if (field.IsPublic) {
						shouldSerialize = true;
					} else {
						foreach (var attr in field.GetCustomAttributes(true)) {
							if (attr.GetType().Name == "SerializeField") {
								shouldSerialize = true;
								break;
							}
						}
					}

					if (shouldSerialize) {
						Variables_RegisterSerializeField(type.Name, field.Name);
					}
				}
			}
		} catch (System.Exception e) {
			Debug.LogError("Failed to initialize SerializeField cache: " + e.Message);
		}
	}
	
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
		return group;
	}

	static public void ClearECSGroup(string name) {
		if (groups.TryGetValue(name, out ECSGroup group)) {
			group.DeleteEntityAll();
			groups.Remove(name);
		}
	}

	static public ECSGroup GetECSGroup(string name) {
		if (groups.TryGetValue(name, out ECSGroup group)) {
			return group;
		} else {
			return null;
		}
	}

	
	static public void DeleteEntityAll() {
		foreach (var group in groups.Values) {
			group.DeleteEntityAll();
		}
	}
	

	static public Entity GetEntity(string groupName, int id) {
		if (groups.TryGetValue(groupName, out ECSGroup group)) {
			return group.GetEntity(id);
		} else {
			return null;
		}
	}

	static public MonoScript GetMonoBehavior(string groupName, int entityId, string scriptName) {
		if (groups.TryGetValue(groupName, out ECSGroup group)) {
			Entity entity = group.GetEntity(entityId);
			if (entity != null) {
				return entity.GetScript(scriptName);
			} else {
				return null;
			}
		} else {
			return null;
		}
	}

}