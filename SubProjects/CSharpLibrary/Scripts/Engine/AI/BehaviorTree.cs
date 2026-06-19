using System;
using System.Collections.Generic;
using System.IO;
using System.Reflection;
using System.Runtime.CompilerServices;
using System.Linq;
using Newtonsoft.Json.Linq;

public class BehaviorTree {
	public string treePath;
	public int entityId;
	public string ecsGroupName;
	public BehaviorNode rootNode;

	// ブラックボード
	private Dictionary<string, object> blackboard_ = new Dictionary<string, object>();

	// C++側エディタとリアルタイム同期するためのInternal Call
	[MethodImpl(MethodImplOptions.InternalCall)]
	internal static extern void Internal_UpdateNodeStatus(uint nodeIdHash, int status, string treePath);

	[MethodImpl(MethodImplOptions.InternalCall)]
	internal static extern void Internal_UpdateBlackboardValue(uint keyHash, string value, string typeName);

	public void UpdateNodeStatus(uint runtimeId, NodeStatus status) {
		Internal_UpdateNodeStatus(runtimeId, (int)status, treePath);
	}

	public void SetBlackboardValue(string key, object value) {
		blackboard_[key] = value;
		uint keyHash = GetHash(key);
		string valStr = value?.ToString() ?? "null";
		string typeName = value?.GetType().Name ?? "null";
		Internal_UpdateBlackboardValue(keyHash, valStr, typeName);
	}

	public T GetBlackboardValue<T>(string key, T defaultValue = default) {
		if (blackboard_.TryGetValue(key, out var val)) {
			try {
				return (T)Convert.ChangeType(val, typeof(T));
			} catch {
				return (T)val;
			}
		}
		return defaultValue;
	}

	private uint GetHash(string str) {
		uint hash = 2166136261;
		foreach (char c in str) {
			hash = (hash ^ c) * 16777619;
		}
		return hash;
	}

