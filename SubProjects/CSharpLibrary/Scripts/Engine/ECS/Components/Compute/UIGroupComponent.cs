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
}
