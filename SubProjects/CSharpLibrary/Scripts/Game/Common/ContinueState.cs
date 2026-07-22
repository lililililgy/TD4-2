using System;
using System.IO;
using Newtonsoft.Json;

// コンティニュー地点の永続化。アプリを再起動しても続きから遊べるよう、
// 到達済みのフェーズ名を JSON でディスクに保存する。
//
// 「地点」はフェーズ名1つだけで表す。敵の湧きもプレイヤーもシーン定義から
// 決定的に再構築されるため、エンティティのスナップショットを持つ必要はない。
//
// index ではなくフェーズ「名」で保存すること。ObjectiveSystem.phaseSequence_ は
// エディタから編集できるため、番号で持つとフェーズを挿入・削除した瞬間に
// 既存のセーブが別のフェーズを指す。さらに範囲外の index は JumpToPhase() が
// 進行終了に倒すので、起動直後にクリア扱いでクリアシーンへ飛ぶ壊れ方をする。
//
// IO 失敗は全て握りつぶして「セーブ無し = 最初から」に倒す。
// セーブの失敗でゲームが落ちるのが一番まずいため。
public static class ContinueState {

    // 実行時のカレントディレクトリは Project/（エンジンが ./Assets/engine_config.json を読む前提）。
    // Assets/ の外に置く。配下に置くとアセット扱いでホットリロードや meta 生成に巻き込まれる。
    private const string kSaveDir = "./SaveData";
    private const string kSavePath = "./SaveData/continue.json";

    // 保存フォーマット。version は将来フィールドを足した際に古いセーブを弾くための逃げ道。
    private class SaveData {
        public int version = kCurrentVersion;
        public string phaseName = "";
    }

    private const int kCurrentVersion = 1;

    private static bool loaded_ = false;
    private static string phaseName_ = "";

    // 保存されているコンティニュー地点のフェーズ名。無ければ ""。
    public static string PhaseName {
        get {
            EnsureLoaded();
            return phaseName_;
        }
    }

    // コンティニュー可能か（メニュー項目の出し分けに使う）
    public static bool HasPoint {
        get { return !string.IsNullOrEmpty(PhaseName); }
    }

    // フェーズ突入時に呼ぶ。同じフェーズなら書き込みを省く（1プレイ数回だが無駄は避ける）。
    public static void Save(string phaseName) {
        EnsureLoaded();

        if (string.IsNullOrEmpty(phaseName)) return;
        if (phaseName_ == phaseName) return;

        phaseName_ = phaseName;
        WriteToDisk();
    }

    // セーブを破棄する。「最初から」を選んだ時とゲームクリア時に呼ぶ。
    public static void Clear() {
        phaseName_ = "";
        loaded_ = true; // 消した直後にディスクを読み直さない

        try {
            if (File.Exists(kSavePath)) {
                File.Delete(kSavePath);
            }
        } catch (Exception e) {
            Console.WriteLine("[error] ContinueState.Clear: " + e.Message);
        }
    }

    // 起動後の初回アクセス時に1回だけディスクを読む。以降は static にキャッシュする。
    private static void EnsureLoaded() {
        if (loaded_) return;
        loaded_ = true;

        try {
            if (!File.Exists(kSavePath)) return;

            SaveData data = JsonConvert.DeserializeObject<SaveData>(File.ReadAllText(kSavePath));
            if (data == null || data.version != kCurrentVersion) return;

            phaseName_ = data.phaseName != null ? data.phaseName : "";
        } catch (Exception e) {
            // 壊れたセーブは無かったことにする（最初から始まる）
            Console.WriteLine("[error] ContinueState.EnsureLoaded: " + e.Message);
            phaseName_ = "";
        }
    }

    private static void WriteToDisk() {
        try {
            if (!Directory.Exists(kSaveDir)) {
                Directory.CreateDirectory(kSaveDir);
            }

            SaveData data = new SaveData();
            data.phaseName = phaseName_;
            File.WriteAllText(kSavePath, JsonConvert.SerializeObject(data, Formatting.Indented));
        } catch (Exception e) {
            Console.WriteLine("[error] ContinueState.WriteToDisk: " + e.Message);
        }
    }
}
