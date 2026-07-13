using System;
using System.Runtime.InteropServices;
using System.Runtime.CompilerServices;

class TextRenderer : Component {
	[StructLayout(LayoutKind.Sequential)]
	public struct BatchData {
		public uint compId;
		public Vector4 color;
		public Vector2 textureSize;
		public UVTransform uvTransform;
	}

	BatchData batchData;
	public BatchData GetBatchData() {
		return batchData;
	}

	public override void SyncFromNative(string ecsGroupName) {
		if (nativeHandle == 0) return;

		BatchData[] batch = new BatchData[1];
		batch[0].compId = compId;
		ComponentBatchManager.InternalGetBatch(typeof(TextRenderer), batch, 1, ecsGroupName);

		batchData.color = batch[0].color;
		batchData.textureSize = batch[0].textureSize;
		batchData.uvTransform = batch[0].uvTransform;
	}

	private string _text = "";
	public string text {
		get { return _text; }
		set { _text = value; }
	}

	private string _fontPath = "./Assets/Fonts/MPLUSRounded1c-Black.ttf";
	public string fontPath {
		get { return _fontPath; }
		set { _fontPath = value; }
	}

	private int _fontSize = 32;
	public string fontSize_str = ""; // Dummy to bypass compile warnings if any
	public int fontSize {
		get { return _fontSize; }
		set { _fontSize = value; }
	}

	public Vector4 color {
		get {
			return batchData.color;
		}
		set {
			batchData.color = value;
		}
	}

	public UVTransform uvTransform {
		get {
			return batchData.uvTransform;
		}
		set {
			batchData.uvTransform = value;
		}
	}

	public Vector2 textureSize {
		get {
			return batchData.textureSize;
		}
	}

	/// -------------------------------------------
	/// internal methods (Temporarily disabled)
	/// -------------------------------------------

	// [MethodImpl(MethodImplOptions.InternalCall)]
	// static extern Vector4 InternalGetColor(ulong nativeHandle);

	// [MethodImpl(MethodImplOptions.InternalCall)]
	// static extern void InternalSetColor(ulong nativeHandle, Vector4 color);

	// [MethodImpl(MethodImplOptions.InternalCall)]
	// static extern string InternalGetText(ulong nativeHandle);

	// [MethodImpl(MethodImplOptions.InternalCall)]
	// static extern void InternalSetText(ulong nativeHandle, string text);

	// [MethodImpl(MethodImplOptions.InternalCall)]
	// static extern string InternalGetFontPath(ulong nativeHandle);

	// [MethodImpl(MethodImplOptions.InternalCall)]
	// static extern void InternalSetFontPath(ulong nativeHandle, string fontPath);

	// [MethodImpl(MethodImplOptions.InternalCall)]
	// static extern int InternalGetFontSize(ulong nativeHandle);

	// [MethodImpl(MethodImplOptions.InternalCall)]
	// static extern void InternalSetFontSize(ulong nativeHandle, int fontSize);
}
