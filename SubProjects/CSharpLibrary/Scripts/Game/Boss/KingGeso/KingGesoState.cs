using System.Threading;

using System.Collections.Generic;

/// <summary>
/// キングゲソの状態(基底クラス)
/// </summary>
internal interface IKingGesoState
{
    void Enter(KingGeso owner);
    void Update(KingGeso owner);
    void Exit(KingGeso owner);
}

//==========================================-
// キングゲソの待機状態クラス
//==========================================
internal sealed class KingGesoIdleState : IKingGesoState
{
    // 経過時間
    private float elapsed;

    /// <summary>
    /// 状態開始時の処理
    /// </summary>
    /// <param name="owner"></param>
    public void Enter(KingGeso owner)
    {
        elapsed = 0.0f;
    }

    /// <summary>
    /// 状態更新時の処理
    /// </summary>
    /// <param name="owner"></param>
    public void Update(KingGeso owner)
    {
        elapsed += Time.deltaTime;
        if (owner.ConsumeAttackRequest() || elapsed >= owner.IdleDuration)
        {
            // 待機時間が経過したか、攻撃要求があった場合、攻撃状態に遷移
            owner.ChangeState(new KingGesoAttackState());
        }
    }

    /// <summary>
    /// 状態終了時の処理
    /// </summary>
    /// <param name="owner"></param>
    public void Exit(KingGeso owner)
    {
    }
}



//==========================================-
// キングゲソの攻撃状態
//==========================================
internal sealed class KingGesoAttackState : IKingGesoState
{
    private IKingGesoAttack currentAttack_;

    public void Enter(KingGeso owner)
    {
        currentAttack_ = CreateAttack(owner.SelectAttackType());
        currentAttack_.Enter(owner);
    }

    public void Update(KingGeso owner)
    {
        if (currentAttack_ == null || currentAttack_.Update(owner))
        {
            //attack終了後、クールダウン状態に遷移
            owner.ChangeState(new KingGesoCooldownState());
        }
    }

    public void Exit(KingGeso owner)
    {
        if (currentAttack_ != null)
        {
            currentAttack_.Exit(owner);
            currentAttack_ = null;
        }

        // 攻撃終了時にアクティブなゲソを破棄
        owner.DestroyActiveGeso();
    }

    private IKingGesoAttack CreateAttack(KingGesoAttackType attackType)
    {
  
        if (attackType == KingGesoAttackType.PincerThrust)
        {
            return new KingGesoPincerThrustAttack();
        }

        if(attackType == KingGesoAttackType.HomingBullet) {
            return new KingGesoHomingAttack();
        }

        if (attackType == KingGesoAttackType.InkBarrage) {
            return new KingGesoInkBarrageAttack();
        }


        return new KingGesoWaveThrustAttack();
    }
}

//==========================================-
// キングゲソのクールダウン状態
//==========================================
internal sealed class KingGesoCooldownState : IKingGesoState
{
    private float elapsed;

    public void Enter(KingGeso owner)
    {
        // クールダウン状態に入った時点で経過時間をリセット
        elapsed = 0.0f;
    }

    public void Update(KingGeso owner)
    {
        elapsed += Time.deltaTime;
        if (elapsed >= owner.CooldownDuration)
        {
            // クールダウン終了後、待機状態に遷移
            owner.ChangeState(new KingGesoIdleState());
        }
    }

    public void Exit(KingGeso owner)
    {
    }
}

//==========================================-
// キングゲソのダメージ状態
//==========================================
internal sealed class  KingGesoDamageState : IKingGesoState {

    private float duration = 1.0f; // ダメージ状態の持続時間（秒）
    private float elapsed = 0.0f; // 経過時間

    public void Enter(KingGeso owner) {
        // ダメージ状態に入った時の処理
        // ダメージ状態のタイマーを開始
        elapsed = 0.0f;
        duration = owner.CooldownDuration;
    }

    public void Update(KingGeso owner) {
        // ダメージ状態の更新処理（必要に応じて追加）
        elapsed += Time.deltaTime;

        //スプライトの点滅処理
        //カラーを点滅させる
        SpriteRenderer spriteRenderer = owner.entity.GetComponent<SpriteRenderer>();
        if (spriteRenderer != null) {
            float blinkFrequency = 5.0f; // 点滅の周波数（Hz）
            float blinkPhase = Mathf.Sin(Time.time * blinkFrequency * Mathf.PI * 2.0f);
            float alpha = Mathf.Clamp01((blinkPhase + 1.0f) * 0.5f); // 0から1の範囲に変換
            Vector4 originalColor = spriteRenderer.color;
            spriteRenderer.color = new Vector4(1.0f, 0.0f, 0.0f, alpha);
        }

        if(elapsed >= duration) {
            // ダメージ状態が終了した場合、待機状態に遷移
            owner.ChangeState(new KingGesoIdleState());
        }
    }

    public void Exit(KingGeso owner) {
        // ダメージ状態からの遷移時の処理

        // スプライトの色を元に戻す
        SpriteRenderer spriteRenderer = owner.entity.GetComponent<SpriteRenderer>();
        if (spriteRenderer != null) {
            spriteRenderer.color = new Vector4(1.0f, 1.0f, 1.0f, 1.0f); // 元の色に戻す
        }

    }
}

//==========================================
// キングゲソの死亡状態
//==========================================
internal sealed class KingGesoDeadState : IKingGesoState
{
    public void Enter(KingGeso owner)
    {
        // 死亡時の処理（必要に応じて追加）
        owner.DestroyActiveGeso();
        owner.DeactivateAllInkBullets();
    }
    public void Update(KingGeso owner)
    {
        // 死亡状態では特に更新処理は不要
    }
    public void Exit(KingGeso owner)
    {
        // 死亡状態からの遷移は通常ないため、特に処理は不要
    }
}
