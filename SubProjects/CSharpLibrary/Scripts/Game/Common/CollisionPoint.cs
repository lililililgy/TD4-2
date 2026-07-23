using System;

// 衝突点(接触点)の推定。
//
// engine の CollisionSystem は内部で contactPoint を計算しているが、C# 側の
// OnCollisionEnter(Entity collision) には相手の Entity しか渡ってこないため、
// スクリプトからは接触点を受け取れない。そこでコライダーの形状から同じ値を計算し直す。
//
// engine 側(Collision2DSystem.cpp の CheckMethod2D)と同じ式を使っているので、
// 円どうしの衝突では engine が持っている contactPoint と一致する。
public static class CollisionPoint {

    // self と other の接触点を推定して返す。
    // self 側のコライダー表面のうち、other に最も近い点を接触点とみなす。
    // どちらの形状も分からない場合は中心どうしの中点にフォールバックする。
    public static Vector3 Estimate(Entity self, Entity other) {
        if (self == null || other == null) {
            return Vector3.zero;
        }

        Vector3 selfPos  = Position(self);
        Vector3 otherPos = Position(other);

        Vector3 delta = otherPos - selfPos;
        if (delta.LengthSq() <= 0.0001f) {
            return selfPos; // 完全に重なっている。方向が決まらないので中心を返す
        }

        // 円/球: 中心から相手方向へ半径ぶん進んだ点。engine の
        // contactPoint = p1 + dir * r1 と同じ式。
        float radius = SelfRadius(self);
        if (radius > 0.0f) {
            return selfPos + delta.Normalized() * radius;
        }

        // 箱: 相手の中心を箱のローカル軸に落として、半サイズで clamp した点。
        // engine の Box2D vs Circle の最近接点計算と同じ。
        BoxCollider2D box = self.GetComponent<BoxCollider2D>();
        if (box != null) {
            return ClosestPointOnBox2D(self, box, otherPos);
        }

        // 形状が取れない(コライダー未対応 or 親子構成で別 Entity に付いている)。
        // 中点なら少なくとも二者の間に出るので、演出としては破綻しない。
        return Vector3.Lerp(selfPos, otherPos, 0.5f);
    }

    // self の円/球コライダーの実効半径。持っていなければ 0。
    // useOwnerScale のときに max(scale.x, scale.y) を掛けるのは engine と同じ扱い。
    private static float SelfRadius(Entity self) {
        CircleCollider circle = self.GetComponent<CircleCollider>();
        if (circle != null) {
            float radius = circle.radius;
            if (circle.useOwnerScale) {
                Vector3 scale = Scale(self);
                radius *= Math.Max(scale.x, scale.y);
            }
            return radius;
        }

        // SphereCollider は C# 側に useOwnerScale が無いので radius をそのまま使う
        SphereCollider sphere = self.GetComponent<SphereCollider>();
        if (sphere != null) {
            return sphere.radius;
        }

        return 0.0f;
    }

    // 箱(回転込み)の表面のうち target に最も近い点。
    private static Vector3 ClosestPointOnBox2D(Entity self, BoxCollider2D box, Vector3 target) {
        Transform transform = self.GetComponent<Transform>();
        if (transform == null) {
            return Position(self);
        }

        Vector3 center = Position(self);
        Vector3 axisX  = transform.right;
        Vector3 axisY  = transform.up;

        Vector2 size = box.size;
        if (box.useOwnerScale) {
            Vector3 scale = Scale(self);
            size.x *= scale.x;
            size.y *= scale.y;
        }
        float halfX = size.x * 0.5f;
        float halfY = size.y * 0.5f;

        Vector3 delta = target - center;
        float localX = Mathf.Clamp(Vector3.Dot(delta, axisX), -halfX, halfX);
        float localY = Mathf.Clamp(Vector3.Dot(delta, axisY), -halfY, halfY);

        return center + axisX * localX + axisY * localY;
    }

    // 座標。transform.matrix（ワールド行列）はシーン読み込み時にしか C# へ同期されない
    // （ComponentBatchManager.ReceiveAllBatches の呼び出し元が SyncInitialComponentsToCS だけ）ので
    // 実行中は使えない。毎フレーム更新されるのは C# 側が書いている position のほう。
    //
    // そのため親を持つ Entity ではローカル座標になる。このプロジェクトのコライダーは
    // すべてルート Entity に付いている（ローカル＝ワールド）ので実害はないが、
    // 子 Entity にコライダーを付ける構成が出てきたらここを直す必要がある。
    private static Vector3 Position(Entity entity) {
        Transform transform = entity.GetComponent<Transform>();
        if (transform == null) {
            return Vector3.zero;
        }
        return transform.position;
    }

    private static Vector3 Scale(Entity entity) {
        Transform transform = entity.GetComponent<Transform>();
        if (transform == null) {
            return Vector3.one;
        }
        return transform.scale;
    }
}
