using System;
using System.Runtime.InteropServices;
using System.Runtime.CompilerServices;

public enum HorizontalAlignment {
	Left = 0,
	Center = 1,
	Right = 2
}

public enum VerticalAlignment {
	Top = 0,
	Middle = 1,
	Bottom = 2
}

class TextRenderer : Component {
	[StructLayout(LayoutKind.Sequential, Pack = 4)]
	public struct BatchData {
		public uint compId;
		public Vector4 color;
		public Vector2 textureSize;
		public UVTransform uvTransform;
		public int horizontalAlignment;
		public int verticalAlignment;
		public Vector4 outlineColor;
		public int outlineWidth;
		public Vector4 shadowColor;
		public Vector2 shadowOffset;
		public int characterSpacing;
		public float lineSpacing;
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
		batchData.horizontalAlignment = batch[0].horizontalAlignment;
		batchData.verticalAlignment = batch[0].verticalAlignment;
		batchData.outlineColor = batch[0].outlineColor;
		batchData.outlineWidth = batch[0].outlineWidth;
		batchData.shadowColor = batch[0].shadowColor;
		batchData.shadowOffset = batch[0].shadowOffset;
		batchData.characterSpacing = batch[0].characterSpacing;
		batchData.lineSpacing = batch[0].lineSpacing;
	}

	public string text {
		get {
			if (nativeHandle != 0) {
				return InternalGetText(nativeHandle);
			}
			return "";
		}
		set {
			if (nativeHandle != 0) {
				InternalSetText(nativeHandle, value);
			}
		}
	}

	public string fontPath {
		get {
			if (nativeHandle != 0) {
				return InternalGetFontPath(nativeHandle);
			}
			return "";
		}
		set {
			if (nativeHandle != 0) {
				InternalSetFontPath(nativeHandle, value);
			}
		}
	}

