using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

public class UIGroupComponent : Component {
	[StructLayout(LayoutKind.Sequential, Pack = 4)]
	public struct BatchData {
		public uint compId;
		public int enable;
		public int currentSelectedId;
		public byte isFocused;
		public byte isVisible;
		public byte padding1;
		public byte padding2;
		public int parentGroupId;
	}

	public int currentSelectedId = 0;

	private bool isFocused_ = false;
	public bool isFocused {
		get => isFocused_;
		set {
			if (isFocused_ == value) return;
			isFocused_ = value;
			if (nativeHandle != 0) {
				InternalSetFocused(compId, value, entity.ecsGroupName);
			}
		}
	}

	private bool isVisible_ = true;
	public bool isVisible {
		get => isVisible_;
		set {
			if (isVisible_ == value) return;
			isVisible_ = value;
			if (nativeHandle != 0) {
				InternalSetVisible(compId, value, entity.ecsGroupName);
			}
		}
	}

	public int parentGroupId = 0;

	public override void SyncFromNative(string ecsGroupName) {
		if (nativeHandle == 0) return;

		BatchData[] batch = new BatchData[1];
		batch[0].compId = compId;
		ComponentBatchManager.InternalGetBatch(typeof(UIGroupComponent), batch, 1, ecsGroupName);

		this.enable = batch[0].enable;
		this.currentSelectedId = batch[0].currentSelectedId;
		this.isFocused_ = batch[0].isFocused != 0;
		this.isVisible_ = batch[0].isVisible != 0;
		this.parentGroupId = batch[0].parentGroupId;
	}

	[MethodImpl(MethodImplOptions.InternalCall)]
	private static extern void InternalSetVisible(uint compId, bool isVisible, string ecsGroupName);

	[MethodImpl(MethodImplOptions.InternalCall)]
	private static extern void InternalSetFocused(uint compId, bool isFocused, string ecsGroupName);
}
