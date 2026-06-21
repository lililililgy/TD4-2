# ONEngine (TD4-2) BehaviorTree（ビヘイビアツリー） AI作成マニュアル

本ドキュメントは、ONEngine上でボスやザコ敵（Enemy）の行動AIをビヘイビアツリーを使用して構築するための総合ガイドです。
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
                                       | (e.g. MoveToPos)   |
                                       +--------------------+
```

### 1.1 主要な役割
*   **AgentIntentComponent (C++ & C#)**:
    AIの「移動方向」「回転方向」「攻撃フラグ」「ターゲット情報」といった最終的な意思決定（意図）を格納し、C++側とC#側で共有するためのコンポーネントです。
*   **BehaviorTreeEditorWindow (C++ / Editor)**:
    視覚的にビヘイビアツリー（JSON）を構築・編集するグラフィカルツールです。実行中のアクティブノードやBlackboard変数の値をリアルタイムに可視化（デバッグ）できます。
*   **BehaviorTree (C# / Runtime)**:
    JSONからツリーの構造を動的にロードし、Blackboard（記憶領域）を用いて毎フレーム評価（Tick）を実行するスクリプト側のAI実行エンジン本体です。

---

## 2. BehaviorTreeEditor（エディタ）の使い方

### 2.1 エディタの起動とファイル操作
1.  エンジン（エディタ）を起動し、メニュータブから **[AI Behavior Tree]** (または **[Develop] -> [AI Behavior Tree]**) を開きます。
2.  左側のファイルブラウザ領域から、既存のビヘイビアツリー（`.json`）を選択してダブルクリックで開くか、新しいファイル名を入力して新規作成します。

### 2.2 画面構成
*   **Blackboard（黒板変数）**:
    左下の領域です。AIの状態や標的の情報を保持する変数（Int, Float, Bool, Vector3, String, Entity）を定義・管理します。
*   **グラフ編集エリア**:
    メインのノード配置・リンク接続エリアです。
    - **右クリック**: 各種制御ノードやカスタムアクションノードを生成します。
    - **ドラッグ接続**: ノードの出力ピン（Output）から別ノードの入力ピン（Input）に接続して実行フローを定義します。
*   **ノードインスペクター**:
    右側の領域です。選択したノードの詳細プロパティ（公開フィールド）を変更したり、ノードに **Decorator（実行条件）** や **Service（定期処理）** を追加したりできます。

### 2.3 フロー制御の基本ルール
*   ツリーは **Root** ノードから下に繋がるノードへ実行フローが開始されます。
*   **Compositeノード（制御フロー）**:
    - **`Sequence`**: 子ノードを上（左）から順番に評価します。すべてが **Success** を返せば自身も Success を返します。いずれかが **Failure** を返せばそこで処理を止め Failure を返し、**Running** を返せばそのノードの評価を次フレームでも継続します。
    - **`Selector`**: 子ノードを上（左）から順番に評価します。いずれか一つでも **Success** または **Running** を返せば、以降の子ノードは評価せずにその状態を親に返します。すべてが **Failure** だった場合のみ Failure を返します。
    - **`Parallel`**: すべての子ノードを同時に（並行して）実行します。
*   **優先順位のルール**:
    エディタ上で複数の子ノードに接続している場合、子ノードの**エディタ上の表示位置のY座標が小さい（上にある）順**に優先して実行されます。

---

## 3. 標準提供ノード・モジュール一覧

プロジェクトには、すでに実用的で多彩なノード・モジュール群が C# ライブラリ内に実装されています。これらを組み合わせるだけで、多くのAI行動を構築できます。

### 3.1 Composite ノード（制御フロー）
*   **`Sequence`**: 子ノードを上から順に実行し、すべて成功で Success。
*   **`Selector`**: 子ノードを上から順に実行し、いずれか成功で Success。
*   **`Parallel`**: すべての子ノードを同時に実行。
*   **`RandomSelector`**: 子ノードをランダムに選んで実行（ノードごとの確率ウェイトに基づく）。

### 3.2 標準アクションノード
*   **`WaitNode`**: 指定時間待機する。
    - プロパティ: `duration` (待機秒数)、`durationKey` (Blackboardキーから取得する場合)、`animationName` (待機中に再生するアニメーション)。
*   **`WaitRandomNode`**: 最小〜最大時間のランダムな秒数待機する。
    - プロパティ: `minDuration` (最小秒数)、`maxDuration` (最大秒数)。
*   **`MoveToPosNode`**: Blackboard上の Vector3 座標に向かって移動する。
    - プロパティ: `posKey` (目標座標が格納されているBlackboardキー)、`stopDistance` (到着とみなす許容距離)。
*   **`RotateToFaceNode`**: Blackboard上のターゲット（Entity）に向き直る。
    - プロパティ: `targetKey` (ターゲットが格納されているBlackboardキー)、`rotationSpeed` (回転速度)、`precisionAngle` (完了とみなす角度差閾値)。
*   **`RunBehaviorNode`**: 別のビヘイビアツリーJSON資産を読み込んで「サブツリー」として実行する（再利用に有用）。
    - プロパティ: `treePath` (ビヘイビアツリーJSONファイルのパス)。
*   **`SetBBValueNode`**: Blackboardのキーに指定した値を代入、またはクリアする。
*   **`LogNode`**: デバッグ用のテキストをコンソールに出力する。

### 3.3 標準デコレーター（Decorator / 実行条件判定）
*   **`BlackboardDecorator`**: Blackboardの変数の値を比較（Equal, NotEqual, GreaterThan, LessThan, IsTrue, IsFalse）。
*   **`LogicDecorator`**: AND/OR/NOTなどの論理演算による複数条件。
*   **`CooldownDecorator`**: ノード実行完了後にクールダウン時間を設け、指定秒数が経過するまで再実行を制限する。
    - プロパティ: `cooldownTime` (秒数)。
*   **`LoopDecorator`**: ノードを強制的に指定回数（または無限に）ループ実行させる。
    - プロパティ: `loopCount` (ループ回数)、`infinite` (無限ループフラグ)。
*   **`ForceSuccessDecorator`**: 子ノードが Failure を返したとしても、親には強制的に Success を報告する。

### 3.4 標準サービス（Service / 定期補助処理）
*   **`SimpleEQSService`**: ターゲット周辺の最適な位置（位置と向き）を計算して Blackboard（`MoveToPos` キーなど）に書き込みます。
    - プロパティ: `targetKey` (基準とするターゲット)、`resultPosKey` (結果書き込みキー)、`preferredDistance` (理想の維持距離)、`angleOffset` (正面からの回り込み角度。180度なら背後)。

---

## 4. C#によるカスタムノードの拡張手順

標準ノードでは表現できない、プロジェクト固有のゲームロジック（特定の攻撃スキルの実行など）は、C#側でスクリプトを作成することで簡単に追加・拡張できます。

### 4.1 カスタムアクションノードの作成
`BehaviorNode` を継承し、`Execute` および必要に応じて `OnAbort` を実装します。

```csharp
using System;

