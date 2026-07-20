using System;
using System.Collections.Generic;

// フェーズに応じて、同じエンティティの EnemySpawnSystem の稼働を切り替える。
// EnemySpawnSystem と同じエンティティに付ける（名前で引かずに entity.GetScript で取るため）。
//
// activePhases_ に挙げたフェーズの間だけ湧かせる。フェーズ外・進行終了後は止める。
// これにより EnemySpawnSystem は「湧かせる」ことだけに専念でき、フェーズ進行を知らずに済む。
//
// 購読を Awake() で行っているのは意図的。ObjectiveSystem.Initialize() が最初の
// PhaseBeganEvent を発行するため、Initialize() で購読するとエンティティの並び順次第で
// 初回を取りこぼす（ECSGroup は CallAwake() → CallInitialize() の順に全エンティティを回す）。
public class PhaseSpawnActivator : MonoScript
{

    // この名前のフェーズの間だけ EnemySpawnSystem を稼働させる（例: "Phase_Hunt"）
    [SerializeField] private List<string> activePhases_ = new List<string>();

    private EnemySpawnSystem spawnSystem_;
    private bool subscribed_ = false;

    public override void Initialize()
    {
        if (!subscribed_)
        {
            MessageBus.Subscribe<PhaseBeganEvent>(OnPhaseBegan);
            MessageBus.Subscribe<PhaseEndedEvent>(OnPhaseEnded);
            subscribed_ = true;
        }
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
        SetActive(activePhases_ != null && activePhases_.Contains(e.phaseName));
    }

    // フェーズ離脱時はいったん止める。次のフェーズが対象なら PhaseBeganEvent で再開する。
    // 進行終了（末尾のフェーズ完了）時は PhaseBeganEvent が続かないので、ここで止まったままになる。
    private void OnPhaseEnded(PhaseEndedEvent e)
    {
        SetActive(false);
    }

    private void SetActive(bool active)
    {
        if (spawnSystem_ == null)
        {
            spawnSystem_ = entity.GetScript<EnemySpawnSystem>();
            if (spawnSystem_ == null) return;
        }
        spawnSystem_.IsActive = active;
    }
}
