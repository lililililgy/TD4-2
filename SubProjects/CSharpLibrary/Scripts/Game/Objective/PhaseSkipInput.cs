using System;
using System.Collections.Generic;

// スキップ入力を受け取り、指定したフェーズ群をまとめて飛ばす。
// 既定値はチュートリアルの4フェーズなので、そのまま置けば
// 「チュートリアル中にスキップキーを押し続けるとチュートリアルが飛ぶ」になる。
// フェーズ名を差し替えれば、演出フェーズ等のスキップにもそのまま使える。
//
// PlayerInputComponent と同じく、毎フレーム入力を引くだけのスクリプト。
// 割り当ては [SerializeField] のキー/パッドのリストで差し替えられる（判定は InputUtil に共通化）。
//
// スキップの実体は PhaseRequest への要求で、コンティニューと同じ経路を通る。
// ObjectiveSystem を直接叩かないのは、要求元がスキップ入力・ポーズ画面・デバッグと増えても
// 受け口を1つに保つためと、GameUIScene のような別 ECSGroup に置き換えてもそのまま動くようにするため。
//
// スキップ先は「skipPhases_ に載っていない最初のフェーズ」を列の前方向に探して決める。
// 飛び先の名前を別項目で持たせると、フェーズ名を変えた時に2箇所直すことになり、
// 片方だけ直した状態（＝飛び先が見つからず無反応）を作りやすいため、対象リスト1つを設定の正にしている。
public class PhaseSkipInput : MonoScript, IGaugeSource {

    // スキップ対象のフェーズ名。この中にいる間だけスキップ入力を受け付ける。
    [SerializeField] private List<string> skipPhases_ = new List<string> {
        "TutorialPhase_Move",
        "TutorialPhase_Dash",
        "TutorialPhase_Enemy",
        "TutorialPhase_Shot",
    };

    // スキップのキー割り当て
    [SerializeField] private List<KeyCode> skipKeys_ = new List<KeyCode> { KeyCode.Tab };
    [SerializeField] private List<Gamepad> skipButtons_ = new List<Gamepad> { Gamepad.Back };

    // 押しっぱなしで確定するまでの時間[秒]。0 なら押した瞬間に飛ぶ。
    // 既定で長押しにしているのは、スキップが戻せない操作のため
    // （誤爆でチュートリアルが飛んでも、プレイヤーには元に戻す手段がない）。
    [SerializeField] private float holdSeconds_ = 0.6f;

    // ObjectiveSystem が付いているエンティティ名。空文字なら自分と同じエンティティを見る。
    [SerializeField] private string objectiveSystemEntityName_ = "GameController";

    private ObjectiveSystem objectiveSystem_;
    private float heldSeconds_ = 0.0f;

    public override void Initialize() {
        objectiveSystem_ = ResolveObjectiveSystem();
    }

    public override void Update() {
        // 相手のエンティティがまだ生成されていなかった場合に備えた遅延再解決
        if (objectiveSystem_ == null) {
            objectiveSystem_ = ResolveObjectiveSystem();
            if (objectiveSystem_ == null) return;
        }

        // スキップできないフェーズに入った時点で長押しは無かったことにする
        // （対象外フェーズで押し続けた時間が、次にスキップ対象へ入った瞬間に効くのを防ぐ）。
        if (!IsSkippablePhase() || !InputUtil.AnyPressed(skipKeys_, skipButtons_)) {
            heldSeconds_ = 0.0f;
            return;
        }

        heldSeconds_ += Time.deltaTime;
        if (heldSeconds_ < holdSeconds_) return;

        // 押しっぱなしのまま次のスキップ対象へ連続で飛ばないよう、押し直しを要求する
        heldSeconds_ = 0.0f;
        RequestSkip();
    }

    // 長押しの進捗(0.0〜1.0)。「離すと戻るゲージ」の表示用に UI から引く想定。
    // 長押し無し設定(holdSeconds_ <= 0)では溜める時間が無いので常に 0 を返す。
    public float HoldProgress {
        get {
            if (holdSeconds_ <= 0.0f) return 0.0f;
            return Mathf.Clamp01(heldSeconds_ / holdSeconds_);
        }
    }

    // スキップゲージ(SpriteGauge)が毎フレーム引く割合。
    // 押している間だけ伸び、離す・対象外フェーズへ移ると Update() で heldSeconds_ が
    // 0 に戻るので、ゲージも即座に空へ戻る。
    public float GetGaugeRatio() {
        return HoldProgress;
    }

    // 今スキップ入力を受け付けているか。「スキップできます」の表示出し分け用。
    public bool IsSkippable {
        get { return objectiveSystem_ != null && IsSkippablePhase(); }
    }

    // IsPhaseBegan を見るのは、フェーズエンティティの解決待ち中
    // （目標がまだ動き出していない状態）にスキップさせないため。
    // 進行終了後は CurrentPhaseName が "" になるので、そのまま受け付けなくなる。
    private bool IsSkippablePhase() {
        return objectiveSystem_.IsPhaseBegan &&
               skipPhases_ != null &&
               skipPhases_.Contains(objectiveSystem_.CurrentPhaseName);
    }

    private void RequestSkip() {
        string targetPhaseName = ResolveSkipTargetName();
        if (string.IsNullOrEmpty(targetPhaseName)) return;

        PhaseRequest.Request(targetPhaseName);
    }

    // スキップ先を決める。現在位置から前方向に、skipPhases_ に載っていない最初のフェーズを探す。
    // 末尾までスキップ対象しか無い場合は "" を返して何もしない
    // （スキップ操作でゲームがクリア扱いになるのが一番まずいので、無反応に倒す）。
    private string ResolveSkipTargetName() {
        int phaseCount = objectiveSystem_.PhaseCount;

        for (int i = objectiveSystem_.CurrentPhaseIndex + 1; i < phaseCount; i++) {
            string phaseName = objectiveSystem_.PhaseNameAt(i);
            if (!skipPhases_.Contains(phaseName)) return phaseName;
        }

        return "";
    }

    private ObjectiveSystem ResolveObjectiveSystem() {
        if (string.IsNullOrEmpty(objectiveSystemEntityName_)) {
            return entity.GetScript<ObjectiveSystem>();
        }

        Entity systemEntity = ecsGroup.FindEntity(objectiveSystemEntityName_);
        return systemEntity != null ? systemEntity.GetScript<ObjectiveSystem>() : null;
    }
}
