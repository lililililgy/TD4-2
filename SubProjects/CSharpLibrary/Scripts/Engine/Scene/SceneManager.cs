using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Runtime.CompilerServices;

static public class SceneManager {
	static public string sceneName_;

	static public void LoadScene(string sceneName) {
		InternalLoadScene(sceneName);
	}

	static public void AddScene(string sceneName) {
		InternalAddScene(sceneName);
	}

	static public void Add(string sceneName) {
		InternalAddScene(sceneName);
	}

	static public void UnloadScene(string sceneName) {
		InternalUnloadScene(sceneName);
	}

	static public void Unload(string sceneName) {
		InternalUnloadScene(sceneName);
	}
	
	static public void SetUpdatePaused(string sceneName, bool paused) {
		InternalSetUpdatePaused(sceneName, paused);
	}

	static public bool IsUpdatePaused(string sceneName) {
		return InternalIsUpdatePaused(sceneName);
	}

	static public void SetDrawPaused(string sceneName, bool paused) {
		InternalSetDrawPaused(sceneName, paused);
	}

	static public bool IsDrawPaused(string sceneName) {
		return InternalIsDrawPaused(sceneName);
	}
	
	
	[MethodImpl(MethodImplOptions.InternalCall)]
	static private extern void InternalLoadScene(string sceneName);

	[MethodImpl(MethodImplOptions.InternalCall)]
	static private extern void InternalAddScene(string sceneName);

	[MethodImpl(MethodImplOptions.InternalCall)]
	static private extern void InternalUnloadScene(string sceneName);

	[MethodImpl(MethodImplOptions.InternalCall)]
	static private extern void InternalSetUpdatePaused(string sceneName, bool paused);

	[MethodImpl(MethodImplOptions.InternalCall)]
	static private extern bool InternalIsUpdatePaused(string sceneName);

	[MethodImpl(MethodImplOptions.InternalCall)]
	static private extern void InternalSetDrawPaused(string sceneName, bool paused);

	[MethodImpl(MethodImplOptions.InternalCall)]
	static private extern bool InternalIsDrawPaused(string sceneName);

}
