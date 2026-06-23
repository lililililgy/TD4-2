using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

public class LevelingComponent : MonoScript {
    [SerializeField] private int maxLevel_;

    private int currentLevel_;
    private float currentExp_;
    private float requiredExp_; // 次のLevelに必要な経験値

    private bool isLevelUp_ = false; // Levelが上がったかどうかのフラグ

    public override void Update() {
        isLevelUp_ = false; // フラグをリセット

        if (currentExp_ >= requiredExp_) {
            LevelUp();
        }
    }

    /// <summary>
    /// LevelUp処理を行うメソッド
    /// </summary>
    private void LevelUp() {
        isLevelUp_ = true;
        currentLevel_++;

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
        return 100f; // 一旦固定
    }

    public override void OnCollisionEnter(Entity collision) {
        ExperiencePoint exp = collision.GetScript<ExperiencePoint>();
        if (exp == null) {
            return;
        }
        currentExp_ += exp.ExperiencePoints;
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

    public bool IsLevelUp {
        get { return isLevelUp_; }
    }

}

