using System;

// 指定した Entity の SEPlayer からワンショットで SE を鳴らす共通処理。
//
// SEPlayer は Entity に1つしか持てず、Play() はコンポーネントに設定された path しか鳴らせない。
// 1つの Entity で複数種類の音を鳴らし分けるため、再生のたびに path を渡せる OneShotPlay を使う。
// path のクリップが読み込まれていなければ engine 側が黙って無視するので、
// 音源ファイルを置くまでは単に鳴らないだけで、エラーにはならない。
//
// MonoScript ではないのでアタッチは不要。音量・ピッチなどのパラメータは
// 呼び出し元の MonoScript が SerializeField で持つ（調整はプレハブ側で行う）。
public static class SEOneShot {

    // entity に SEPlayer が付いていなければ何もしない（音の有無はアタッチで切り分ける）
    public static void Play(Entity entity, string path, float volume, float pitch) {
        if (!entity || String.IsNullOrEmpty(path)) {
            return;
        }

        SEPlayer sePlayer = entity.GetComponent<SEPlayer>();
        if (!sePlayer) {
            return;
        }

        // OneShotPlay は engine 側のリクエストキューに積むだけなので、
        // 衝突コールバックや他スクリプトの Update 中から呼んでも問題ない。
        sePlayer.OneShotPlay(volume, pitch, path);
    }
}
