using System;
using System.Collections.Generic;
using System.Reflection;
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;

/// <summary>
/// C++エディタで作成され JSON フォーマットで保存されたビヘイビアツリー資産を読み込み、
/// C#の実行用インスタンスとして展開（デシリアライズ）する静的ローダークラス。
/// </summary>
public static class BehaviorTreeLoader {
	/// <summary>
	/// 指定されたファイルパスのJSONを読み込み、ビヘイビアツリーを構築する。
	/// </summary>
	/// <param name="path">JSONファイルのパス</param>
	/// <param name="owner">このツリーを実行するエンティティ（AI本体）</param>
	/// <returns>構築済みのBehaviorTreeインスタンス、失敗した場合はnull</returns>
	public static BehaviorTree LoadFromFile(string path, Entity owner) {
		// 1. ファイルの読み込みとパース
		string jsonText = Mathf.LoadFile(path);
		if (string.IsNullOrEmpty(jsonText)) {
			return null;
		}

		var root = JObject.Parse(jsonText);
		BehaviorTree tree = new BehaviorTree(owner);
		tree.SourcePath = path; // Normalized in property setter


		// 2. Blackboard（共有変数）のロード
		if (root["blackboard"] != null) {
			foreach (var v in root["blackboard"]) {
				string key = (string)v["key"];
				uint keyHash = HashString(key);
				int type = (int)v["type"];

				switch (type) {
				case 0: // Int
					tree.Blackboard.SetInt(keyHash, (int)v["iVal"]);
					break;
				case 1: // Float
					tree.Blackboard.SetFloat(keyHash, (float)v["fVal"]);
					break;
				case 2: // Bool
					tree.Blackboard.SetBool(keyHash, (bool)v["bVal"]);
					break;
				case 3: // Vector3
					var va = v["vVal"];
					Vector3 v3 = new Vector3((float)va[0], (float)va[1], (float)va[2]);
					tree.Blackboard.SetVector3(keyHash, v3);
					break;
				case 4: // String
					tree.Blackboard.SetString(keyHash, (string)v["sVal"]);
					break;
				}
				tree.Blackboard.SaveAsDefault(keyHash);
			}
		}

		// 3. ノード（タスク・コンポジット）とモジュール（デコレーター・サービス）のインスタンス化
		Dictionary<ulong, BehaviorNode> nodeInstances = new Dictionary<ulong, BehaviorNode>();
		Dictionary<ulong, ulong> pinToNodeMap = new Dictionary<ulong, ulong>();
		ulong entryNodeId = 0;

		foreach (var n in root["nodes"]) {
			ulong id = (ulong)n["id"];
			string className = (string)n["className"];

			if (className == "Entry") {
				entryNodeId = id;
				tree.EntryNodeId = (uint)id;
				foreach (var pin in n["outputs"]) pinToNodeMap[(ulong)pin["id"]] = id;
				continue;
			}

			Type type = Type.GetType(className);
			if (type == null) type = Type.GetType(className + ", CSharpLibrary");
			if (type == null) type = Type.GetType(className + ", Engine");

			if (type != null) {
				BehaviorNode node = (BehaviorNode)Activator.CreateInstance(type);
				node.NodeId = (int)id; // 生のIDを保存
				node.NodeIdHash = (uint)id;
				node.name = (string)n["name"] ?? className;
				if (string.IsNullOrEmpty(node.name)) node.name = className;

				node.Tree = tree;
				if (n["hasBreakpoint"] != null) node.HasBreakpoint = (bool)n["hasBreakpoint"];

				nodeInstances[id] = node;

				ApplyProperties(type, node, n["properties"]);

				// 4. Decorator のロード
				if (n["decorators"] is JArray decorators) {
					foreach (var d in decorators) {
						string dClassName = (string)d["className"];
						Type dType = Type.GetType(dClassName);
						if (dType == null) dType = Type.GetType(dClassName + ", CSharpLibrary");
						if (dType != null) {
							var decorator = (BehaviorDecorator)Activator.CreateInstance(dType);
							if (d["id"] != null) decorator.NodeIdHash = (uint)d["id"];
							ApplyProperties(dType, decorator, d["properties"]);
							node.AddDecorator(decorator);
						}
					}
				}

				// 5. Service のロード
				if (n["services"] is JArray services) {
					foreach (var s in services) {
						string sClassName = (string)s["className"];
						Type sType = Type.GetType(sClassName);
						if (sType == null) sType = Type.GetType(sClassName + ", CSharpLibrary");
						if (sType != null) {
							var service = (BehaviorService)Activator.CreateInstance(sType);
							if (s["id"] != null) service.NodeIdHash = (uint)s["id"];
							ApplyProperties(sType, service, s["properties"]);
							node.AddService(service);
						}
					}
				}

				if (n["inputs"] != null) foreach (var pin in n["inputs"]) pinToNodeMap[(ulong)pin["id"]] = id;
				if (n["outputs"] != null) foreach (var pin in n["outputs"]) pinToNodeMap[(ulong)pin["id"]] = id;
			} else {
			}
		}

		// 7. リンク情報の構築とバリデーション
		var links = new List<JToken>(root["links"]);
		int linkErrorCount = 0;

		// リンクで使用されているピンがすべて存在するか事前にチェック
		foreach (var l in links) {
			ulong startPin = (ulong)l["startPin"];
			ulong endPin = (ulong)l["endPin"];
			ulong linkId = (ulong)l["id"];

			if (!pinToNodeMap.ContainsKey(startPin)) {
				linkErrorCount++;
			}
			if (!pinToNodeMap.ContainsKey(endPin)) {
				linkErrorCount++;
			}
		}

		if (linkErrorCount > 0) {
		}

		links.Sort((a, b) => {
			ulong childIdA = 0, childIdB = 0;
			pinToNodeMap.TryGetValue((ulong)a["endPin"], out childIdA);
			pinToNodeMap.TryGetValue((ulong)b["endPin"], out childIdB);

			float yA = 0, yB = 0;
			foreach (var n in root["nodes"]) {
				if ((ulong)n["id"] == childIdA) yA = (float)n["pos"][1];
				if ((ulong)n["id"] == childIdB) yB = (float)n["pos"][1];
			}
			return yA.CompareTo(yB);
		});

		foreach (var l in links) {
			ulong startPin = (ulong)l["startPin"];
			ulong endPin = (ulong)l["endPin"];

			if (pinToNodeMap.TryGetValue(startPin, out ulong parentId) &&
				pinToNodeMap.TryGetValue(endPin, out ulong childId)) {
				if (parentId == entryNodeId) {
					if (nodeInstances.TryGetValue(childId, out var rootNode)) {
						tree.RootNode = rootNode;
					} else {
					}
				} else if (nodeInstances.TryGetValue(parentId, out var parentNode) &&
						   nodeInstances.TryGetValue(childId, out var childNode)) {
					if (parentNode is CompositeNode composite) {
						composite.AddChild(childNode);
						childNode.Parent = parentNode;
					} else {
					}
				} else {
					if (!nodeInstances.ContainsKey(parentId) && parentId != entryNodeId) {
					}
					if (!nodeInstances.ContainsKey(childId)) {
					}
				}
			}
		}

		if (tree.RootNode == null) {
		}

		tree.InitializeMonitoring();
		return tree;
	}