	public void Load(string jsonPath) {
		this.treePath = jsonPath;
		if (!File.Exists(jsonPath)) {
			throw new FileNotFoundException($"BehaviorTree JSON not found: {jsonPath}");
		}

		string jsonStr = File.ReadAllText(jsonPath);
		JObject root = JObject.Parse(jsonStr);

		// 1. Blackboardの初期化
		if (root["blackboard"] != null) {
			foreach (var varJ in root["blackboard"]) {
				string key = (string)varJ["key"];
				int type = (int)varJ["type"];
				object val = null;
				switch (type) {
					case 0: val = (int)varJ["iVal"]; break;     // Int
					case 1: val = (float)varJ["fVal"]; break;   // Float
					case 2: val = (bool)varJ["bVal"]; break;    // Bool
					case 3:                                     // Vector3
						var vArr = varJ["vVal"];
						val = new Vector3((float)vArr[0], (float)vArr[1], (float)vArr[2]);
						break;
					case 4: val = (string)varJ["sVal"]; break;  // String
				}
				blackboard_[key] = val;
			}
		}

		// 2. ノードのインスタンス化
		var nodesMap = new Dictionary<string, BehaviorNode>(); // JSON ID -> Node
		var pinToNodeMap = new Dictionary<uint, BehaviorNode>(); // Pin ID -> Node
		var nodePositions = new Dictionary<string, float>(); // JSON ID -> Y Position (for sorting)

		if (root["nodes"] != null) {
			foreach (var nodeJ in root["nodes"]) {
				string jsonId = nodeJ["id"].ToString();
				string className = (string)nodeJ["className"];
				string nodeName = (string)nodeJ["name"];
				uint runtimeId = (uint)nodeJ["id"];

				BehaviorNode node = CreateNodeInstance(className);
				if (node == null) {
					continue;
				}

				node.id = jsonId;
				node.runtimeId = runtimeId;
				node.name = nodeName;
				node.tree = this;

				// 位置情報の保持（実行順序ソート用：Y座標）
				var posArr = nodeJ["pos"];
				if (posArr != null) {
					nodePositions[jsonId] = (float)posArr[1];
				} else {
					nodePositions[jsonId] = 0f;
				}

				// プロパティの設定 (リフレクション)
				if (nodeJ["properties"] != null) {
					ApplyProperties(node, (JObject)nodeJ["properties"]);
				}

				// デコレーターのインスタンス化と適用
				if (nodeJ["decorators"] != null) {
					foreach (var decJ in nodeJ["decorators"]) {
						string dClass = (string)decJ["className"];
						string dName = (string)decJ["name"];
						var dec = CreateDecoratorInstance(dClass);
						if (dec != null) {
							dec.name = dName;
							dec.node = node;
							if (decJ["properties"] != null) {
								ApplyProperties(dec, (JObject)decJ["properties"]);
							}
							node.decorators.Add(dec);
						}
					}
				}

				// サービスのインスタンス化と適用
				if (nodeJ["services"] != null) {
					foreach (var svcJ in nodeJ["services"]) {
						string sClass = (string)svcJ["className"];
						string sName = (string)svcJ["name"];
						var svc = CreateServiceInstance(sClass);
						if (svc != null) {
							svc.name = sName;
							svc.node = node;
							if (svcJ["properties"] != null) {
								ApplyProperties(svc, (JObject)svcJ["properties"]);
							}
							node.services.Add(svc);
						}
					}
				}

				nodesMap[jsonId] = node;

				// 入力/出力ピンのマッピング
				if (nodeJ["inputs"] != null) {
					foreach (var pin in nodeJ["inputs"]) {
						uint pinId = (uint)pin["id"];
						pinToNodeMap[pinId] = node;
					}
				}
				if (nodeJ["outputs"] != null) {
					foreach (var pin in nodeJ["outputs"]) {
						uint pinId = (uint)pin["id"];
						pinToNodeMap[pinId] = node;
					}
				}
			}
		}

		// 3. リンクに基づいたツリー構造の再構築
		var parentToChildrenMap = new Dictionary<string, List<BehaviorNode>>(); // Parent JSON ID -> Children

		if (root["links"] != null) {
			foreach (var linkJ in root["links"]) {
				uint startPin = (uint)linkJ["startPin"];
				uint endPin = (uint)linkJ["endPin"];

				if (pinToNodeMap.TryGetValue(startPin, out var parentNode) &&
					pinToNodeMap.TryGetValue(endPin, out var childNode)) {
					
					if (!parentToChildrenMap.TryGetValue(parentNode.id, out var childList)) {
						childList = new List<BehaviorNode>();
						parentToChildrenMap[parentNode.id] = childList;
					}
					childList.Add(childNode);
				}
			}
		}

		// 子ノードのソート（エディタ上のY座標順に実行するようにする）
		foreach (var pair in parentToChildrenMap) {
			string parentId = pair.Key;
			var childList = pair.Value;
			// エディタの見た目通りに上から実行するため、Y座標が小さい（上にある）順にソートする
			var sortedChildren = childList.OrderBy(c => nodePositions.ContainsKey(c.id) ? nodePositions[c.id] : 0f).ToList();

			if (nodesMap.TryGetValue(parentId, out var parentNode)) {
				parentNode.children = sortedChildren;
			}
		}

		// 4. ルートノードの特定（className が "Root" または 入力リンクを持たないノード）
		// 通常、エディタの開始ノードの className は "Root" または "RootNode"
		rootNode = nodesMap.Values.FirstOrDefault(n => n.name == "Root" || n.name == "RootNode");
		if (rootNode == null) {
			// 見つからない場合は、入力リンクを持たないノードを探す
			var allChildrenIds = new HashSet<string>(parentToChildrenMap.Values.SelectMany(list => list.Select(c => c.id)));
			rootNode = nodesMap.Values.FirstOrDefault(n => !allChildrenIds.Contains(n.id));
		}

		// 5. 各ノードの Initialize の呼び出し
		foreach (var node in nodesMap.Values) {
			node.Initialize();
		}
	}

	public void Tick(float deltaTime) {
		if (rootNode != null) {
			rootNode.Tick(deltaTime);
		}
	}

	private BehaviorNode CreateNodeInstance(string className) {
		if (className == "Sequence") return new Sequence();
		if (className == "Selector") return new Selector();
		if (className == "Parallel") return new Parallel();

		// リフレクションでカスタムクラスを探す
		Type type = FindTypeInAssemblies(className);
		if (type != null) {
			return (BehaviorNode)Activator.CreateInstance(type);
		}
		return null;
	}

	private BehaviorDecorator CreateDecoratorInstance(string className) {
		if (className == "BlackboardDecorator") return new BlackboardDecorator();

		Type type = FindTypeInAssemblies(className);
		if (type != null) {
			return (BehaviorDecorator)Activator.CreateInstance(type);
		}
		return null;
	}

	private BehaviorService CreateServiceInstance(string className) {
		Type type = FindTypeInAssemblies(className);
		if (type != null) {
			return (BehaviorService)Activator.CreateInstance(type);
		}
		return null;
	}

	private Type FindTypeInAssemblies(string className) {
		// 現在読み込まれているアセンブリから検索
		foreach (var assembly in AppDomain.CurrentDomain.GetAssemblies()) {
			Type type = assembly.GetType(className);
			if (type != null) return type;
		}
		return null;
	}

	private void ApplyProperties(object target, JObject propJ) {
		Type type = target.GetType();
		foreach (var prop in propJ) {
			string name = prop.Key;
			string valStr = prop.Value.ToString();

			FieldInfo field = type.GetField(name, BindingFlags.Public | BindingFlags.Instance | BindingFlags.NonPublic);
			if (field != null) {
				try {
					object val = ConvertType(valStr, field.FieldType);
					field.SetValue(target, val);
				} catch (Exception ex) {
					// 変換失敗時は無視するかログを出す
				}
			}
		}
	}

