using System;

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
			// 複数行テスト
			textRenderer.text = "一行目\n二行目です。\n三行目も！";
			System.Console.WriteLine("[TextRendererTest] Changed text to multi-line.");
		}

		if (frameCount == 30) {
			// 空文字テスト
			textRenderer.text = "";
			System.Console.WriteLine("[TextRendererTest] Changed text to empty string.");
		}

		if (frameCount == 40) {
			// 値の整合性チェック
			textRenderer.text = "最終確認";
			textRenderer.fontSize = 28;
			
			if (textRenderer.text != "最終確認") {
				throw new Exception("Failed to set text. Expected '最終確認', got: " + textRenderer.text);
			}

			if (textRenderer.fontSize != 28) {
				throw new Exception("Failed to set fontSize. Expected 28, got: " + textRenderer.fontSize);
			}

			System.Console.WriteLine("[TextRendererTest] Verification successful!");
		}
	}
}