	private static void ApplyProperties(Type type, object instance, JToken props) {
		if (props == null) return;
		foreach (var p in props.Children<JProperty>()) {
			FieldInfo field = type.GetField(p.Name, BindingFlags.Public | BindingFlags.Instance);
			if (field != null) {
				try {
					object val = ConvertValue(field.FieldType, p.Value.ToString());
					field.SetValue(instance, val);
				} catch (Exception e) {
				}
			} else {
				PropertyInfo prop = type.GetProperty(p.Name, BindingFlags.Public | BindingFlags.Instance);
				if (prop != null && prop.CanWrite) {
					try {
						object val = ConvertValue(prop.PropertyType, p.Value.ToString());
						prop.SetValue(instance, val);
					} catch (Exception e) {
					}
				} else {
				}
			}
		}
	}

	private static object ConvertValue(Type type, string value) {
		if (type == typeof(string)) return value;
		if (type == typeof(int)) return int.Parse(value);
		if (type == typeof(float)) return float.Parse(value);
		if (type == typeof(bool)) return bool.Parse(value);
		if (type == typeof(Vector3)) {
			var parts = value.Split(',');
			if (parts.Length == 3) {
				return new Vector3(float.Parse(parts[0]), float.Parse(parts[1]), float.Parse(parts[2]));
			}
		}
		if (type == typeof(Vector4)) {
			var parts = value.Split(',');
			if (parts.Length == 4) {
				return new Vector4(float.Parse(parts[0]), float.Parse(parts[1]), float.Parse(parts[2]), float.Parse(parts[3]));
			}
		}
		if (type.IsEnum) {
			if (int.TryParse(value, out int intVal)) {
				object result = Enum.ToObject(type, intVal);
				return result;
			}
			return Enum.Parse(type, value, true);
		}
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

