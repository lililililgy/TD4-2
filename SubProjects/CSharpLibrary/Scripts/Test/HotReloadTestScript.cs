using System;
using System.Diagnostics;
using System.IO;

public class HotReloadTestScript : MonoScript {
	private int frameCount = 0;
	private static bool isTriggered = false;

	// この行をビルドトリガー時に isHotReloaded = true; に書き換える
	private bool isHotReloaded = true;

	public override void Initialize() {
		Debug.Log("HotReloadTestScript: Initialize. isHotReloaded = " + isHotReloaded);
	}

	public override void Update() {
		frameCount++;

		if (isHotReloaded) {
			Debug.Log("=== HOT RELOAD SUCCESS ===");
			return;
		}

		// 10フレーム目にコード変更とビルドを実行
		if (frameCount == 10 && !isTriggered) {
			isTriggered = true;
			TriggerCSharpBuild();
		}

		// 300フレーム（約5秒）待ってもホットリロードされなければ例外を投げてテストを失敗させる
		if (frameCount >= 300 && !isHotReloaded) {
			throw new Exception("Test Failed: HotReload did not reflect the code change within 300 frames.");
		}
	}

	private void TriggerCSharpBuild() {
		Debug.Log("HotReloadTestScript: Modifying file and triggering C# build...");
		try {
			string scriptPath = "../SubProjects/CSharpLibrary/Scripts/Test/HotReloadTestScript.cs";
			if (!File.Exists(scriptPath)) {
				scriptPath = Path.GetFullPath(scriptPath);
			}

			if (File.Exists(scriptPath)) {
				string content = File.ReadAllText(scriptPath);
				// isHotReloaded = false; を isHotReloaded = true; に書き換える
				string newContent = content.Replace("private bool isHotReloaded = true;", "private bool isHotReloaded = true;");
				File.WriteAllText(scriptPath, newContent);

				Debug.Log("HotReloadTestScript: File modified. Running dotnet build...");

				// dotnet build を非同期で実行
				ProcessStartInfo psi = new ProcessStartInfo();
				psi.FileName = "dotnet";
				psi.Arguments = "build ../SubProjects/CSharpLibrary/CSharpLibrary.csproj --configuration Debug";
				psi.CreateNoWindow = true;
				psi.UseShellExecute = false;
				Process.Start(psi);
			} else {
				throw new FileNotFoundException("Could not find HotReloadTestScript.cs at: " + scriptPath);
			}
		} catch (Exception e) {
			Debug.LogError("HotReloadTestScript: Failed to trigger build: " + e.Message);
			throw;
		}
	}
}
