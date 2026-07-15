using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Runtime.InteropServices;
using System.Runtime.CompilerServices;

class SpriteRenderer : Component {
	[StructLayout(LayoutKind.Explicit, Size = 80)]
	public struct BatchData {
		[FieldOffset(0)] public uint compId;
		[FieldOffset(4)] public Vector4 color;
		[FieldOffset(20)] public Vector2 textureSize;
		[FieldOffset(28)] public uint postEffectFlags;
		[FieldOffset(32)] public UVTransform uvTransform;
		[FieldOffset(64)] public float bloomIntensity;
		[FieldOffset(68)] public float bloomThreshold;
	}

	BatchData batchData;
	public BatchData GetBatchData() {
		return batchData;
	}

	public override void SyncFromNative(string ecsGroupName) {
		if (nativeHandle == 0) return;

		BatchData[] batch = new BatchData[1];
		batch[0].compId = compId;
		ComponentBatchManager.InternalGetBatch(typeof(SpriteRenderer), batch, 1, ecsGroupName);

		batchData.color = batch[0].color;
		batchData.textureSize = batch[0].textureSize;
		batchData.postEffectFlags = batch[0].postEffectFlags;
		batchData.uvTransform = batch[0].uvTransform;
		batchData.bloomIntensity = batch[0].bloomIntensity;
		batchData.bloomThreshold = batch[0].bloomThreshold;
	}

	//public string meshPath {
	//	get {
	//		return InternalGetMeshName(nativeHandle);
	//	}
	//	set {
	//		InternalSetMeshName(nativeHandle, value);
	//	}
	//}

	public Vector4 color {
		get {
			return batchData.color;
		}
		set {
			batchData.color = value;
			// バッチ同期システムが自動でC++側に色を反映するため、値型Vector4のレジスタアライメント崩れを誘発する個別のInternalCallは不要
			//if (nativeHandle != 0) {
			//	InternalSetColor(nativeHandle, value);
			//}
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

	public uint postEffectFlags {
		get {
			return batchData.postEffectFlags;
		}
		set {
			batchData.postEffectFlags = value;
		}
	}

	public float bloomIntensity {
		get {
			return batchData.bloomIntensity;
		}
		set {
			batchData.bloomIntensity = value;
		}
	}

	public float bloomThreshold {
		get {
			return batchData.bloomThreshold;
		}
		set {
			batchData.bloomThreshold = value;
		}
	}

	public bool isPixelPerfect {
		get {
			if (nativeHandle != 0) {
				return InternalGetPixelPerfect(nativeHandle);
			}
			return false;
		}
		set {
			if (nativeHandle != 0) {
				InternalSetPixelPerfect(nativeHandle, value);
			}
		}
	}


	/// -------------------------------------------
	/// internal methods
	/// -------------------------------------------

	//[MethodImpl(MethodImplOptions.InternalCall)]
	//static extern string InternalGetMeshName(ulong nativeHandle);

	//[MethodImpl(MethodImplOptions.InternalCall)]
	//static extern void InternalSetMeshName(ulong nativeHandle, string meshName);



	[MethodImpl(MethodImplOptions.InternalCall)]
	static extern void InternalSetColor(ulong nativeHandle, Vector4 color);

	[MethodImpl(MethodImplOptions.InternalCall)]
	static extern bool InternalGetPixelPerfect(ulong nativeHandle);

	[MethodImpl(MethodImplOptions.InternalCall)]
	static extern void InternalSetPixelPerfect(ulong nativeHandle, bool enable);

}
