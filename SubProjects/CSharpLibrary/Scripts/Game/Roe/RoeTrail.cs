using System;
using System.Collections.Generic;

// プレイヤー（母体）の移動経路を記録し、現在位置から経路に沿って指定距離だけ後ろの点を返す。
// 卵(roe)はこれを「自分の順番 × 間隔」の距離でサンプリングして追従する。
// 一定間隔でサンプリングするので、卵は重ならず・順番が保たれる。
public class RoeTrail : MonoScript {

    [SerializeField] private float recordMinDistance_ = 2.0f;    // この距離動くごとに経路点を記録
    [SerializeField] private float maxTrailLength_    = 400.0f;  // 保持する最大経路長

    private readonly List<Vector3> points_ = new List<Vector3>(); // [0] が最新の記録点

    public override void Update() {
        Vector3 cur = transform.position;
        if (points_.Count == 0 || (cur - points_[0]).Length() >= recordMinDistance_) {
            points_.Insert(0, cur);
            TrimToLength();
        }
    }

    // 現在位置から経路に沿って distance だけ後ろの点を返す
    public Vector3 SampleBehind(float distance) {
        Vector3 prev = transform.position;
        if (distance <= 0.0f) {
            return prev;
        }

        float remaining = distance;
        for (int i = 0; i < points_.Count; i++) {
            Vector3 next = points_[i];
            float seg = (next - prev).Length();
            if (seg >= remaining) {
                float t = (seg > 0.0f) ? remaining / seg : 0.0f;
                return prev + (next - prev) * t;
            }
            remaining -= seg;
            prev = next;
        }
        return prev; // 経路がまだ短ければ末尾（最古点）
    }

    private void TrimToLength() {
        float total = 0.0f;
        for (int i = 0; i < points_.Count - 1; i++) {
            total += (points_[i + 1] - points_[i]).Length();
            if (total > maxTrailLength_) {
                points_.RemoveRange(i + 1, points_.Count - (i + 1));
                return;
            }
        }
    }
}
