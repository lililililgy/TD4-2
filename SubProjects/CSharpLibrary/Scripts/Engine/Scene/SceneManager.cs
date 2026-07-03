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
	
	
	
	[MethodImpl(MethodImplOptions.InternalCall)]
	static private extern void InternalLoadScene(string sceneName);

	[MethodImpl(MethodImplOptions.InternalCall)]
	static private extern void InternalAddScene(string sceneName);

	[MethodImpl(MethodImplOptions.InternalCall)]
	static private extern void InternalUnloadScene(string sceneName);

}
