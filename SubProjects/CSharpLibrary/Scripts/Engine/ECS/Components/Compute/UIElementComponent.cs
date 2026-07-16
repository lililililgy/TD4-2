using System;
using System.Runtime.InteropServices;
using System.Runtime.CompilerServices;

public class UIElementComponent : Component {
	[StructLayout(LayoutKind.Sequential, Pack = 4)]
	public struct BatchData {
		public uint compId;
		public int enable;
		public int groupIdId;
		public int elementIndex;
	}

	public int groupIdId = 0;
	public int elementIndex = 0;

	public string elementId {
		get {
			IntPtr ptr = InternalGetElementId(compId, entity.ecsGroupName);
			if (ptr == IntPtr.Zero) return "";
			return Marshal.PtrToStringAnsi(ptr);
		}
	}

	[MethodImpl(MethodImplOptions.InternalCall)]
	private static extern IntPtr InternalGetElementId(uint compId, string groupName);

	public override void SyncFromNative(string ecsGroupName) {
		if (nativeHandle == 0) return;

		BatchData[] batch = new BatchData[1];
		batch[0].compId = compId;
		ComponentBatchManager.InternalGetBatch(typeof(UIElementComponent), batch, 1, ecsGroupName);

		this.enable = batch[0].enable;
		this.groupIdId = batch[0].groupIdId;
		this.elementIndex = batch[0].elementIndex;
	}
}
