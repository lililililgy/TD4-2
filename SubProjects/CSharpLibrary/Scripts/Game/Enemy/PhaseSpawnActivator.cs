using System;
using System.Collections.Generic;

// フェーズに応じて EnemySpawnSystem の稼働を切り替える。
//
// activePhases_ に挙げたフェーズの間だけ湧かせる。フェーズ外・進行終了後は止める。
// これにより EnemySpawnSystem は「湧かせる」ことだけに専念でき、フェーズ進行を知らずに済む。
//
// EnemySpawnSystem は別エンティティ（既定では "EnemySpawnSystem"）にいるので、
// entity.GetScript ではなく enemySpawnSystemEntityName_ で名前引きする。
// 空文字にすると自分と同じエンティティから取る。
//
// PhaseBeganEvent は購読せず、Update() で ObjectiveSystem を毎フレーム引く（pull 型）。
// ObjectiveSystem は同じエンティティにいて、その Initialize() が最初の PhaseBeganEvent を
// 発行する。Entity.GetScripts() は Dictionary 列挙で順序が保証されないため、
// こちらが購読するより先に発行されうる = 最初のフェーズを取りこぼしうる。
// 毎フレーム引けば取りこぼしようがなく、JumpToPhase による任意フェーズ移動にも追従する。
public class PhaseSpawnActivator : MonoScript
{

    // この名前のフェーズの間だけ EnemySpawnSystem を稼働させる（例: "EnemyHuntPhase_1"）
    [SerializeField] private List<string> activePhases_ = new List<string>();

    // EnemySpawnSystem が付いているエンティティ名。空文字なら自分と同じエンティティを見る。
    [SerializeField] private string enemySpawnSystemEntityName_ = "EnemySpawnSystem";

    // ObjectiveSystem が付いているエンティティ名
    [SerializeField] private string objectiveSystemEntityName_ = "GameController";

    private EnemySpawnSystem spawnSystem_;
    private ObjectiveSystem objectiveSystem_;

    public override void Initialize()
    {
        spawnSystem_ = ResolveSpawnSystem();
        objectiveSystem_ = ResolveObjectiveSystem();
        SetActive(false);
    }

    public override void Update()
    {
        // 相手のエンティティがまだ生成されていなかった場合に備えた遅延再解決
        if (objectiveSystem_ == null)
        {
            objectiveSystem_ = ResolveObjectiveSystem();
            if (objectiveSystem_ == null)
            {
                SetActive(false);
                return;
            }
        }

        // IsPhaseBegan を見るのは、フェーズエンティティの解決待ち中
        // （目標がまだ動き出していない状態）で湧かせないため。
        // 進行終了後は CurrentPhaseName が "" になるので、そのまま止まる。
        SetActive(objectiveSystem_.IsPhaseBegan &&
                  activePhases_ != null &&
                  activePhases_.Contains(objectiveSystem_.CurrentPhaseName));
    }

    private void SetActive(bool active)
    {
        if (spawnSystem_ == null)
        {
            spawnSystem_ = ResolveSpawnSystem();
            if (spawnSystem_ == null) return;
        }
        spawnSystem_.IsActive = active;
    }

    private EnemySpawnSystem ResolveSpawnSystem()
    {
        if (string.IsNullOrEmpty(enemySpawnSystemEntityName_))
        {
            return entity.GetScript<EnemySpawnSystem>();
        }

        Entity spawnEntity = ecsGroup.FindEntity(enemySpawnSystemEntityName_);
        return spawnEntity != null ? spawnEntity.GetScript<EnemySpawnSystem>() : null;
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