// プレイヤーに弾を発射するカスタムアクションの例
public class ShotAtPlayerAction : BehaviorNode
{
	// インスペクターで設定可能なパラメータ
	public float bulletSpeed = 10.0f;
	public string shootJointName = "Muzzle";

	[BlackboardKey]
	public string targetKey = "Target";

	/// <summary>
	/// ノードのメインロジック。
	/// 毎フレーム呼び出され、最終結果(Success/Failure)が出るまで Running を返し続けます。
	/// </summary>
	protected override NodeStatus Execute(Blackboard blackboard, Entity owner)
	{
		// 1. ターゲットの取得
		uint keyHash = BehaviorTreeLoader.HashString(targetKey);
		Entity target = blackboard.GetEntity(keyHash);
		if (target == null) return NodeStatus.Failure;

		// 2. 攻撃インテントを有効化
		var intent = owner.GetComponent<AgentIntentComponent>();
		if (intent != null)
		{
			intent.isAttacking = true;
		}

		// 3. 例：弾の発射処理などを行うスクリプトを取得して呼び出し
		var shooter = owner.GetScript("PlayerShotComponent"); // 敵弾発射スクリプト
		if (shooter != null)
		{
			// 弾を発射させる
			// shooter.Shoot(target.transform.position, bulletSpeed);
		}

		// 攻撃が即座に終わるアクションの場合、Success を返す
		return NodeStatus.Success;
	}

