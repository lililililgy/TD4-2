using System;

// 「このフェーズへ移ってほしい」というワンショットの要求。
//
// コンティニュー（保存地点から再開）もチュートリアルスキップも、やりたいことは
// 「ObjectiveSystem を指定のフェーズへ持っていく」だけで同じなので、窓口を1本に集約する。
// 要求の出どころ（ポーズ画面 / スキップ入力 / デバッグ）が増えても、
// 受け手は ObjectiveSystem のここ1箇所だけを見ればよい。
//
// 要求はフェーズ「名」で持つ。番号だと phaseSequence_ を編集した瞬間に別のフェーズを指す
// （ContinueState が名前で保存しているのと同じ理由）。名前が現在の列に無い場合の倒し方は
// 受け手側の文脈で変わるため、ここでは検証しない
// （開始時は先頭へフォールバック、プレイ中は無視。ObjectiveSystem 側に書いてある）。
//
// static なのは、要求元がシーンやECSグループをまたぐため。
//   - コンティニュー: GameScene の読み込み前（タイトル/ポーズ画面）から要求する
//   - スキップ: GameUIScene のような別 ECSGroup から要求されうる
// エンティティ参照では前者を（相手がまだ居ない）、MessageBus では後者を扱えない
// （別グループのハンドラで書いた値はバッチ同期で潰される）。
//
// ObjectiveSystem は Initialize() と Update() の両方で引く。
// 前者が「開始地点の決定」＝コンティニュー、後者が「プレイ中のジャンプ」＝スキップになる。
public static class PhaseRequest {

    private static string requestedPhaseName_ = "";

    // 指定フェーズへ移るよう要求する。
    // 空文字は「要求なし」として捨てる。保存地点が無いままコンティニューを選んだ場合など、
    // 呼び出し側が毎回「空だったら要求しない」と書かなくて済むようにするため。
    public static void Request(string phaseName) {
        if (string.IsNullOrEmpty(phaseName)) return;
        requestedPhaseName_ = phaseName;
    }

    // 要求を取り出す。読んだら下ろす（ワンショット）。要求が無ければ "" を返す。
    public static string Consume() {
        string phaseName = requestedPhaseName_;
        requestedPhaseName_ = "";
        return phaseName;
    }

    // 溜まっている要求を捨てる。「最初から始める」経路が、
    // 消費されないまま残った古い要求を拾わないようにするために使う。
    public static void Clear() {
        requestedPhaseName_ = "";
    }
}
