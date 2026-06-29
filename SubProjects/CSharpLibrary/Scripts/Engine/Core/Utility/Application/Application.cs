using System;
using System.Runtime.CompilerServices;

static public class Application {

	static public void Quit() {
		InternalQuit();
	}

	[MethodImpl(MethodImplOptions.InternalCall)]
	static private extern void InternalQuit();
}
