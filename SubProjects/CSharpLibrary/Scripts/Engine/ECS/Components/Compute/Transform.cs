using System;
using System.Runtime.InteropServices;
using System.Runtime.CompilerServices;

public class TransformData {
	public Vector3 position;
	public Quaternion rotate;
	public Vector3 scale;
}

public class Transform : Component {
	[StructLayout(LayoutKind.Sequential, Pack = 4)]
	public struct BatchData {
		public uint compId;
		public int enable;
		public Vector3 position;
		public Quaternion rotate;
		public Vector3 scale;
		public Matrix4x4 matrix;
	}

	public Vector3 position = new Vector3(0f, 0f, 0f);
	public Vector3 worldPosition = Vector3.zero;
	public Quaternion rotate = Quaternion.identity;
	public Quaternion rotation { get => rotate; set => rotate = value; }
	public Vector3 scale = Vector3.one;
	public Matrix4x4 matrix = Matrix4x4.kIdentity;

    public Vector3 forward => rotate * Vector3.forward;
    public Vector3 up => rotate * Vector3.up;
    public Vector3 right => rotate * Vector3.right;

	public override void SyncFromNative(string ecsGroupName) {
		if (nativeHandle == 0) return;

		BatchData[] batch = new BatchData[1];
		batch[0].compId = compId;
		ComponentBatchManager.InternalGetBatch(typeof(Transform), batch, 1, ecsGroupName);

		this.enable = batch[0].enable;
		this.position = batch[0].position;
		this.rotate = batch[0].rotate;
		this.scale = batch[0].scale;
		this.matrix = batch[0].matrix;
	}

	/// <summary>
	/// ネイティブへ即座に座標を書き込む。
	/// position への代入は毎フレーム末尾の SendAllBatches でしか C++ に届かないため、
	/// 生成直後のエンティティを即座に配置したい場合はこちらを使う。
	/// C++ 側でワールド行列の再計算と子への伝播まで行われる。
	/// C# 側の position も同時に更新するので、あとから来るバッチ送信と食い違わない。
	/// </summary>
	public void SetPositionImmediate(Vector3 value) {
		position = value;
		if (nativeHandle != 0) {
			InternalSetPosition(nativeHandle, value.x, value.y, value.z);
		}
	}

	/// <summary>
	/// ネイティブのワールド行列から現在のワールド座標を読む。
	/// position は親基準のローカル、matrix はシーン読み込み時のスナップショットなので、
	/// 実行中に正しいワールド座標が要るときはこれを使う。
	/// </summary>
	public Vector3 GetWorldPosition() {
		if (nativeHandle == 0) {
			return position;
		}

		float x, y, z;
		InternalGetPosition(nativeHandle, out x, out y, out z);
		return new Vector3(x, y, z);
	}

	/// =================================
	/// internal methods
	/// =================================

	[MethodImpl(MethodImplOptions.InternalCall)]
	static extern void InternalSetPosition(ulong nativeHandle, float x, float y, float z);

	[MethodImpl(MethodImplOptions.InternalCall)]
	static extern void InternalGetPosition(ulong nativeHandle, out float x, out float y, out float z);
}
