using System;
using System.Collections.Generic;

// プレイヤー（母体）が抱える卵(roe)を統括する。卵は別 Entity で、各卵の状態は RoeStateComponent が持つ。
// このリスト＋各卵の状態が唯一の真実(single source of truth)で、残機・弾薬はここから導出する：
//   残機 = 成熟卵(MATURE)の数 = MatureCount()
//   弾薬 = 幼生卵(LARVAE)の数 = LarveCount()
//
// 卵は有限資源。母体が時間（暫定。本来は経験値）で UNMATURE を産卵し、上限 maxBrood_ まで増える。
//   リロード   : MATURE を1つ LARVAE 化（残機を犠牲に弾を作る）→ TryReload()
//   発射       : LARVAE を1つ隊列から外して返す（呼び出し側が弾化して破棄）→ TryConsumeLarvae()
//
// 隊列順は entityId をキーに記録し、卵が自分の Initialize / Update で pull する
// （生成直後の卵はまだスクリプトインスタンスが無く直接 push できないため）。
public class RoeManager : MonoScript {

    [SerializeField] private string roePrefabName_ = "Roe";  // 生成する卵プレハブ名

    [SerializeField] private int   maxBrood_     = 5;    // 抱えられる卵の上限（全状態の合計）
    [SerializeField] private float layInterval_  = 3.0f; // 産卵間隔（暫定：時間。経験値実装時に差し替え）

    private float layTimer_ = 0.0f;
    private readonly List<Entity> roe_ = new List<Entity>();
    private readonly Dictionary<int, int> orderById_ = new Dictionary<int, int>(); // entityId -> 隊列順(1始まり)

    public override void Update() {
        // 破棄済み（発射・撃破）の卵を隊列から掃除
        Prune();

        // 産卵（暫定：時間経過。経験値が実装されたらここを「経験値消費で1個産む」に差し替える）
        layTimer_ += Time.deltaTime;
        if (layTimer_ >= layInterval_) {
            layTimer_ = 0.0f;
            if (roe_.Count < maxBrood_) {
                Spawn();
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

    public int MaxBrood { get { return maxBrood_; } }

    // ---- 状態遷移（リロード・発射） ----

    // 成熟卵(残機)を1つ幼生卵(弾薬)に変える。インデックスの小さい（隊列の前の）卵から探す。成功で true。
    public bool TryReload() {
        for (int i = 0; i < roe_.Count; i++) {
            RoeStateComponent state = GetState(roe_[i]);
            if (state != null && state.CurrentState == RoeState.MATURE) {
                state.SetState(RoeState.LARVAE);
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
