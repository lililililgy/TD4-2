using System;
using System.Collections.Generic;

// プレイヤー（母体）が抱える卵(roe)を統括する。卵は別 Entity で、各卵の状態は RoeStateComponent が持つ。
//
// 唯一の真実(single source of truth)は HP。卵の数は毎フレーム HP.CurrentHp に合わせて増減させる：
//   卵数 = HP.CurrentHp（= 残機）。上限は HP.MaxHp。弾数 = 成熟卵(MatureCount)。
//   被弾で HP が減れば隊列末尾の卵が消え、回復（レベルアップ）で増えれば卵が生える。
//   発射(TryConsumeMature)は成熟卵を消費し、そのぶん HP も1減らす
//   （減らさないと次フレームに卵が再生成されて弾が無限になる。ただし最後の1卵は残す）。
//
// 卵の増減を要求する側は HP を書き換えるだけでよく、卵の生成・破棄はここが吸収する。
//
// 隊列順は entityId をキーに記録し、卵が自分の Initialize / Update で pull する
// （生成直後の卵はまだスクリプトインスタンスが無く直接 push できないため）。
public class RoeManager : MonoScript
{

    [SerializeField] private string roePrefabName_ = "Roe";  // 生成する卵プレハブ名

    private readonly List<Entity> roe_ = new List<Entity>();
    private HP hp_;
    private float lastDistributedPlayerExp_ = 0f; // 前フレームまでに卵へ分配済みのプレイヤー累計経験値

    public override void Initialize()
    {
        hp_ = entity.GetScript<HP>();
        // 初期卵は Spawn しない。最初の Update で HP.CurrentHp ぶんが揃う
    }

    public override void Update()
    {
        // 破棄済み（発射・撃破）の卵を隊列から掃除
        Prune();

        // 卵の数を HP に合わせる（被弾で減った／回復で増えた分をここで吸収する）
        SyncEggsToHp();

        // 隊列順(リスト順)を各卵の TrailFollower に push する。
        // 生成直後でスクリプト未生成の卵も、次フレーム以降にここで順番が入る。
        PushOrders();

        LevelingComponent levelingComponent = entity.GetScript<LevelingComponent>();

        // プレイヤーが新たに取得した経験値(前フレームからの増分)を卵に分配する。
        // AddedExp は LevelingComponent.Update が毎フレーム先頭でリセットするため、
        // スクリプト実行順(Leveling→RoeManager)では常に 0 になってしまう。
        // 累計値(TotalGainedExp)の差分を自分で取ることで実行順に依存せず増分を得る。
        float playerTotalExp = levelingComponent.TotalGainedExp;
        float gainedExp = playerTotalExp - lastDistributedPlayerExp_;
        lastDistributedPlayerExp_ = playerTotalExp;
        if (gainedExp > 0)
        {
            foreach (Entity e in roe_)
            {
                if (e == null)
                {
                    continue;
                }
                RoeStateComponent state = e.GetScript<RoeStateComponent>();
                if (state == null || state.IsMature)
                {
                    continue;
                }

                LevelingComponent roeLevel = e.GetScript<LevelingComponent>();
                roeLevel?.AddExperience(gainedExp);
            }
        }
    }

    // ---- 導出値（残機・弾・HP） ----

    // 弾数 = 成熟した卵の数
    public int MatureCount()
    {
        int count = 0;
        for (int i = 0; i < roe_.Count; i++)
        {
            RoeStateComponent state = GetState(roe_[i]);
            if (state != null && state.IsMature)
            {
                count++;
            }
        }
        return count;
    }

    // 隊列の全卵数（未成熟の子たまごも含む）。HP に追従するので、実質「現在の残機」と同じ。
    public int EggCount()
    {
        return roe_.Count;
    }

    // 発射できる状態か（成熟卵があり、かつ最後の1卵ではない）。TryConsumeMature() が成功する条件そのもの。
    public bool CanShoot()
    {
        return EggCount() > 1 && MatureCount() > 0;
    }

