using System;

// ボス撃破の目標。MessageBus の EnemyKilledEvent を購読し、撃破された Entity 名が
// ボス名と一致したら完了とする（HP の死亡ポーリングではなくイベント駆動）。
// カウントの定型は CountObjective<EnemyKilledEvent> に集約し、ここでは
// Accept() でボス名フィルタを注入するだけ。targetCount_ は 1 のまま（撃破1回で完了）。
// ボスの HP は Progress() の表示用にだけ参照する（DamageRelay と同じ流儀:
// ecsGroup.FindEntity → GetScript<HP>()）。完了判定はイベントで成立するため、
// フェーズ開始時点でボスが未スポーンでも撃破の検知は可能
// （HP は Progress() 内で遅延再解決するので、スポーン後は進捗表示も追従する）。
public class BossDefeatObjective : CountObjective<EnemyKilledEvent> {

    [SerializeField] private string bossEntityName_ = "KingGeso";

    private HP bossHp_;

    public override void BeginObjective() {
        base.BeginObjective();
        bossHp_ = ResolveBossHp();
    }

    // ボス名が一致した撃破イベントだけカウントする
    protected override bool Accept(EnemyKilledEvent e) {
        return e.entityName == bossEntityName_;
    }

    public override float Progress() {
        if (IsCompleted()) return 1.0f;

        // フェーズ開始時点でボスが未スポーンだった場合に備えた遅延再解決
        if (bossHp_ == null) {
            bossHp_ = ResolveBossHp();
        }

        if (bossHp_ == null) return 0.0f;
        return Mathf.Clamp01(1.0f - bossHp_.CurrentHpRatio());
    }

    private HP ResolveBossHp() {
        Entity bossEntity = ecsGroup.FindEntity(bossEntityName_);
        return bossEntity != null ? bossEntity.GetScript<HP>() : null;
    }
}
