using System;
using System.Runtime.CompilerServices;

public class BGMPlayer : Component {
	public float volume = 1f;
	public float pitch = 1f;
	public bool loop = true;
	public bool playOnAwake = true;
	public string path = "";

	public void Play() {
		InternalSetParams(nativeHandle, volume, pitch, loop, playOnAwake);
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
		InternalSetParams(nativeHandle, volume, pitch, loop, playOnAwake);
	}

	[MethodImpl(MethodImplOptions.InternalCall)]
	static private extern void InternalGetParams(ulong _nativeHandle, out float _volume, out float _pitch, out bool _loop, out bool _playOnAwake);

	[MethodImpl(MethodImplOptions.InternalCall)]
	static private extern void InternalSetParams(ulong _nativeHandle, float volume, float pitch, bool loop, bool playOnAwake);

	[MethodImpl(MethodImplOptions.InternalCall)]
	static private extern void InternalPlay(ulong _nativeHandle);

	[MethodImpl(MethodImplOptions.InternalCall)]
	static private extern void InternalStop(ulong _nativeHandle);

	public override void SyncFromNative(string ecsGroupName) {
		if (nativeHandle == 0) return;
		InternalGetParams(nativeHandle, out volume, out pitch, out loop, out playOnAwake);
	}
}
