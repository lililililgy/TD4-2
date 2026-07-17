using System;

// 「レベルnに到達」の目標。対象エンティティの LevelingComponent の現在レベルを
// 毎フレームのポーリング（ObjectiveSystem が呼ぶ IsCompleted()）で調べる方式。
// イベント購読ではないので、フェーズ開始前に既に達成済みのケースもそのまま拾える。
// 対象は targetEntityName_ で指定する（LevelingComponent は Player 以外に Roe 等にも付くため）。
// LevelingComponent の解決は BeginObjective() で行い、未解決なら判定時に遅延再解決する
// （BossDefeatObjective の bossHp_ と同じ流儀）。
public class LevelReachObjective : Objective
{
    [SerializeField] private int targetLevel_ = 2;

    // 監視対象の Entity 名
    [SerializeField] private string targetEntityName_ = "Player";

    private LevelingComponent leveling_;

    public override void BeginObjective()
    {
        leveling_ = ResolveLeveling();
    }

    public override bool IsCompleted()
    {
        // 対象がフェーズ開始時点で未生成だった場合に備えた遅延再解決
        if (leveling_ == null)
        {
            leveling_ = ResolveLeveling();
        }

        if (leveling_ == null) return false;
        return leveling_.CurrentLevel >= targetLevel_;
    }

    public override float Progress()
    {
        if (targetLevel_ <= 0) return 1.0f;
        if (leveling_ == null) return 0.0f;
        return Mathf.Clamp01((float)leveling_.CurrentLevel / targetLevel_);
    }

    // 「1 / 2」形式。IsCompleted() と同様、未解決なら遅延再解決する
    public override string ProgressText()
    {
        if (leveling_ == null)
        {
            leveling_ = ResolveLeveling();
        }

        int current = leveling_ != null ? leveling_.CurrentLevel : 0;
        return current + " / " + targetLevel_;
    }

    private LevelingComponent ResolveLeveling()
    {
        Entity target = ecsGroup.FindEntity(targetEntityName_);
        return target != null ? target.GetScript<LevelingComponent>() : null;
    }
}
