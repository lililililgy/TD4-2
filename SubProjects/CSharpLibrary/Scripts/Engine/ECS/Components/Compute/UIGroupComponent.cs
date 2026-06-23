using System;
using System.Runtime.InteropServices;

public class UIGroupComponent : Component {
	[StructLayout(LayoutKind.Sequential)]
	public struct BatchData {
		public uint compId;
		public int currentSelectedId;
		public byte isFocused;
		public byte isVisible;
		public int parentGroupId;
	}

	public int currentSelectedId = 0;
	public bool isFocused = false;
	public bool isVisible = true;
	public int parentGroupId = 0;

	public override void SyncFromNative(string ecsGroupName) {
		if (nativeHandle == 0) return;

		BatchData[] batch = new BatchData[1];
		batch[0].compId = compId;
		ComponentBatchManager.InternalGetBatch(typeof(UIGroupComponent), batch, 1, ecsGroupName);

		this.currentSelectedId = batch[0].currentSelectedId;
		this.isFocused = batch[0].isFocused != 0;
		this.isVisible = batch[0].isVisible != 0;
		this.parentGroupId = batch[0].parentGroupId;
	}
}
