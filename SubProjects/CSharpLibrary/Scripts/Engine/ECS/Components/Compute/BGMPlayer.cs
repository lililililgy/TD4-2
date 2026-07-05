using System;
using System.Runtime.CompilerServices;

public class BGMPlayer : Component {
	public float volume = 1f;
	public float pitch = 1f;
	public bool loop = true;
	public string path = "";

	public void Play() {
		InternalSetParams(nativeHandle, volume, pitch, loop);
		InternalPlay(nativeHandle);
	}

	public void Stop() {
		InternalStop(nativeHandle);
	}

	public void SetParams(float _volume, float _pitch, bool _loop) {
		volume = _volume;
		pitch = _pitch;
		loop = _loop;
		InternalSetParams(nativeHandle, volume, pitch, loop);
	}

	[MethodImpl(MethodImplOptions.InternalCall)]
	static private extern void InternalGetParams(ulong _nativeHandle, out float _volume, out float _pitch, out bool _loop);

	[MethodImpl(MethodImplOptions.InternalCall)]
	static private extern void InternalSetParams(ulong _nativeHandle, float volume, float pitch, bool loop);

	[MethodImpl(MethodImplOptions.InternalCall)]
	static private extern void InternalPlay(ulong _nativeHandle);

	[MethodImpl(MethodImplOptions.InternalCall)]
	static private extern void InternalStop(ulong _nativeHandle);

	public override void SyncFromNative(string ecsGroupName) {
		if (nativeHandle == 0) return;
		InternalGetParams(nativeHandle, out volume, out pitch, out loop);
	}
}
