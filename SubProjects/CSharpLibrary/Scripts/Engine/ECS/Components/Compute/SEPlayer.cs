using System;
using System.Runtime.CompilerServices;

public class SEPlayer : Component {
	public float volume = 1f;
	public float pitch = 1f;
	public string path = "";

	public void Play() {
		InternalSetParams(nativeHandle, volume, pitch);
		InternalPlay(nativeHandle);
	}

	public void Stop() {
		InternalStop(nativeHandle);
	}

	public void SetParams(float _volume, float _pitch) {
		volume = _volume;
		pitch = _pitch;
		InternalSetParams(nativeHandle, volume, pitch);
	}

	public void OneShotPlay(float _volume, float _pitch, string _path) {
		InternalPlayOneShot(nativeHandle, _volume, _pitch, _path);
	}

	[MethodImpl(MethodImplOptions.InternalCall)]
	static private extern void InternalGetParams(ulong _nativeHandle, out float _volume, out float _pitch);

	[MethodImpl(MethodImplOptions.InternalCall)]
	static private extern void InternalSetParams(ulong _nativeHandle, float volume, float pitch);

	[MethodImpl(MethodImplOptions.InternalCall)]
	static private extern void InternalPlay(ulong _nativeHandle);

	[MethodImpl(MethodImplOptions.InternalCall)]
	static private extern void InternalStop(ulong _nativeHandle);

	[MethodImpl(MethodImplOptions.InternalCall)]
	static private extern void InternalPlayOneShot(ulong _nativeHandle, float _volume, float _pitch, string _path);

	public override void SyncFromNative(string ecsGroupName) {
		if (nativeHandle == 0) return;
		InternalGetParams(nativeHandle, out volume, out pitch);
	}
}
