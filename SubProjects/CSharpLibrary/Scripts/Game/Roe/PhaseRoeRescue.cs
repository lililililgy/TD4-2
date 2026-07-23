using System;

// フェーズ開始時に「たまごを撃てない」状態なら、幼生たまご(未成熟の卵)を与えて詰みを防ぐ救済。
// RoeManager と同じエンティティ（＝プレイヤー）に付ける。
//
// 詰みが起きるのは卵が1個しか残っていないとき。RoeManager.TryConsumeMature() は
// 発射での即ゲームオーバーを防ぐため最後の1卵を撃たせないので、成熟卵を持っていても発射できず、
// 「射撃せよ」系の目標(ShotObjective)が永久に達成できなくなる。
// 幼生たまごを1つ足して2個にすれば、手持ちの成熟卵をそのまま撃てるようになる。
//
// 卵が minEggCount_ 個以上あるのに撃てない場合（成熟卵が0）は何もしない。
// この状態は経験値で卵が育てば解消するし、幼生たまごを足しても撃てるようにはならないため。
//
// PhaseBeganEvent は購読せず、Update() で ObjectiveSystem を毎フレーム引く（pull 型）。
// ObjectiveSystem は別エンティティ(GameController)にいて、その Initialize() が最初の
// PhaseBeganEvent を発行するため、購読方式ではエンティティの並び順次第で最初のフェーズを
// 取りこぼしうる。毎フレーム引けば取りこぼしようがなく、JumpToPhase による任意フェーズ移動
// （チュートリアルスキップ・コンティニュー）にも追従する。
public class PhaseRoeRescue : MonoScript
{

    // フェーズ開始時に最低限そろえる卵の数。TryConsumeMature() が最後の1卵を残すため、
    // 発射できるようにするには2個必要。
    [SerializeField] private int minEggCount_ = 2;

    // ObjectiveSystem が付いているエンティティ名。空文字なら自分と同じエンティティを見る。
    [SerializeField] private string objectiveSystemEntityName_ = "GameController";

    private RoeManager roeManager_;
    private ObjectiveSystem objectiveSystem_;

    // 最後に救済判定を行ったフェーズ名。これが変わった瞬間が「フェーズ開始時」。
    // 空文字始まりなので、最初に始まったフェーズもちゃんと1回判定される。
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
        // （目標がまだ動き出していない状態）で数えないため。
        if (!objectiveSystem_.IsPhaseBegan) return;

        string phaseName = objectiveSystem_.CurrentPhaseName;
        if (phaseName == lastPhaseName_) return;

        lastPhaseName_ = phaseName;
        RescueIfCannotShoot();
    }

    // 撃てないなら幼生たまごを足す。撃てるならそのまま。
    private void RescueIfCannotShoot()
    {
        if (roeManager_ == null)
        {
            roeManager_ = entity.GetScript<RoeManager>();
            if (roeManager_ == null) return;
        }

        if (roeManager_.CanShoot()) return;

        roeManager_.EnsureEggCount(minEggCount_);
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
