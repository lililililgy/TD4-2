using System;

// GameScene への遷移の入口。「どこから始めるか」を決めてから読み込む。
//
// 開始地点そのものは PhaseRequest（フェーズ要求）に載せ、ObjectiveSystem がそれを引く。
// ここの責務は「要求を立てる／消す」＋「シーンを読む」までで、
// フェーズ列の中身や名前の妥当性は知らない。
//
// 再開は「要求が明示的に出ている時だけ」行う特殊な遷移なので、必ずこちらを通す。
// 逆（GameScene の読み込みを既定で再開扱いにする）にすると、
// 普通に開始したいだけの呼び出し元すべてが打ち消しを書く羽目になる。
public static class GameFlow {

    private const string kGameSceneName = "GameScene";

    // 最初から始める。タイトルの「はじめから」やクリア後のリスタートはこちら。
    // 消費されずに残った古いフェーズ要求を明示的に捨ててから読み込む
    // （要求はワンショットなので普通は残らないが、ここが「先頭から」の保証になる）。
    public static void StartNewGame() {
        PhaseRequest.Clear();
        LoadGameScene();
    }

    // コンティニュー地点から再開する。
    // 保存地点が無ければ要求は立たない（PhaseRequest が空文字を捨てる）ので、
    // そのまま先頭フェーズから始まる。呼び出し側でセーブの有無を分岐しなくてよい。
    public static void ResumeFromCheckPoint() {
        PhaseRequest.Request(ContinueState.PhaseName);
        LoadGameScene();
    }

    private static void LoadGameScene() {
        // ポーズ経由で来た場合、GameScene は一時停止フラグが立ったままになっている。
        // シーン名で管理されているフラグなので、読み込み直しても停止したままになりうる。
        SceneManager.SetUpdatePaused(kGameSceneName, false);
        SceneManager.LoadScene(kGameSceneName);
    }
}
