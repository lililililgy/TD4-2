# ONEngine (TD4-2) BehaviorTree（ビヘイビアツリー） AI作成マニュアル

本ドキュメントは、ONEngine上でボスやザコ敵（Enemy）の行動AIをビヘイビアツリーを使用して構築するためのガイドです。
ビヘイビアツリーエディタ（C++エディタ）の使い方、およびC#によるカスタムアクション・デコレーター・サービスの実装手順を記述しています。

---

## 1. 全体設計と仕組み

ONEngineのAIシステムは、C++エンジン側の **AISystem** とC#側の **AIUpdater**、およびImGuiベースの **BehaviorTreeEditorWindow** が連携して動作します。

```
+------------------+                   +--------------------+
|  C++ AI System   | --(UpdateIntents)--> |   C# AI Updater    |
|                  |                   |  (Loads & ticks    |
|   Applies final  | <---(BatchData)--- |   BehaviorTrees)   |
|   move/attack    |                   |                    |
+------------------+                   +--------------------+
                                                 |
                                         Evaluates Nodes
                                                 v
                                       +--------------------+
                                       | Custom C# Actions  |
                                       | (e.g. ChasePlayer) |
                                       +--------------------+
```

### 1.1 主要な役割
*   **AgentIntentComponent (C++ & C#)**:
    AIの「移動方向」「攻撃中か」「ターゲット情報」といった意思決定データ（意図）を保持するコンポーネントです。
*   **BehaviorTreeEditorWindow (C++ / Editor)**:
    ImGuiを使用して視覚的にビヘイビアツリー（JSON）を構築・編集するグラフィカルツールです。実行時のアクティブノードやBlackboard変数の値をリアルタイムに可視化（デバッグ）できます。
*   **BehaviorTree (C# / Runtime)**:
    JSONからツリーの構造を動的にロード・構築し、毎フレーム評価を実行するスクリプト側の本体です。

---

## 2. BehaviorTreeEditor（エディタ）の使い方

### 2.1 エディタの起動とファイルブラウザ
1.  エンジン（エディタ）を起動し、メニュータブから **[AI Behavior Tree]** (または **[Develop] -> [AI Behavior Tree]**) を開きます。
2.  左側のファイルブラウザ領域から、既存のビヘイビアツリー（`.json`）を選択してダブルクリックで開くか、新しいファイル名を入力して新規作成します。

### 2.2 画面構成
*   **Blackboard（黒板変数）**:
    左下の領域です。AIの状態や標的の情報を保持する変数（Int, Float, Bool, Vector3, String）を定義・管理します。
*   **グラフ編集エリア**:
    メインのノード配置・リンク接続エリアです。
    - **右クリック**: 各種ノード（`Sequence`, `Selector`, `Parallel` など、および自作したC#カスタムアクション）を生成します。
    - **ドラッグ接続**: ノードの出力ピン（Output）から入力ピン（Input）に接続して実行フローを定義します。
*   **ノードインスペクター**:
    右側の領域です。選択したノードの詳細プロパティを変更したり、ノードに **Decorator（実行条件）** や **Service（補助処理）** を追加したりできます。

### 2.3 構築の基本ルール
*   ツリーは **Root** ノード（開始地点）から実行フローが開始されます。
*   **Compositeノード（制御フロー）**:
    - **`Sequence`**: 子ノードを上（左）から順番に評価します。すべてが **Success** を返せば自身も Success を返します。いずれかが **Failure** を返せば中断して Failure を返し、**Running** を返せばそのノードの評価を次フレームでも継続します。
    - **`Selector`**: 子ノードを上（左）から順番に評価します。いずれか一つでも **Success** または **Running** を返せば、以降の子ノードは評価せずにその状態を親に返します。すべてが **Failure** だった場合のみ Failure を返します。
*   エディタ上で子ノードを接続した際、**表示位置のY座標が小さい（上にある）順**に優先して実行されます。

---

## 3. C#によるカスタムアクションノードの作成

エネミーの「プレイヤーを追跡する」「プレイヤーを攻撃する」といった具体的な行動は、C#側で `BehaviorNode` を継承したクラスを作成することで実装します。

### 3.1 アクションノードの実装テンプレート
`SubProjects/CSharpLibrary/Scripts/Game/`（または任意のC#スクリプト領域）に以下の構造で作成します。

```csharp
using System;

// プレイヤーに接近するアクションノードの例
public class MoveToPlayer : BehaviorNode {
	// 公開フィールドはエディタのインスペクター上でパラメータとして編集可能になります
	public float chaseSpeed = 5.0f;
	public float stopDistance = 1.5f;

	// Blackboardからキーを指定して値を取得するためのフィールド
	[BlackboardKey]
	public string targetEntityIdKey = "TargetEntityId";

	/// <summary>
	/// ノードが読み込まれた際の初期化処理（必要なコンポーネントの参照取得など）
	/// </summary>
	public override void Initialize() {
		// 初期化処理が必要であればここに記述します
	}

	/// <summary>
	/// 毎フレーム呼び出されるメインロジック
	/// </summary>
	protected override NodeStatus OnTick(float deltaTime) {
		// 1. Blackboardから標的のEntity IDを取得
		int targetId = tree.GetBlackboardValue<int>(targetEntityIdKey, -1);
		if (targetId == -1) {
			return NodeStatus.Failure; // ターゲットがいないため行動失敗
		}

		// 2. 自エンティティのアタッチされたコンポーネント（Transform, AgentIntentComponent）を取得
		// tree.entityId がこのAIを実行しているエンティティのIDです
		var ecsGroup = EntityComponentSystem.GetECSGroup(tree.ecsGroupName);
		var selfEntity = ecsGroup.GetEntity(tree.entityId);
		if (selfEntity == null) return NodeStatus.Failure;

		var intent = selfEntity.GetComponent<AgentIntentComponent>();
		if (intent == null) return NodeStatus.Failure;

		// 3. 標的エンティティの位置を取得して移動方向を決定
		var targetEntity = ecsGroup.GetEntity(targetId);
		if (targetEntity == null) return NodeStatus.Failure;

		Vector3 selfPos = selfEntity.transform.position;
		Vector3 targetPos = targetEntity.transform.position;

		float distance = Vector3.Distance(selfPos, targetPos);
		if (distance <= stopDistance) {
			// 目標距離に到達したためSuccessを返す（移動方向はリセット）
			intent.desiredMoveDirection = Vector3.zero;
			return NodeStatus.Success;
		}

		// 接近方向を計算してインテントに書き込む
		Vector3 dir = (targetPos - selfPos).Normalized();
		intent.desiredMoveDirection = dir;
		intent.maxSpeed = chaseSpeed;

		// プレイヤーの方向を向くように設定
		intent.useDesiredRotation = true;
		intent.desiredRotation = Quaternion.LookRotation(dir, Vector3.up);

		// 移動継続中であることを示すためにRunningを返す
		return NodeStatus.Running;
	}
}
```

### 3.2 戻り値（NodeStatus）のルール
*   **`NodeStatus.Success`**: アクションが完了し、成功したことを示します。親ノード（Sequence等）は次のノードの処理へ進みます。
*   **`NodeStatus.Failure`**: アクションが失敗した（または条件を満たさなくなった）ことを示します。
*   **`NodeStatus.Running`**: アクションが現在進行中であることを示します。次フレームも同じノードの `OnTick` から実行を再開します。

---

## 4. Decorator（デコレーター）と Service（サービス）の作成

エディタ上のノードに対して、「特定の時だけ実行する（Decorator）」「実行中に常にバックグラウンドで処理を行う（Service）」といった機能を追加できます。

### 4.1 カスタムデコレーターの実装例
`BehaviorDecorator` を継承し、クラスに `[Decorator]` 属性を付与します。

```csharp
using System;

// HPが一定割合以下の場合にのみ実行を許可するデコレーター
[Decorator]
public class CheckHPPercentage : BehaviorDecorator {
	public float threshold = 0.3f; // 30%以下

	public override bool Condition() {
		// 実行元エンティティの情報を取得
		var ecsGroup = EntityComponentSystem.GetECSGroup(node.tree.ecsGroupName);
		var entity = ecsGroup.GetEntity(node.tree.entityId);
		if (entity == null) return false;

		// 例：エンティティにアタッチされているEnemyスクリプトからHP比率を取得
		var enemyScript = entity.GetScript("Enemy") as Enemy; // プロジェクト内のEnemyクラス名に適宜書き換え
		if (enemyScript != null) {
			return (enemyScript.CurrentHP / enemyScript.MaxHP) <= threshold;
		}

		return false;
	}
}
```

### 4.2 カスタムサービスの実装例
`BehaviorService` を継承します。

```csharp
using System;

// 実行中に毎フレーム自動的に一番近いプレイヤーを検出し、Blackboardに書き込むサービス
public class DistanceSensorService : BehaviorService {
	public float detectRange = 15.0f;

	[BlackboardKey]
	public string targetEntityIdKey = "TargetEntityId";

	public override void Update(float deltaTime) {
		var ecsGroup = EntityComponentSystem.GetECSGroup(node.tree.ecsGroupName);
		var selfEntity = ecsGroup.GetEntity(node.tree.entityId);
		if (selfEntity == null) return;

		// プレイヤーに該当するエンティティを探す
		Entity playerEntity = ecsGroup.FindEntity("Player");
		if (playerEntity != null) {
			float dist = Vector3.Distance(selfEntity.transform.position, playerEntity.transform.position);
			if (dist <= detectRange) {
				// BlackboardにプレイヤーのEntity IDを設定
				node.tree.SetBlackboardValue(targetEntityIdKey, playerEntity.Id);
				return;
			}
		}

		// 検出範囲外ならターゲットIDをクリア (-1)
		node.tree.SetBlackboardValue(targetEntityIdKey, -1);
	}
}
```

---

## 5. AIを実機キャラクターに適用する手順

実際に作成したBehaviorTreeをボスやEnemyのエンティティに適用するステップです。

### STEP 1: C#のEnemyスクリプト側でパスを宣言
エンティティにアタッチするスクリプト（例：`Enemy.cs`）に、ビヘイビアツリーのJSONファイルを指定するフィールド（`behaviorTreePath`）を実装します。

```csharp
public class Enemy : MonoScript {
	// シリアライズ可能なフィールド。エディタのインスペクターからツリーパスを設定します
	[SerializeField] private string behaviorTreePath = "Assets/AI/BossBehavior.json";

	// HP情報など（カスタムデコレーターが参照する値）
	public float MaxHP = 100f;
	public float CurrentHP = 100f;

	public override void Initialize() {
		CurrentHP = MaxHP;
	}

	public override void Update() {
		// 移動などはAISystem(C++)がAgentIntentComponentの情報をもとに行うため、
		// ここでは個別の移動ロジックを書く必要はありません。
	}
}
```

### STEP 2: シーン上でのコンポーネント割り当て
1.  エディタのシーンウィンドウ上で、AIを適用したいキャラクターのEntityを選択します。
2.  そのEntityに対して、以下の2つをアタッチ（追加）します。
    - **`AgentIntentComponent`** (AIの移動・アクション意図をやり取りするためのコンポーネント)
    - 作成した **C# の `Enemy` スクリプト** (またはボス用のスクリプト)
3.  C#のスクリプトコンポーネントのパラメータ項目にある `behaviorTreePath` に対して、作成したツリーJSONのパス（例：`Assets/AI/BossBehavior.json`）を設定します。

### STEP 3: 実行とリアルタイムデバッグ
1.  ゲームを再生（実行）します。
2.  **[AI Behavior Tree]** エディタウィンドウで該当するJSONファイルを開いた状態にすると、現在実行中のエージェントの処理フローに合わせて**アクティブなノードがリアルタイムに可視化（緑色の輪郭表示など）されます**。
3.  エディタ上でBlackboardの変数リストの値が動的に変化していく様子を観察し、AIの思考状況をデバッグできます。
