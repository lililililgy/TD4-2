using System;
using System.Runtime.InteropServices;
using System.Runtime.CompilerServices;

public class BGMPlayer : Component {
	[StructLayout(LayoutKind.Sequential, Pack = 4)]
	public struct BatchData {
		public uint compId;
		public int enable;
		public float volume;
		public float pitch;
		public int loop;
		public int playOnAwake;
	}

	public float volume = 1f;
	public float pitch = 1f;
	public bool loop = true;
	public bool playOnAwake = true;
	public string path = "";

	public void Play() {
		InternalPlay(nativeHandle);
	}

	public void Stop() {
		InternalStop(nativeHandle);
	}

	public void SetParams(float _volume, float _pitch, bool _loop, bool _playOnAwake) {
		volume = _volume;
		pitch = _pitch;
		loop = _loop;
		playOnAwake = _playOnAwake;
	}

	[MethodImpl(MethodImplOptions.InternalCall)]
	static private extern void InternalPlay(ulong _nativeHandle);

	[MethodImpl(MethodImplOptions.InternalCall)]
	static private extern void InternalStop(ulong _nativeHandle);

	public override void SyncFromNative(string ecsGroupName) {
		if (nativeHandle == 0) return;

		BatchData[] batch = new BatchData[1];
		batch[0].compId = compId;
		ComponentBatchManager.InternalGetBatch(typeof(BGMPlayer), batch, 1, ecsGroupName);

		this.enable = batch[0].enable;
		this.volume = batch[0].volume;
		this.pitch = batch[0].pitch;
		this.loop = batch[0].loop != 0;
		this.playOnAwake = batch[0].playOnAwake != 0;
	}
}
