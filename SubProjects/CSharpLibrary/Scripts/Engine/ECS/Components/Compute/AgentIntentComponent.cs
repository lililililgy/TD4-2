using System.Runtime.InteropServices;

[StructLayout(LayoutKind.Sequential)]
public class AgentIntentComponent : Component {
	[StructLayout(LayoutKind.Sequential)]
	public struct BatchData {
		public uint compId;
		public Vector3 desiredMoveDirection;
		public Quaternion desiredRotation;
		public float rotationSpeed;
		public float maxSpeed;
		public byte useDesiredRotation;
		public byte isAttacking;
		private byte pad1, pad2; // Alignment for targetEntityId
		public int targetEntityId;
	}

	public Vector3 desiredMoveDirection = Vector3.zero;
	public Quaternion desiredRotation = Quaternion.identity;
	public float rotationSpeed = 10.0f;
	public float maxSpeed = 5.0f;
	public bool useDesiredRotation = true;
	public bool isAttacking = false;
	public int targetEntityId = -1;

	public override void SyncFromNative(string ecsGroupName) {
		if (nativeHandle == 0) return;

		BatchData[] batch = new BatchData[1];
		batch[0].compId = compId;
		ComponentBatchManager.InternalGetBatch(typeof(AgentIntentComponent), batch, 1, ecsGroupName);

		desiredMoveDirection = batch[0].desiredMoveDirection;
		desiredRotation = batch[0].desiredRotation;
		rotationSpeed = batch[0].rotationSpeed;
		maxSpeed = batch[0].maxSpeed;
		useDesiredRotation = batch[0].useDesiredRotation != 0;
		isAttacking = batch[0].isAttacking != 0;
		targetEntityId = batch[0].targetEntityId;
	}
}
