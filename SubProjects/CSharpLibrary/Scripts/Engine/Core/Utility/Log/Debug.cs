using System.Runtime.CompilerServices;

static public class Debug {
	public static bool IgnoreLog = false;

	static public void Log(string message) {
		if (IgnoreLog) return;
#if DEBUG
		InternalConsoleLog("[script] " + message);
#endif
	}

	static public void LogInfo(string message) {
		if (IgnoreLog) return;
		Log("[info] " + message);
	}

	static public void LogWarning(string message) {
		if (IgnoreLog) return;
		Log("[warning] " + message);
	}

	static public void LogError(string message) {
		if (IgnoreLog) return;
		Log("[error] " + message);
	}


	[MethodImpl(MethodImplOptions.InternalCall)]
	static extern void InternalConsoleLog(string s);
}