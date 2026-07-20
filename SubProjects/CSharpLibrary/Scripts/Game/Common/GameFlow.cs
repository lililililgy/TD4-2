using System;

// コンティニュー専用のシーン遷移の入口。
//
// 通常のゲーム開始は今までどおり SceneManager.LoadScene("GameScene") でよい。
// 再開は「要求が明示的に出ている時だけ」行う特殊な遷移なので、こちらを通す。
// 逆（GameScene の読み込みを既定で再開扱いにする）にすると、
// 普通に開始したいだけの呼び出し元すべてが打ち消しを書く羽目になる。
//
// GameOver 画面から繋ぐ場合も、UI スクリプトから ResumeFromCheckPoint() を呼ぶだけでよい。
public static class GameFlow {

    private const string kGameSceneName = "GameScene";

    // 次に読み込む GameScene を再開扱いにするワンショットの要求。
    // ディスクには保存しない（保存地点そのものは ContinueState が持つ）。
    private static bool resumeRequested_ = false;

    // コンティニュー地点から再開する。
    // 保存地点が無い場合は先頭フェーズから始まる（ObjectiveSystem 側でフォールバックする）。
    public static void ResumeFromCheckPoint() {
        resumeRequested_ = true;

        // ポーズ経由で来た場合、GameScene は一時停止フラグが立ったままになっている。
        // シーン名で管理されているフラグなので、読み込み直しても停止したままになりうる。
        SceneManager.SetUpdatePaused(kGameSceneName, false);
        SceneManager.LoadScene(kGameSceneName);
    }

    // ObjectiveSystem.Initialize() が1回だけ引く。読んだら下ろす。
    // 立っていなければ通常の開始として先頭フェーズから始まる。
    public static bool ConsumeResumeRequest() {
        bool requested = resumeRequested_;
        resumeRequested_ = false;
        return requested;
    }
}
