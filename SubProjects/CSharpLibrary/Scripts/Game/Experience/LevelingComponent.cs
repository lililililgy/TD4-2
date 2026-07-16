using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

public class LevelingComponent : MonoScript {
    [SerializeField] private int maxLevel_ = 0;
    [SerializeField] private float baseRequiredExp_ = 100f;

    [SerializeField] private int currentLevel_;
    [SerializeField] private float currentExp_;
    private float addedExp_; // このフレームに追加された経験値（毎フレーム先頭でリセット）
    private float totalGainedExp_; // 累計で取得した経験値（リセットしない。実行順に依存しない増分取得用）
    private float requiredExp_; // 次のLevelに必要な経験値

    private bool isLevelUp_ = false; // Levelが上がったかどうかのフラグ

    // 取得した経験値オブジェクトの破棄予定リスト。OnCollisionEnter で積み、Update で破棄する。
    // 同じ相手が二重に入って二重 Destroy しないよう Id で重複排除する。
    private readonly HashSet<int> pendingDestroyIds_ = new HashSet<int>();

    public override void Initialize() {
        currentLevel_ = 0;
        currentExp_ = 0f;
        addedExp_ = 0f;
        totalGainedExp_ = 0f;
        pendingDestroyIds_.Clear();
        requiredExp_ = CalculateRequiredExp(currentLevel_);
    }
    public override void Update() {
        isLevelUp_ = false; // フラグをリセット
        addedExp_ = 0;

        if (currentExp_ >= requiredExp_) {
            LevelUp();
        }

        // Id 経由で取り直し、既に破棄済み(null)なら飛ばす。二重 Destroy を防ぐ。
        foreach (int id in pendingDestroyIds_) {
            Entity coll = ecsGroup.GetEntity(id);
            if (coll == null || coll.Id == entity.Id) {
                continue;
            }
            coll.Destroy(); // 経験値オブジェクトを破棄
        }
        pendingDestroyIds_.Clear(); // 破棄予定リストをクリア
    }

    /// <summary>
    /// LevelUp処理を行うメソッド
    /// </summary>
    private void LevelUp() {
        isLevelUp_ = true;
        currentLevel_++;

        // レベル上昇の確定を通知する（「レベルnに到達」目標用に新レベル値を運ぶ）。
        // LevelUp() は1回の呼び出しで1レベルだけ上げるので、上がったレベルごとに1回発行される。
        MessageBus.Publish(new LevelUpEvent(entity.name, currentLevel_));

        float excessExp = currentExp_ - requiredExp_; // 残りの経験値を計算
        currentExp_ = excessExp;

        requiredExp_ = CalculateRequiredExp(currentLevel_); // 次のLevelに必要な経験値を計算
    }

    /// <summary>
    /// 次のLevelに必要な経験値を計算するメソッド
    /// </summary>
    /// <param name="level"></param>
    /// <returns></returns>
    private float CalculateRequiredExp(int level) {
        return baseRequiredExp_; // 一旦固定
    }

    public override void OnCollisionEnter(Entity collision) {
        if (collision == null || collision.Id <= 0 || collision.Id == entity.Id) {
            return;
        }

        // 衝突通知までに相手が破棄されている可能性があるため、現在のECSから引き直す。
        int collisionId = collision.Id;
        Entity experienceEntity = ecsGroup.GetEntity(collisionId);
        if (experienceEntity == null) {
            return;
        }

        ExperiencePoint exp = experienceEntity.GetScript<ExperiencePoint>();
        if (exp == null) {
            return;
        }
        currentExp_ += exp.ExperiencePoints;
        addedExp_ += exp.ExperiencePoints;
        totalGainedExp_ += exp.ExperiencePoints;

        pendingDestroyIds_.Add(collisionId);// 経験値オブジェクトを破棄予定リストに追加
    }

    public void AddExperience(float exp) {
        currentExp_ += exp;
        addedExp_ += exp;
        totalGainedExp_ += exp;
    }

    public int MaxLevel {
        get { return maxLevel_; }
    }
    public int CurrentLevel {
        get { return currentLevel_; }
    }
    public float CurrentExp {
        get { return currentExp_; }
    }

    public float AddedExp {
        get { return addedExp_; }
    }

    // 累計取得経験値。毎フレームのリセットが無いので、消費側が前回値との差分を取れば
    // スクリプトの実行順に依存せず「このフレームの増分」を得られる。
    public float TotalGainedExp {
        get { return totalGainedExp_; }
    }

    public bool IsLevelUp {
        get { return isLevelUp_; }
    }

    public float RequiredExp {
        get { return requiredExp_; }
    }
    /// <summary>
    /// 現在の経験値の進捗を0.0～1.0で返すメソッド
    /// </summary>
    /// <returns></returns>
    public float GetExpProgress() {
        if (requiredExp_ == 0) {
            return 0f;
        }
        return currentExp_ / requiredExp_;
    }
}

