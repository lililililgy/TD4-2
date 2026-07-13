using System;
using System.Runtime.InteropServices;

public class TextRendererTestScript : MonoScript {
	private int frameCount = 0;
	private TextRenderer textRenderer;

	public override void Initialize() {
		System.Console.WriteLine("[TextRendererTest] Initializing test...");
		
		// TextRenderer をアタッチ
		textRenderer = entity.AddComponent<TextRenderer>();
		if (textRenderer == null) {
			throw new Exception("Failed to add TextRenderer component.");
		}

		// 初期パラメータを設定
		textRenderer.fontPath = "./Assets/Fonts/MPLUSRounded1c-Black.ttf";
		textRenderer.fontSize = 24;
		textRenderer.text = "Hello ONEngine Text!";
		textRenderer.color = new Vector4(1f, 1f, 1f, 1f);

		System.Console.WriteLine("[TextRendererTest] Setup done. Text: " + textRenderer.text);
	}

	public override void Update() {
		frameCount++;

		if (frameCount == 10) {
			// テキストの動的変更テスト
			textRenderer.text = "動的テキスト変更テスト！";
			textRenderer.fontSize = 32;
			System.Console.WriteLine("[TextRendererTest] Changed text to: " + textRenderer.text + " Size: " + textRenderer.fontSize);
		}

		if (frameCount == 20) {
			// アライメントの変更テスト
			textRenderer.text = "アライメントテスト\n中央揃えです。\n右揃えもテスト！";
			textRenderer.horizontalAlignment = HorizontalAlignment.Center;
			textRenderer.verticalAlignment = VerticalAlignment.Middle;
			System.Console.WriteLine("[TextRendererTest] Changed text alignment to Center/Middle.");
		}

		if (frameCount == 25) {
			textRenderer.horizontalAlignment = HorizontalAlignment.Right;
			System.Console.WriteLine("[TextRendererTest] Changed text alignment to Right.");
		}

		if (frameCount == 30) {
			// 空文字テスト
			textRenderer.text = "";
			System.Console.WriteLine("[TextRendererTest] Changed text to empty string.");
		}

		if (frameCount == 35) {
			// アウトライン（フチ取り）の設定テスト
			textRenderer.text = "フチ取りテキスト！";
			textRenderer.outlineColor = new Vector4(1f, 0f, 0f, 1f); // 赤フチ
			textRenderer.outlineWidth = 3;
			System.Console.WriteLine("[TextRendererTest] Set outline (Red, width: 3)");
		}

		if (frameCount == 40) {
			// シャドウの設定テスト
			textRenderer.shadowColor = new Vector4(0f, 0f, 0f, 0.5f); // 半透明黒影
			textRenderer.shadowOffset = new Vector2(4f, -4f);
			System.Console.WriteLine("[TextRendererTest] Set shadow (Semi-transparent black, offset: 4, -4)");
		}

		if (frameCount == 50) {
			// 値の整合性チェック
			if (textRenderer.text != "フチ取りテキスト！") {
				throw new Exception("Failed to set text. Expected 'フチ取りテキスト！', got: " + textRenderer.text);
			}

			if (textRenderer.horizontalAlignment != HorizontalAlignment.Right) {
				throw new Exception("HorizontalAlignment mismatch. Expected Right, got: " + textRenderer.horizontalAlignment);
			}

			if (textRenderer.verticalAlignment != VerticalAlignment.Middle) {
				throw new Exception("VerticalAlignment mismatch. Expected Middle, got: " + textRenderer.verticalAlignment);
			}

			if (Math.Abs(textRenderer.outlineColor.x - 1f) > 0.001f || Math.Abs(textRenderer.outlineColor.y - 0f) > 0.001f) {
				throw new Exception("OutlineColor mismatch. Expected Red, got: " + textRenderer.outlineColor);
			}

			if (textRenderer.outlineWidth != 3) {
				throw new Exception("OutlineWidth mismatch. Expected 3, got: " + textRenderer.outlineWidth);
			}

			if (Math.Abs(textRenderer.shadowColor.w - 0.5f) > 0.001f) {
				throw new Exception("ShadowColor alpha mismatch. Expected 0.5, got: " + textRenderer.shadowColor.w);
			}

			if (Math.Abs(textRenderer.shadowOffset.x - 4f) > 0.001f || Math.Abs(textRenderer.shadowOffset.y - (-4f)) > 0.001f) {
				throw new Exception("ShadowOffset mismatch. Expected (4, -4), got: " + textRenderer.shadowOffset);
			}

			System.Console.WriteLine("[TextRendererTest] Verification successful!");
		}
	}
}
