

/// @brief ポストエフェクトの適用
public enum PostEffectFlags : uint {
	None                  = 0,      ///< なし
	Lighting              = 1 << 0, ///< ライティング
	Grayscale             = 1 << 1, ///< グレースケール
	EnvironmentReflection = 1 << 2, ///< 天球に合わせて環境反射を行う
	Shadow                = 1 << 3, ///< 影
	Bloom                 = 1 << 4, ///< ブルーム
	Outline               = 1 << 5, ///< アウトライン
	RadialBlur            = 1 << 6, ///< ラジアルブラー
};