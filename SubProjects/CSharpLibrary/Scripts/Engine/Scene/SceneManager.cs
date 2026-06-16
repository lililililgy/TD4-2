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
	
	
	
	[MethodImpl(MethodImplOptions.InternalCall)]
	static private extern void InternalLoadScene(string sceneName);

}
