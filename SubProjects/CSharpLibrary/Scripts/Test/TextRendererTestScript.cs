using System;
using System.Runtime.InteropServices;

public class TextRendererTestScript : MonoScript {
	private int frameCount = 0;
	private TextRenderer textRenderer1;
	private TextRenderer textRenderer2;
	private Entity entity2;

	public override void Initialize() {
		System.Console.WriteLine("[TextRendererTest] Initializing test...");
		
		// TextRenderer1 をアタッチ
		textRenderer1 = entity.AddComponent<TextRenderer>();
		if (textRenderer1 == null) {
			throw new Exception("Failed to add TextRenderer1 component.");
		}

		// 初期パラメータを設定
		textRenderer1.fontPath = "./Assets/Fonts/MPLUSRounded1c-Black.ttf";
		textRenderer1.fontSize = 24;
		textRenderer1.text = "Hello ONEngine Text 1!";
		textRenderer1.color = new Vector4(1f, 1f, 1f, 1f);

		// TextRenderer2 用の別 Entity を生成し、TextRenderer2 をアタッチ
		entity2 = ecsGroup.CreateEntity("");
		textRenderer2 = entity2.AddComponent<TextRenderer>();
		if (textRenderer2 == null) {
			throw new Exception("Failed to add TextRenderer2 component.");
		}

		textRenderer2.fontPath = "./Assets/Fonts/MPLUSRounded1c-Black.ttf";
		textRenderer2.fontSize = 36;
		textRenderer2.text = "Hello ONEngine Text 2!";
		textRenderer2.color = new Vector4(0f, 1f, 0f, 1f); // 緑

		System.Console.WriteLine("[TextRendererTest] Setup done. Text1: " + textRenderer1.text + ", Text2: " + textRenderer2.text);
	}

	public override void Update() {
		frameCount++;

		if (frameCount == 10) {
			// テキストの動的変更テスト
			textRenderer1.text = "動的テキスト変更テスト！";
			textRenderer1.fontSize = 32;
			System.Console.WriteLine("[TextRendererTest] Changed text1 to: " + textRenderer1.text + " Size: " + textRenderer1.fontSize);
		}

		if (frameCount == 20) {
			// アライメントの変更テスト
			textRenderer1.text = "アライメントテスト\n中央揃えです。\n右揃えもテスト！";
			textRenderer1.horizontalAlignment = HorizontalAlignment.Center;
			textRenderer1.verticalAlignment = VerticalAlignment.Middle;
			System.Console.WriteLine("[TextRendererTest] Changed text1 alignment to Center/Middle.");
		}

		if (frameCount == 25) {
			textRenderer1.horizontalAlignment = HorizontalAlignment.Right;
			System.Console.WriteLine("[TextRendererTest] Changed text1 alignment to Right.");
		}

		if (frameCount == 30) {
			// 空文字テスト
			textRenderer1.text = "";
			System.Console.WriteLine("[TextRendererTest] Changed text1 to empty string.");
		}

		if (frameCount == 35) {
			// アウトライン（フチ取り）の設定テスト
			textRenderer1.text = "フチ取りテキスト！";
			textRenderer1.outlineColor = new Vector4(1f, 0f, 0f, 1f); // 赤フチ
			textRenderer1.outlineWidth = 3;
			System.Console.WriteLine("[TextRendererTest] Set outline for text1 (Red, width: 3)");
		}

		if (frameCount == 40) {
			// シャドウの設定テスト
			textRenderer1.shadowColor = new Vector4(0f, 0f, 0f, 0.5f); // 半透明黒影
			textRenderer1.shadowOffset = new Vector2(4f, -4f);
			System.Console.WriteLine("[TextRendererTest] Set shadow for text1 (Semi-transparent black, offset: 4, -4)");
		}

		if (frameCount == 50) {
			// 値の整合性チェック (TextRenderer1)
			if (textRenderer1.text != "フチ取りテキスト！") {
				throw new Exception("textRenderer1 text mismatch. Expected 'フチ取りテキスト！', got: " + textRenderer1.text);
			}

			if (textRenderer1.horizontalAlignment != HorizontalAlignment.Right) {
				throw new Exception("textRenderer1 HorizontalAlignment mismatch. Expected Right, got: " + textRenderer1.horizontalAlignment);
			}

			if (textRenderer1.verticalAlignment != VerticalAlignment.Middle) {
				throw new Exception("textRenderer1 VerticalAlignment mismatch. Expected Middle, got: " + textRenderer1.verticalAlignment);
			}

			if (Math.Abs(textRenderer1.outlineColor.x - 1f) > 0.001f || Math.Abs(textRenderer1.outlineColor.y - 0f) > 0.001f) {
				throw new Exception("textRenderer1 OutlineColor mismatch. Expected Red, got: " + textRenderer1.outlineColor);
			}

			if (textRenderer1.outlineWidth != 3) {
				throw new Exception("textRenderer1 OutlineWidth mismatch. Expected 3, got: " + textRenderer1.outlineWidth);
			}

			if (Math.Abs(textRenderer1.shadowColor.w - 0.5f) > 0.001f) {
				throw new Exception("textRenderer1 ShadowColor alpha mismatch. Expected 0.5, got: " + textRenderer1.shadowColor.w);
			}

			if (Math.Abs(textRenderer1.shadowOffset.x - 4f) > 0.001f || Math.Abs(textRenderer1.shadowOffset.y - (-4f)) > 0.001f) {
				throw new Exception("textRenderer1 ShadowOffset mismatch. Expected (4, -4), got: " + textRenderer1.shadowOffset);
			}

			// 値の整合性チェック (TextRenderer2 が textRenderer1 の設定値で汚染・上書きされていないこと)
			if (textRenderer2.text != "Hello ONEngine Text 2!") {
				throw new Exception("textRenderer2 text mismatch (might have been overwritten by textRenderer1). Got: " + textRenderer2.text);
			}

			if (textRenderer2.fontSize != 36) {
				throw new Exception("textRenderer2 fontSize mismatch. Expected 36, got: " + textRenderer2.fontSize);
			}

			if (textRenderer2.horizontalAlignment != HorizontalAlignment.Left) {
				throw new Exception("textRenderer2 horizontalAlignment mismatch. Expected Left, got: " + textRenderer2.horizontalAlignment);
			}

			System.Console.WriteLine("[TextRendererTest] Multi-instance verification successful!");
		}
	}
}
