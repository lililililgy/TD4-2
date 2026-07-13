using System.Runtime.CompilerServices;

public static class FontRasterizer {
	public static bool GenerateTexture(string text, string fontAssetPath, int fontSize, string texturePath) {
		return Internal_GenerateTexture(text, fontAssetPath, fontSize, texturePath);
	}

	[MethodImpl(MethodImplOptions.InternalCall)]
	private static extern bool Internal_GenerateTexture(string text, string fontAssetPath, int fontSize, string texturePath);
}