	private object ConvertType(string valStr, Type targetType) {
		if (targetType == typeof(string)) return valStr;
		if (targetType == typeof(int)) return int.Parse(valStr);
		if (targetType == typeof(float)) return float.Parse(valStr);
		if (targetType == typeof(bool)) return bool.Parse(valStr);
		if (targetType == typeof(Vector3)) {
			// "[x, y, z]" のような文字列を Vector3 にパースする
			string clean = valStr.Trim('[', ']');
			string[] split = clean.Split(',');
			if (split.Length >= 3) {
				return new Vector3(float.Parse(split[0]), float.Parse(split[1]), float.Parse(split[2]));
			}
		}
		if (targetType.IsEnum) {
			return Enum.Parse(targetType, valStr);
		}
		return Convert.ChangeType(valStr, targetType);
	}
}

// ==========================================
// 標準 Composite ノードの実装
// ==========================================

/// <summary>
/// 子ノードを左（上）から順に実行し、すべて成功すればSuccessを返す。
/// いずれかがFailureを返せばそこで終了しFailureを返す。
/// いずれかがRunningを返せばそのノードで処理を留めRunningを返す。
/// </summary>
public class Sequence : BehaviorNode {
	private int currentIndex_ = 0;

	protected override NodeStatus OnTick(float deltaTime) {
		if (children.Count == 0) return NodeStatus.Success;

		for (int i = currentIndex_; i < children.Count; i++) {
			NodeStatus status = children[i].Tick(deltaTime);
			if (status == NodeStatus.Running) {
				currentIndex_ = i;
				return NodeStatus.Running;
			}
			if (status == NodeStatus.Failure) {
				currentIndex_ = 0;
				return NodeStatus.Failure;
			}
		}
		currentIndex_ = 0;
		return NodeStatus.Success;
	}
}

/// <summary>
/// 子ノードを左（上）から順に実行し、いずれかがSuccessを返せばそこで終了しSuccessを返す。
/// いずれかがRunningを返せばそこで留まりRunningを返す。
/// すべてがFailureを返せばFailureを返す。
/// </summary>
public class Selector : BehaviorNode {
	private int currentIndex_ = 0;

	protected override NodeStatus OnTick(float deltaTime) {
		if (children.Count == 0) return NodeStatus.Failure;

		for (int i = currentIndex_; i < children.Count; i++) {
			NodeStatus status = children[i].Tick(deltaTime);
			if (status == NodeStatus.Running) {
				currentIndex_ = i;
				return NodeStatus.Running;
			}
			if (status == NodeStatus.Success) {
				currentIndex_ = 0;
				return NodeStatus.Success;
			}
		}
		currentIndex_ = 0;
		return NodeStatus.Failure;
	}
}

/// <summary>
/// すべての子ノードを同時に（並行して）実行する。
/// </summary>
public class Parallel : BehaviorNode {
	protected override NodeStatus OnTick(float deltaTime) {
		bool anyRunning = false;
		bool anyFailure = false;

		foreach (var child in children) {
			NodeStatus status = child.Tick(deltaTime);
			if (status == NodeStatus.Running) {
				anyRunning = true;
			} else if (status == NodeStatus.Failure) {
				anyFailure = true;
			}
		}

		if (anyFailure) return NodeStatus.Failure;
		if (anyRunning) return NodeStatus.Running;
		return NodeStatus.Success;
	}
}

// ==========================================
// 標準 Decorator ノードの実装
// ==========================================

/// <summary>
/// Blackboardの値をもとに条件分岐を行うデコレーター
/// </summary>
public class BlackboardDecorator : BehaviorDecorator {
	public string keyName;
	public string op; // "Equal", "NotEqual", "GreaterThan", "LessThan", "IsTrue", "IsFalse"
	public string value;

	public override bool Condition() {
		if (node == null || node.tree == null) return false;
		object currentVal = node.tree.GetBlackboardValue<object>(keyName);
		if (currentVal == null) {
			return op == "IsFalse" || op == "NotEqual";
		}

		string currentValStr = currentVal.ToString();

		switch (op) {
			case "Equal":
				return currentValStr == value;
			case "NotEqual":
				return currentValStr != value;
			case "IsTrue":
				if (currentVal is bool b) return b;
				return currentValStr.ToLower() == "true";
			case "IsFalse":
				if (currentVal is bool b2) return !b2;
				return currentValStr.ToLower() == "false";
			case "GreaterThan":
				return float.Parse(currentValStr) > float.Parse(value);
			case "LessThan":
				return float.Parse(currentValStr) < float.Parse(value);
			default:
				return false;
		}
	}
}
