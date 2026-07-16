using System;
using System.Runtime.InteropServices;
using System.Runtime.CompilerServices;

public class SEPlayer : Component {
	[StructLayout(LayoutKind.Sequential, Pack = 4)]
	public struct BatchData {
		public uint compId;
		public int enable;
		public float volume;
		public float pitch;
	}

	public float volume = 1f;
	public float pitch = 1f;
	public string path = "";

	public void Play() {
		InternalPlay(nativeHandle);
	}

	public void Stop() {
		InternalStop(nativeHandle);
	}

	public void SetParams(float _volume, float _pitch) {
		volume = _volume;
		pitch = _pitch;
	}

	public void OneShotPlay(float _volume, float _pitch, string _path) {
		InternalPlayOneShot(nativeHandle, _volume, _pitch, _path);
	}

	[MethodImpl(MethodImplOptions.InternalCall)]
	static private extern void InternalPlay(ulong _nativeHandle);

	[MethodImpl(MethodImplOptions.InternalCall)]
	static private extern void InternalStop(ulong _nativeHandle);

	[MethodImpl(MethodImplOptions.InternalCall)]
	static private extern void InternalPlayOneShot(ulong _nativeHandle, float _volume, float _pitch, string _path);

	public override void SyncFromNative(string ecsGroupName) {
		if (nativeHandle == 0) return;

		BatchData[] batch = new BatchData[1];
		batch[0].compId = compId;
		ComponentBatchManager.InternalGetBatch(typeof(SEPlayer), batch, 1, ecsGroupName);

		this.enable = batch[0].enable;
		this.volume = batch[0].volume;
		this.pitch = batch[0].pitch;
	}
}
