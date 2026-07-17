using System;

// 現在アクティブな目標の進捗を TextRenderer に「目標名 : 進捗」形式で出す UI スクリプト。
// TextRenderer が付いた UI エンティティに付ける。
//
// 目標は ObjectiveSystem.ActiveObjectives（収集済みの List<Objective>）から取る。
// フェーズエンティティを直接引かないので、
// (1) 目標の具象型（KillCountObjective 等）を一切知らずに済み、
// (2) フェーズ送りに自動追従する（Phase_Tutorial → Phase_Hunt → … で表示が勝手に切り替わる）。
// 表示文字列は Objective.ProgressText() が各目標自身の流儀で返す。
// 1フェーズに複数の目標（AND 条件）が積まれていれば、その分だけ行が増える。
public class ObjectiveProgressForText : MonoScript
{

    // ObjectiveSystem が付いているエンティティ名
    [SerializeField] private string objectiveSystemEntityName_ = "GameController";

    // ObjectiveSystem がいるシーン(ECSグループ)名。
    // このUIを GameUIScene のような別シーンに置く場合は、ecsGroup.FindEntity では
    // 自分のグループしか探せないため、ここに探索先のシーン名を入れる。
    // 空文字なら自分と同じグループを探す。
    [SerializeField] private string objectiveSystemSceneName_ = "GameScene";

    // 進行終了後（全フェーズ達成）に出す文字列
    [SerializeField] private string finishedText_ = "";

    private ObjectiveSystem objectiveSystem_;
    private TextRenderer textRenderer_;

    public override void Initialize()
    {
        textRenderer_ = entity.GetComponent<TextRenderer>();
        objectiveSystem_ = ResolveObjectiveSystem();
    }

    public override void Update()
    {
        if (textRenderer_ == null) return;

        // ObjectiveSystem 側のエンティティがまだ生成されていなかった場合に備えた遅延再解決
        if (objectiveSystem_ == null)
        {
            objectiveSystem_ = ResolveObjectiveSystem();
            if (objectiveSystem_ == null) return;
        }

        textRenderer_.text = BuildText();
    }

    private string BuildText()
    {
        if (objectiveSystem_.IsFinished) return finishedText_;

        string result = "";
        foreach (Objective objective in objectiveSystem_.ActiveObjectives)
        {
            if (objective == null) continue;

            if (result.Length > 0) result += "\n";
            result += objective.DisplayName + " : " + objective.ProgressText();
        }
        return result;
    }

    private ObjectiveSystem ResolveObjectiveSystem()
    {
        ECSGroup group = ecsGroup;
        if (!string.IsNullOrEmpty(objectiveSystemSceneName_))
        {
            group = EntityComponentSystem.GetECSGroup(objectiveSystemSceneName_);
            if (!group) return null; // シーンがまだ読まれていない
        }

        Entity systemEntity = group.FindEntity(objectiveSystemEntityName_);
        return systemEntity != null ? systemEntity.GetScript<ObjectiveSystem>() : null;
    }
}
