using System;
using System.Collections.Generic;

// プレイヤー（母体）が抱える卵(roe)を統括する。卵は別 Entity で、各卵の状態は RoeStateComponent が持つ。
// このリスト＋各卵の状態が唯一の真実(single source of truth)で、残機・弾薬はここから導出する：
//   残機 = 成熟卵(MATURE)の数 = MatureCount()
//   弾薬 = 幼生卵(LARVAE)の数 = LarveCount()
//
// 卵は有限資源。上限は「卵」と「弾」で別管理する：
//   卵(UNMATURE+MATURE) の上限 = maxRoe_  … 母体が時間（暫定。本来は経験値）で産卵する分の上限
//   弾(LARVAE)          の上限 = maxAmmo_ … リロードで作り置きできる弾の上限
//   リロード   : MATURE を1つ LARVAE 化（残機を犠牲に弾を作る。弾が上限なら不可）→ TryReload()
//   発射       : LARVAE を1つ隊列から外して返す（呼び出し側が弾化して破棄）→ TryConsumeLarvae()
//
// 隊列順は entityId をキーに記録し、卵が自分の Initialize / Update で pull する
// （生成直後の卵はまだスクリプトインスタンスが無く直接 push できないため）。
public class RoeManager : MonoScript {

    [SerializeField] private string roePrefabName_ = "Roe";  // 生成する卵プレハブ名

    [SerializeField] private int maxRoe_ = 5;    // 卵(UNMATURE+MATURE)の上限
    [SerializeField] private int maxAmmo_ = 5;    // 弾(LARVAE)の上限

    private readonly List<Entity> roe_ = new List<Entity>();
    private readonly Dictionary<int, int> orderById_ = new Dictionary<int, int>(); // entityId -> 隊列順(1始まり)

    public override void Update() {
        // 破棄済み（発射・撃破）の卵を隊列から掃除
        Prune();

        LevelingComponent levelingComponent = entity.GetScript<LevelingComponent>();
        // Levelが上がったタイミングで産卵
        if (levelingComponent.IsLevelUp) {
            if (EggCount() < maxRoe_) {
                Spawn();
            }
        }

        // プレイヤーが受け取った経験値を卵に分配する
        float addedExp = levelingComponent.AddedExp;
        if (addedExp > 0) {
            foreach (Entity e in roe_) {
                if (e == null) {
                    continue;
                }
                RoeStateComponent state = e.GetScript<RoeStateComponent>();
                if (state == null || state.CurrentState != RoeState.UNMATURE) {
                    continue;
                }

                LevelingComponent roeLevel = e.GetScript<LevelingComponent>();
                roeLevel?.AddExperience(addedExp);
            }
        }
    }

    // ---- 導出値（残機・弾薬） ----

    // 残機 = 成熟した卵の数
    public int MatureCount() {
        return CountState(RoeState.MATURE);
    }

    // 弾薬 = 幼生卵の数
    public int LarveCount() {
        return CountState(RoeState.LARVAE);
    }

    // 卵 = 未成熟＋成熟の数（弾は含まない）。産卵上限の判定に使う。
    public int EggCount() {
        return CountState(RoeState.UNMATURE) + CountState(RoeState.MATURE);
    }

    public int MaxRoe { get { return maxRoe_; } }
    public int MaxAmmo { get { return maxAmmo_; } }

    // 弾(LARVAE)に空きがあるか（リロード可否）
    public bool CanLoadAmmo { get { return LarveCount() < maxAmmo_; } }

    // ---- 状態遷移（リロード・発射） ----

    // 成熟卵(残機)を1つ幼生卵(弾薬)に変える。インデックスの小さい（隊列の前の）卵から探す。
    // 弾が上限に達していれば作れない。成功で true。
    public bool TryReload() {
        if (!CanLoadAmmo) {
            return false;
        }
        for (int i = 0; i < roe_.Count; i++) {
            RoeStateComponent state = GetState(roe_[i]);
            if (state != null && state.CurrentState == RoeState.MATURE) {
                state.SetState(RoeState.LARVAE);
                return true;
            }
        }
        return false;
    }

    // ダメージで残機を1つ失う：成熟卵(MATURE)を1つ隊列から外して破棄する。末尾側から。成功で true。
    public bool TryKillLife() {
        for (int i = roe_.Count - 1; i >= 0; i--) {
            RoeStateComponent state = GetState(roe_[i]);
            if (state != null && state.CurrentState == RoeState.MATURE) {
                Entity e = roe_[i];
                roe_.RemoveAt(i);
                ReassignOrders();
                e.Destroy();
                return true;
            }
        }
        return false;
    }

    // 幼生卵(弾薬)を1つ隊列から外して返す（発射用）。無ければ null。破棄は呼び出し側に任せる。
    public Entity TryConsumeLarvae() {
        for (int i = roe_.Count - 1; i >= 0; i--) {
            RoeStateComponent state = GetState(roe_[i]);
            if (state != null && state.CurrentState == RoeState.LARVAE) {
                Entity e = roe_[i];
                roe_.RemoveAt(i);
                ReassignOrders();
                return e;
            }
        }
        return null;
    }

    // 卵が自分の順番を引くための問い合わせ（未登録は -1）
    public int GetOrder(int entityId) {
        return orderById_.TryGetValue(entityId, out int order) ? order : -1;
    }

    // ---- 内部 ----

    private int CountState(RoeState target) {
        int count = 0;
        for (int i = 0; i < roe_.Count; i++) {
            RoeStateComponent state = GetState(roe_[i]);
            if (state != null && state.CurrentState == target) {
                count++;
            }
        }
        return count;
    }

    private RoeStateComponent GetState(Entity e) {
        return e != null ? e.GetScript<RoeStateComponent>() : null;
    }

    // 破棄済みの卵をリストから除去し、隊列順を詰め直す
    private void Prune() {
        bool changed = false;
        for (int i = roe_.Count - 1; i >= 0; i--) {
            if (roe_[i] == null) {
                roe_.RemoveAt(i);
                changed = true;
            }
        }
        if (changed) {
            ReassignOrders();
        }
    }

    // リスト順 = 隊列順(1始まり) で order を振り直す。卵側が GetOrder で pull する。
    private void ReassignOrders() {
        orderById_.Clear();
        for (int i = 0; i < roe_.Count; i++) {
            if (roe_[i] != null) {
                orderById_[roe_[i].Id] = i + 1;
            }
        }
    }

    // 卵を1個生成し隊列末尾に加える。生成できたら true。
    private bool Spawn() {
        Entity e = ecsGroup.CreateEntity(roePrefabName_);
        if (e == null) {
            return false;
        }

        // 初期位置はプレイヤー位置に寄せる（以後 RoeFollower が追従）
        Transform t = e.GetComponent<Transform>();
        if (t != null) {
            t.position = transform.position;
        }

        roe_.Add(e);
        ReassignOrders();
        return true;
    }
}