	/// <summary>
	/// ノードが実行中に、デコレーターによる割り込みなどで強制終了（中断）された際に呼ばれます。
	/// </summary>
	public override void OnAbort(Blackboard blackboard, Entity owner)
	{
		// 中断時は攻撃状態を確実にオフにする
		var intent = owner.GetComponent<AgentIntentComponent>();
		if (intent != null)
		{
			intent.isAttacking = false;
		}
	}
}
```

### 4.2 カスタムデコレーターの作成
`BehaviorDecorator` を継承し、`[Decorator]` 属性を付与して `CalculateCondition` をオーバーライドします。

```csharp
using System;

// キャラクターが瀕死状態（HP比率が閾値以下）か判定するデコレーター
[Decorator]
public class CheckHPPercentage : BehaviorDecorator
{
	public float thresholdRatio = 0.3f; // 30%以下

	public override bool CalculateCondition(Blackboard blackboard, Entity owner)
	{
		// EntityにアタッチされているHPスクリプトから情報を取得
		var hpComponent = owner.GetScript("HP") as HP; // 汎用HPスクリプト
		if (hpComponent != null)
		{
			return (hpComponent.currentHp_ / hpComponent.maxHp_) <= thresholdRatio;
		}
		return false;
	}
}
```

### 4.3 カスタムサービスの作成
`BehaviorService` を継承し、`OnTick` を実装します。`Interval` プロパティによって更新頻度を制御できます。

```csharp
using System;

// 定期的に範囲内の標的をスキャンして記録するサービス
public class AutoScanTargetService : BehaviorService
{
	public float scanRadius = 10.0f;

	[BlackboardKey]
	public string targetResultKey = "Target";

	public override void OnTick(Blackboard blackboard, Entity owner)
	{
		var ecsGroup = EntityComponentSystem.GetECSGroup(owner.transform.entity.ecsGroup_.groupName);
		
		// "Player" という名前のエンティティを探す
		Entity player = ecsGroup.FindEntity("Player");
		if (player != null)
		{
			float dist = Vector3.Distance(owner.transform.position, player.transform.position);
			if (dist <= scanRadius)
			{
				// ターゲットをBlackboardに書き込む
				blackboard.SetEntity(BehaviorTreeLoader.HashString(targetResultKey), player);
				return;
			}
		}

		// 範囲外ならクリア
		blackboard.Remove(BehaviorTreeLoader.HashString(targetResultKey));
	}
}
```

---

## 5. AIキャラクターへの適用手順

### STEP 1: AI動作用の変数（パス）をアタッチスクリプトで定義
キャラクター用のスクリプト（例：`Enemy.cs`）に、ビヘイビアツリーのJSONファイルを指定するフィールド（`behaviorTreePath`）を実装します。

```csharp
public class Enemy : MonoScript
{
	// エディタのインスペクターから設定するツリーのパス
	[SerializeField]
	private string behaviorTreePath = "Assets/AI/BossBehavior.json";

	public override void Initialize()
	{
		// 初期設定
	}
}
```

### STEP 2: シーン上でのコンポーネント構成
1.  エディタのシーンウィンドウ上で、AIを適用したいキャラクターのEntityを選択します。
2.  そのEntityに対して、以下の2つをアタッチ（追加）します。
    - **`AgentIntentComponent`** (AIの意思決定をC++とやり取りするための必須コンポーネント)
    - 作成した **C# の `Enemy` スクリプト** (またはボス用のスクリプト)
3.  インスペクター上の `behaviorTreePath` に、作成したツリーJSONのパス（例：`Assets/AI/BossBehavior.json`）を設定します。

### STEP 3: 実行とリアルタイムデバッグ
1.  ゲームを再生（実行）します。
2.  **[AI Behavior Tree]** エディタウィンドウで該当するJSONファイルを開いた状態にすると、現在実行中のキャラクターの処理フローに合わせて**アクティブなノードが緑色の枠で可視化されます**。
3.  エディタ上でBlackboardの変数リストの値が動的に変化していく様子を観察し、AIの思考状況をデバッグできます。
