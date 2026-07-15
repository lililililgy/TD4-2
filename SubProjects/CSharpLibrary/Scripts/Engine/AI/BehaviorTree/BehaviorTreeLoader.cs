using System;
using System.Collections.Generic;
using System.Reflection;

public static class BehaviorTreeLoader {
	public static BehaviorTree LoadFromFile(string path, Entity owner) {
		Debug.LogWarning("BehaviorTreeLoader.LoadFromFile is stubbed out. Cannot load tree: " + path);
		return null;
	}

	public static uint HashString(string str) {
		if (string.IsNullOrEmpty(str)) return 0;
		uint hash = 2166136261;
		foreach (char c in str) {
			hash = (hash ^ c) * 16777619;
		}
		return hash;
	}
}
