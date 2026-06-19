using System;
using System.Collections.Generic;
using System.Runtime.CompilerServices;

/// <summary>
/// ビヘイビアツリーノードの実行状態
/// </summary>
public enum NodeStatus {
	Success = 0,
	Failure = 1,
	Running = 2,
	Inactive = 3
}

/// <summary>
/// すべてのビヘイビアツリーノードの基底クラス
/// </summary>
public abstract class BehaviorNode {
	public string id;            // JSON上のユニークID
	public uint runtimeId;       // C++エディタ同期用のID
	public string name;          // ノードの表示名
	public BehaviorTree tree;    // 所属するツリー

	// デコレーターとサービス
	public List<BehaviorDecorator> decorators = new List<BehaviorDecorator>();
	public List<BehaviorService> services = new List<BehaviorService>();

	// 子ノードリスト（CompositeやDecoratorなどで使用）
	public List<BehaviorNode> children = new List<BehaviorNode>();

	/// <summary>
	/// ブレークポイントヒット時にC++エンジン側を一時停止させるブリッジメソッド
	/// </summary>
	[MethodImpl(MethodImplOptions.InternalCall)]
	internal static extern void Internal_OnBreakpointHit(uint nodeIdHash);

	/// <summary>
	/// 初回読み込み時の初期化処理
	/// </summary>
	public virtual void Initialize() {}

	/// <summary>
	/// 毎フレームの更新処理。サービスとデコレーターを実行した後にメインのロジックを呼び出します。
	/// </summary>
	public NodeStatus Tick(float deltaTime) {
		// 1. サービスの実行
		foreach (var service in services) {
			service.Update(deltaTime);
		}

		// 2. デコレーターの評価（いずれか一つでも失敗すればノードを実行せずFailureを返す）
		foreach (var decorator in decorators) {
			if (!decorator.Condition()) {
				UpdateStatus(NodeStatus.Failure);
				return NodeStatus.Failure;
			}
		}

		// 3. メインのノード処理
		NodeStatus status = OnTick(deltaTime);
		UpdateStatus(status);
		return status;
	}

	/// <summary>
	/// 各ノード固有の実行ロジックを実装します。
	/// </summary>
	protected abstract NodeStatus OnTick(float deltaTime);

	/// <summary>
	/// 実行状態を更新し、C++エディタへ通知します。
	/// </summary>
	protected void UpdateStatus(NodeStatus status) {
		if (tree != null) {
			tree.UpdateNodeStatus(runtimeId, status);
		}
	}
}

/// <summary>
/// デコレーターであることを示すクラス属性
/// </summary>
[AttributeUsage(AttributeTargets.Class)]
public class DecoratorAttribute : Attribute {}

/// <summary>
/// フィールドがBlackboardのキーであることを示す属性
/// </summary>
[AttributeUsage(AttributeTargets.Field)]
public class BlackboardKeyAttribute : Attribute {}

/// <summary>
/// 条件判定を行うデコレーターの基底クラス
/// </summary>
public abstract class BehaviorDecorator {
	public string name;
	public BehaviorNode node;

	/// <summary>
	/// デコレーターの判定条件を実装します。
	/// </summary>
	/// <returns>実行を許可する場合はtrue、そうでない場合はfalse</returns>
	public abstract bool Condition();
}

/// <summary>
/// ノードの実行中に毎フレーム呼び出される補助処理（サービス）の基底クラス
/// </summary>
public abstract class BehaviorService {
	public string name;
	public BehaviorNode node;

	/// <summary>
	/// 毎フレームの更新処理を実装します。
	/// </summary>
	public abstract void Update(float deltaTime);
}