    // 卵数が count 個に満たなければ、足りないぶんだけ幼生たまご(未成熟の卵)を与える。
    // 実際に増えた数を返す（HP 上限で頭打ちになる／既に足りていれば 0）。
    //
    // 卵数の真実は HP なので、判断も HP で行う。roe_ の実体は次の Update の
    // SyncEggsToHp が追いつかせるため、生成直後の卵はまだ roe_ に居ない（EggCount() は1フレーム遅れる）。
    // 生えたばかりの卵は未成熟なので、増えるのは必ず幼生たまご。
    public int EnsureEggCount(int count)
    {
        if (hp_ == null)
        {
            return 0;
        }

        int current = (int)hp_.CurrentHp;
        if (current >= count)
        {
            return 0;
        }

        // SetHp は MaxHp でクランプする。※ Heal ではなく SetHp なのは TryConsumeMature と同じ理由
        hp_.SetHp(count);
        return (int)hp_.CurrentHp - current;
    }

    // ---- 状態遷移（発射） ----

    // 成熟卵(弾)を1つ隊列から外して返す（発射用）。無ければ null。破棄は呼び出し側に任せる。
    // 卵が合計1個のときは発射不可（発射での即ゲームオーバー防止。子たまごが残っていれば最後の成熟卵も撃てる）。
    public Entity TryConsumeMature()
    {
        if (EggCount() <= 1)
        {
            return null;
        }

        Entity e = ConsumeMature();
        if (e == null)
        {
            return null;
        }

        // 発射ぶん HP も減らす。減らさないと SyncEggsToHp が次フレームに卵を生やし直し、
        // 弾が無限になる。※ TakeDamage ではなく SetHp を使う
        //（TakeDamage は被弾扱いになり、被弾演出(ShakeOnDamaged)が発射のたびに出てしまう）。
        if (hp_ != null)
        {
            hp_.SetHp(hp_.CurrentHp - 1);
        }
        return e;
    }

    // ---- 内部 ----

    // 卵の数を HP.CurrentHp に合わせる。多ければ末尾から破棄し、足りなければ生成する。
    private void SyncEggsToHp()
    {
        if (hp_ == null)
        {
            return;
        }

        int target = (int)hp_.CurrentHp;
        if (target < 0)
        {
            target = 0;
        }

        // 被弾で減った分：隊列末尾の卵を成熟/未成熟問わず外して破棄する
        while (roe_.Count > target)
        {
            if (!RemoveTailEgg())
            {
                break;
            }
        }

        // 回復（レベルアップ）で増えた分：生成に失敗したらそこで打ち切る
        while (roe_.Count < target)
        {
            if (!Spawn())
            {
                break;
            }
        }
    }

    // 隊列末尾の卵を1つ外して破棄する。成功で true
    private bool RemoveTailEgg()
    {
        for (int i = roe_.Count - 1; i >= 0; i--)
        {
            if (roe_[i] != null)
            {
                Entity e = roe_[i];
                roe_.RemoveAt(i);
                e.Destroy();
                return true;
            }
        }
        return false;
    }

    // 成熟卵を1つ隊列から外して返す（発射専用ヘルパー）。末尾側から探す。無ければ null。
    private Entity ConsumeMature()
    {
        for (int i = roe_.Count - 1; i >= 0; i--)
        {
            RoeStateComponent state = GetState(roe_[i]);
            if (state != null && state.IsMature)
            {
                Entity e = roe_[i];
                roe_.RemoveAt(i);
                return e;
            }
        }
        return null;
    }

    private RoeStateComponent GetState(Entity e)
    {
        return e != null ? e.GetScript<RoeStateComponent>() : null;
    }

    // 破棄済みの卵をリストから除去する（隊列順は Update の PushOrders で詰め直す）
    private void Prune()
    {
        for (int i = roe_.Count - 1; i >= 0; i--)
        {
            if (roe_[i] == null)
            {
                roe_.RemoveAt(i);
            }
        }
    }

    // リスト順 = 隊列順(1始まり) を各卵の TrailFollower に push する。
    // 各卵は order を自分で持ち、母体(Player)を order に応じた距離で追従する。
    private void PushOrders()
    {
        for (int i = 0; i < roe_.Count; i++)
        {
            if (roe_[i] != null)
            {
                TrailFollower follower = roe_[i].GetScript<TrailFollower>();
                follower?.SetOrder(i + 1);
            }
        }
    }

    // 卵を1個生成し隊列末尾に加える。生成できたら true。
    private bool Spawn()
    {
        Entity e = ecsGroup.CreateEntity(roePrefabName_);
        if (e == null)
        {
            return false;
        }

        // 初期位置はプレイヤー位置に寄せる（以後 TrailFollower が追従）
        Transform t = e.GetComponent<Transform>();
        if (t != null)
        {
            t.position = transform.position;
        }

        roe_.Add(e);
        return true;
    }
}
