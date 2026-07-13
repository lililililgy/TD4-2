using System;
using System.IO;

public class HotReloadAssetAddTestScript : MonoScript {
	private int frameCount = 0;
	private string tempAssetPath = "Assets/Textures/temp_test_asset.png";
	private bool isAssetCreated = false;

	public override void Initialize() {
		Debug.Log("HotReloadAssetAddTestScript: Initialize");
	}

	public override void Update() {
		frameCount++;

		// 10フレーム目にアセットを新規作成
		if (frameCount == 10 && !isAssetCreated) {
			isAssetCreated = true;
			CreateTempAsset();
		}

		// 120フレーム（約2秒）待って、エンジンがクラッシュせず生き残っていればテスト成功
		if (frameCount >= 120) {
			CleanupTempAsset();
			Debug.Log("=== TEST SUCCESS: HotReload Asset Add Test Passed ===");
		}
	}

	private void CreateTempAsset() {
		Debug.Log("HotReloadAssetAddTestScript: Creating temporary asset at: " + tempAssetPath);
		try {
			// ディレクトリがない場合は作成
			string dir = Path.GetDirectoryName(tempAssetPath);
			if (!string.IsNullOrEmpty(dir) && !Directory.Exists(dir)) {
				Directory.CreateDirectory(dir);
			}

			// 1x1 の最小PNG data
			byte[] dummyPng = new byte[] {
				0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A, 0x00, 0x00, 0x00, 0x0D, 0x49, 0x48, 0x44, 0x52,
				0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x08, 0x02, 0x00, 0x00, 0x00, 0x90, 0x77, 0x53,
				0xDE, 0x00, 0x00, 0x00, 0x0C, 0x49, 0x44, 0x41, 0x54, 0x08, 0xD7, 0x63, 0xF8, 0xCF, 0xC0, 0x00,
				0x00, 0x03, 0x01, 0x01, 0x00, 0x18, 0xDD, 0x8D, 0xB0, 0x00, 0x00, 0x00, 0x00, 0x49, 0x45, 0x4E,
				0x44, 0xAE, 0x42, 0x60, 0x82
			};

			File.WriteAllBytes(tempAssetPath, dummyPng);
			Debug.Log("HotReloadAssetAddTestScript: Asset file written.");
		} catch (Exception e) {
			Debug.LogError("HotReloadAssetAddTestScript: Failed to create temp asset: " + e.Message);
		}
	}

	private void CleanupTempAsset() {
		try {
			if (File.Exists(tempAssetPath)) {
				File.Delete(tempAssetPath);
				Debug.Log("HotReloadAssetAddTestScript: Cleaned up temp asset file.");
			}
			string metaPath = tempAssetPath + ".meta";
			if (File.Exists(metaPath)) {
				File.Delete(metaPath);
				Debug.Log("HotReloadAssetAddTestScript: Cleaned up temp meta file.");
			}
		} catch (Exception e) {
			Debug.LogError("HotReloadAssetAddTestScript: Failed to cleanup: " + e.Message);
		}
	}
}
