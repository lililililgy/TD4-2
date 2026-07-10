using System;

public class SpriteRendererCrashTest : MonoScript {
	private int frameCount = 0;

	public override void Initialize() {
		System.Console.WriteLine("[SpriteRendererCrashTest] Starting verification...");

		// 自身にアタッチされているはずの SpriteRenderer を取得
		SpriteRenderer sr = entity.GetComponent<SpriteRenderer>();
		if (sr == null) {
			throw new Exception("SpriteRenderer component not found on test entity.");
		}

		// textureSize を呼び出す。
		// このシーンに配置された Entity は、アセットコレクションに存在しない無効なテクスチャ GUID が指定されているため、
		// 修正前はここで境界外アクセスが発生しクラッシュしていました。
		// 修正後は安全に Vector2.zero (0, 0) が返ってくるはずです。
		Vector2 size = sr.textureSize;
		System.Console.WriteLine("[SpriteRendererCrashTest] Retrieved texture size: " + size.x + "x" + size.y);

		// クラッシュせずに (0,0) を返すことを確認
		if (size.x != 0.0f || size.y != 0.0f) {
			throw new Exception("Expected texture size to be (0,0) for invalid texture GUID, but got: " + size.x + "x" + size.y);
		}

		System.Console.WriteLine("[SpriteRendererCrashTest] Verification successful. No crash detected.");
	}

	public override void Update() {
		frameCount++;
		if (frameCount > 10) {
			// テストを正常終了させるために一定フレーム経過したら何もしない
		}
	}
}
