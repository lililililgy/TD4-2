using System;
using System.Runtime.InteropServices;
using System.Runtime.CompilerServices;

public class UIElementComponent : Component {
	[StructLayout(LayoutKind.Sequential)]
	public struct BatchData {
		public uint compId;
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
}
