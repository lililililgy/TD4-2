using System;
using System.Runtime.InteropServices;
using System.Runtime.CompilerServices;

/// <summary>
/// AIの「意図」を格納するコンポーネント
/// C#側で計算され、C++側で行動に変換される
/// </summary>
public class AgentIntentComponent : Component {
    /// <summary>
    /// C++とC#でデータを一括同期するための構造体
    /// </summary>
    [StructLayout(LayoutKind.Sequential)]
    public struct BatchData {
        public uint compId;
        public Vector3 desiredMoveDirection;
        public Quaternion desiredRotation;
        public float rotationSpeed;
        public float maxSpeed;
        public byte useDesiredRotation; // bool interop
        public byte isAttacking; // Use byte for bool interop
        public int targetEntityId;
    }

    public Vector3 desiredMoveDirection = Vector3.zero;
    public Quaternion desiredRotation = Quaternion.identity;
    public float rotationSpeed = 5.0f;
    public float maxSpeed = 8.0f;
    public bool useDesiredRotation = false;
    public bool isAttacking = false;
    public int targetEntityId = 0; // 0 is considered an invalid ID
    public bool isPaused = false; // New: Pause AI update during performances

    /// <summary>
    /// このコンポーネントに関連付けられたビヘイビアツリー
    /// </summary>
    public BehaviorTree behaviorTree;

    /// <summary>
    /// ビヘイビアツリーを初期化する
    /// </summary>
    public void InitBehaviorTree(BehaviorNode root) {
        behaviorTree = new BehaviorTree(this.entity);
        behaviorTree.RootNode = root;
    }

    /// <summary>
    /// JSONファイルからビヘイビアツリーを読み込む
    /// </summary>
    public void LoadBehaviorTree(string path) {
        behaviorTree = BehaviorTreeLoader.LoadFromFile(path, this.entity);
        if (behaviorTree != null) {
        }
    }
}