	public int fontSize {
		get {
			if (nativeHandle != 0) {
				return InternalGetFontSize(nativeHandle);
			}
			return 0;
		}
		set {
			if (nativeHandle != 0) {
				InternalSetFontSize(nativeHandle, value);
			}
		}
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

	public HorizontalAlignment horizontalAlignment {
		get {
			if (nativeHandle != 0) {
				return (HorizontalAlignment)InternalGetHorizontalAlignment(nativeHandle);
			}
			return HorizontalAlignment.Left;
		}
		set {
			if (nativeHandle != 0) {
				InternalSetHorizontalAlignment(nativeHandle, (int)value);
			}
		}
	}

	public VerticalAlignment verticalAlignment {
		get {
			if (nativeHandle != 0) {
				return (VerticalAlignment)InternalGetVerticalAlignment(nativeHandle);
			}
			return VerticalAlignment.Top;
		}
		set {
			if (nativeHandle != 0) {
				InternalSetVerticalAlignment(nativeHandle, (int)value);
			}
		}
	}

	public Vector4 outlineColor {
		get {
			if (nativeHandle != 0) {
				return InternalGetOutlineColor(nativeHandle);
			}
			return new Vector4(0f, 0f, 0f, 1f);
		}
		set {
			if (nativeHandle != 0) {
				InternalSetOutlineColor(nativeHandle, value);
			}
		}
	}

	public int outlineWidth {
		get {
			if (nativeHandle != 0) {
				return InternalGetOutlineWidth(nativeHandle);
			}
			return 0;
		}
		set {
			if (nativeHandle != 0) {
				InternalSetOutlineWidth(nativeHandle, value);
			}
		}
	}

	public Vector4 shadowColor {
		get {
			if (nativeHandle != 0) {
				return InternalGetShadowColor(nativeHandle);
			}
			return new Vector4(0f, 0f, 0f, 0f);
		}
		set {
			if (nativeHandle != 0) {
				InternalSetShadowColor(nativeHandle, value);
			}
		}
	}

	public Vector2 shadowOffset {
		get {
			if (nativeHandle != 0) {
				return InternalGetShadowOffset(nativeHandle);
			}
			return new Vector2(2f, -2f);
		}
		set {
			if (nativeHandle != 0) {
				InternalSetShadowOffset(nativeHandle, value);
			}
		}
	}

	public int characterSpacing {
		get {
			if (nativeHandle != 0) {
				return InternalGetCharacterSpacing(nativeHandle);
			}
			return 0;
		}
		set {
			if (nativeHandle != 0) {
				InternalSetCharacterSpacing(nativeHandle, value);
			}
		}
	}

	public float lineSpacing {
		get {
			if (nativeHandle != 0) {
				return InternalGetLineSpacing(nativeHandle);
			}
			return 1.0f;
		}
		set {
			if (nativeHandle != 0) {
				InternalSetLineSpacing(nativeHandle, value);
			}
		}
	}

	/// -------------------------------------------
	/// internal methods
	/// -------------------------------------------

	[MethodImpl(MethodImplOptions.InternalCall)]
	static extern string InternalGetText(ulong nativeHandle);

	[MethodImpl(MethodImplOptions.InternalCall)]
	static extern void InternalSetText(ulong nativeHandle, string text);

	[MethodImpl(MethodImplOptions.InternalCall)]
	static extern string InternalGetFontPath(ulong nativeHandle);

	[MethodImpl(MethodImplOptions.InternalCall)]
	static extern void InternalSetFontPath(ulong nativeHandle, string fontPath);

	[MethodImpl(MethodImplOptions.InternalCall)]
	static extern int InternalGetFontSize(ulong nativeHandle);

	[MethodImpl(MethodImplOptions.InternalCall)]
	static extern void InternalSetFontSize(ulong nativeHandle, int fontSize);

	[MethodImpl(MethodImplOptions.InternalCall)]
	static extern int InternalGetHorizontalAlignment(ulong nativeHandle);

	[MethodImpl(MethodImplOptions.InternalCall)]
	static extern void InternalSetHorizontalAlignment(ulong nativeHandle, int alignment);

	[MethodImpl(MethodImplOptions.InternalCall)]
	static extern int InternalGetVerticalAlignment(ulong nativeHandle);

	[MethodImpl(MethodImplOptions.InternalCall)]
	static extern void InternalSetVerticalAlignment(ulong nativeHandle, int alignment);

	[MethodImpl(MethodImplOptions.InternalCall)]
	static extern Vector4 InternalGetOutlineColor(ulong nativeHandle);

	[MethodImpl(MethodImplOptions.InternalCall)]
	static extern void InternalSetOutlineColor(ulong nativeHandle, Vector4 color);

	[MethodImpl(MethodImplOptions.InternalCall)]
	static extern int InternalGetOutlineWidth(ulong nativeHandle);

	[MethodImpl(MethodImplOptions.InternalCall)]
	static extern void InternalSetOutlineWidth(ulong nativeHandle, int width);

	[MethodImpl(MethodImplOptions.InternalCall)]
	static extern Vector4 InternalGetShadowColor(ulong nativeHandle);

	[MethodImpl(MethodImplOptions.InternalCall)]
	static extern void InternalSetShadowColor(ulong nativeHandle, Vector4 color);

	[MethodImpl(MethodImplOptions.InternalCall)]
	static extern Vector2 InternalGetShadowOffset(ulong nativeHandle);

	[MethodImpl(MethodImplOptions.InternalCall)]
	static extern void InternalSetShadowOffset(ulong nativeHandle, Vector2 offset);

	[MethodImpl(MethodImplOptions.InternalCall)]
	static extern int InternalGetCharacterSpacing(ulong nativeHandle);

	[MethodImpl(MethodImplOptions.InternalCall)]
	static extern void InternalSetCharacterSpacing(ulong nativeHandle, int spacing);

	[MethodImpl(MethodImplOptions.InternalCall)]
	static extern float InternalGetLineSpacing(ulong nativeHandle);

	[MethodImpl(MethodImplOptions.InternalCall)]
	static extern void InternalSetLineSpacing(ulong nativeHandle, float spacing);
}
