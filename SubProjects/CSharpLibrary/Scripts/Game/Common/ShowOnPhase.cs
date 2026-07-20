using System;
using System.Collections.Generic;

// 指定したフェーズの間だけ、同じエンティティの SpriteRenderer / TextRenderer を表示する。
// フェーズ外・進行終了後は隠す。両方持っている必要はなく、付いている方だけを切り替える。
//
// 表示切り替えは Component.enable(0/1) で行う。ComponentBatchManager が
// batch[i].enable = comp.enable として native に送るため、これが表示を制御する実体
// （BatchData 側にも enable があるが、そちらは native からの読み取り用）。
//
// Awake() で「まず隠す」ところまでやるのは意図的。エディタ側で enable=false にし忘れても
// 契約（= visiblePhases_ の間だけ見える）が守られる。
// ECSGroup は CallAwake() → CallInitialize() の順に全エンティティを回すため、
// ObjectiveSystem.Initialize() が発行する最初の PhaseBeganEvent より前に必ず隠し終わる。
// これを Initialize() でやるとエンティティの並び順次第で、
// 「イベントで表示 → あとから初期化で非表示」と取り消してしまう。
public class ShowOnPhase : MonoScript
{

    // この名前のフェーズの間だけ表示する（例: "Phase_Tutorial"）。
    // 複数フェーズにまたがって出しっぱなしにもできる。
    [SerializeField] private List<string> visiblePhases_ = new List<string>();

    private bool subscribed_ = false;

    public override void Initialize()
    {
        if (!subscribed_)
        {
            MessageBus.Subscribe<PhaseBeganEvent>(OnPhaseBegan);
            MessageBus.Subscribe<PhaseEndedEvent>(OnPhaseEnded);
            subscribed_ = true;
        }

        SetVisible(false);
    }

    public override void OnDestroy()
    {
        if (subscribed_)
        {
            MessageBus.Unsubscribe<PhaseBeganEvent>(OnPhaseBegan);
            MessageBus.Unsubscribe<PhaseEndedEvent>(OnPhaseEnded);
            subscribed_ = false;
        }
    }

    private void OnPhaseBegan(PhaseBeganEvent e)
    {
        SetVisible(visiblePhases_ != null && visiblePhases_.Contains(e.phaseName));
    }

    // フェーズ離脱時はいったん隠す。次のフェーズが対象なら PhaseBeganEvent で出し直す。
    // 進行終了（末尾のフェーズ完了）時は PhaseBeganEvent が続かないので、隠れたままになる。
    private void OnPhaseEnded(PhaseEndedEvent e)
    {
        SetVisible(false);
    }

    private void SetVisible(bool visible)
    {
        int enable = visible ? 1 : 0;

        SpriteRenderer sprite = entity.GetComponent<SpriteRenderer>();
        if (sprite != null)
        {
            sprite.enable = enable;
        }

        TextRenderer text = entity.GetComponent<TextRenderer>();
        if (text != null)
        {
            text.enable = enable;
        }
    }
}
