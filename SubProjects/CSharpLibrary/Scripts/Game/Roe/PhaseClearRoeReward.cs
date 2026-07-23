using System;

// フェーズをクリアするたびに、成熟済みのたまご(＝すぐ撃てる弾)を1つ与える。
// RoeManager と同じエンティティ（＝プレイヤー）に付ける。
//
// 卵は経験値でしか育たないので、敵が湧かないフェーズでは待っても弾が増えない。
// 進むたびに必ず1発ぶん増やすことで、「撃て」系の目標(ShotObjective)で弾が無くて詰むのを防ぐ。
//
// 「クリアした」の検出は、次のフェーズが始まったこと（CurrentPhaseName の変化）で行う。
// シーン開始・コンティニュー時の最初のフェーズは前のフェーズが無いので付与しない。
// スキップ(PhaseSkipInput)や JumpToPhase での移動も「進んだ」とみなして付与する
// （チュートリアルを飛ばした人が弾無しで本編に入らないようにするため）。
//
// PhaseBeganEvent は購読せず、Update() で ObjectiveSystem を毎フレーム引く（pull 型）。
// ObjectiveSystem は別エンティティ(GameController)にいて、その Initialize() が最初の
// PhaseBeganEvent を発行するため、購読方式ではエンティティの並び順次第で最初のフェーズを
// 取りこぼしうる。毎フレーム引けば取りこぼしようがない。
public class PhaseClearRoeReward : MonoScript
{

    // ObjectiveSystem が付いているエンティティ名。空文字なら自分と同じエンティティを見る。
    [SerializeField] private string objectiveSystemEntityName_ = "GameController";

    private RoeManager roeManager_;
    private ObjectiveSystem objectiveSystem_;

    // 最後に観測したフェーズ名。これが変わった＝前のフェーズを終えて次が始まった。
    // 空文字始まりなので、最初に始まったフェーズは「前のフェーズ無し」として付与されない。
    private string lastPhaseName_ = "";

    public override void Initialize()
    {
        roeManager_ = entity.GetScript<RoeManager>();
        objectiveSystem_ = ResolveObjectiveSystem();
        lastPhaseName_ = "";
    }

    public override void Update()
    {
        // 相手のエンティティがまだ生成されていなかった場合に備えた遅延再解決
        if (objectiveSystem_ == null)
        {
            objectiveSystem_ = ResolveObjectiveSystem();
            if (objectiveSystem_ == null) return;
        }

        // IsPhaseBegan を見るのは、フェーズエンティティの解決待ち中
        // （目標がまだ動き出していない状態）を「始まった」と誤認しないため。
        if (!objectiveSystem_.IsPhaseBegan) return;

        string phaseName = objectiveSystem_.CurrentPhaseName;
        if (phaseName == lastPhaseName_) return;

        bool hadPreviousPhase = !string.IsNullOrEmpty(lastPhaseName_);
        lastPhaseName_ = phaseName;
        if (!hadPreviousPhase) return;

        GrantReward();
    }

    private void GrantReward()
    {
        if (roeManager_ == null)
        {
            roeManager_ = entity.GetScript<RoeManager>();
            if (roeManager_ == null) return;
        }

        roeManager_.GrantMatureEgg();
    }

    private ObjectiveSystem ResolveObjectiveSystem()
    {
        if (string.IsNullOrEmpty(objectiveSystemEntityName_))
        {
            return entity.GetScript<ObjectiveSystem>();
        }

        Entity systemEntity = ecsGroup.FindEntity(objectiveSystemEntityName_);
        return systemEntity != null ? systemEntity.GetScript<ObjectiveSystem>() : null;
    }
}
