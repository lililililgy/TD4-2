# Rigidbody2D 扱い方ガイド

本エンジンの **`Rigidbody2D`** コンポーネントおよび物理挙動の制御方法についてまとめたドキュメントです。

---

## 1. 基本的な設計コンセプト
本エンジンの `Rigidbody2D` は、**「C++ 側で速度や衝突応答（跳ね返り）の物理計算を行い、C# 側で座標（`Transform.position`）の更新を行う」** というハイブリッド方式を採用しています。
これにより、既存の C# 側での座標更新ロジックを壊すことなく、物理的な衝突挙動を簡単に組み込むことができます。

---

## 2. パラメータ一覧
C# 側（およびシーンの JSON 定義）から設定可能なパラメータは以下の通りです。

| パラメータ | 型 | 初期値 | 説明 |
| :--- | :--- | :--- | :--- |
| `velocity` | `Vector2` | `(0, 0)` | 剛体の移動速度（毎フレーム C# スクリプトが参照して移動に適用します） |
| `mass` | `float` | `1.0f` | 剛体の質量（衝突時の跳ね返りの勢いに影響します） |
| `restitution` | `float` | `1.0f` | 反発係数（`0.0` で跳ね返りなし、`1.0` で完全弾性衝突） |
| `useGravity` | `bool` | `false` | 重力適用の有無 |
| `gravityScale` | `float` | `1.0f` | 重力倍率（`useGravity` が有効な場合のみ） |
| `freezeX` | `bool` | `false` | X軸方向の物理運動（速度）を 0 に固定 |
| `freezeY` | `bool` | `false` | Y軸方向の物理運動（速度）を 0 に固定 |

---

## 3. C# スクリプトでの使用例
`Rigidbody2D` を持つオブジェクトを動かす場合、スクリプトの `Update` 等で **`velocity` に基づいて `transform.position` を更新する必要があります**。

### 動作スクリプトの書き方
```csharp
using System;
using ONEngine;

public class PhysicsObject : MonoScript
{
    private Rigidbody2D rb;

    public override void Initialize()
    {
        // Rigidbody2D コンポーネントの取得
        rb = entity.GetComponent<Rigidbody2D>();
    }

    public override void Update()
    {
        if (rb && transform)
        {
            // 1. 必要に応じてキー入力などから速度を直接変更可能
            // Vector2 vel = rb.velocity;
            // vel.x = Input.GetAxis("Horizontal") * speed;
            // rb.velocity = vel;

            // 2. Rigidbody2D の速度を参照して座標を更新する（重要）
            Vector2 velocity = rb.velocity;
            Vector3 position = transform.position;
            
            position.x += velocity.x * Time.deltaTime;
            position.y += velocity.y * Time.deltaTime;
            
            transform.position = position;
        }
    }
}
```

---

## 4. 衝突判定と跳ね返り (衝突応答) の流れ
コライダー (`CircleCollider` や `BoxCollider2D`) と `Rigidbody2D` の両方をアタッチすることで、衝突時の自動跳ね返り処理が有効になります。

1. **移動**: C# 側のスクリプトが `rb.velocity` に基本移動量を掛けて座標を更新します。
2. **同期 (C# → C++)**: `Transform.position` と `Rigidbody2D.velocity` が C++ 側へ同期されます。
3. **物理処理 (C++側)**:
   - コライダーの交差検知後、押し戻し（めり込み修正）を行います。
   - 双方が `Rigidbody2D` を持っている場合、お互いの質量 (`mass`) と反発係数 (`restitution`) に応じた**撃力（インパルス）**が計算され、C++ 側の `velocity` が反転・減速更新されます。
4. **同期 (C++ → C#)**: 書き換わった `velocity` が C# 側の `rb.velocity` に逆同期されます。
5. **反射移動**: 次フレームの C# スクリプトで、反転した `rb.velocity` に基づきオブジェクトが跳ね返る方向に移動します。

---

## 5. シーンファイル (`.entity`) での定義例
シーンの構成ファイル（`.entity`）に `Rigidbody2D` を記述する際のフォーマットです。

```json
{
    "active": true,
    "components": [
        {
            "enable": 1,
            "type": "Transform",
            "position": { "x": -5.0, "y": 0.0, "z": 0.0 }
        },
        {
            "enable": 1,
            "type": "CircleCollider",
            "radius": 1.0,
            "isTrigger": false,
            "mass": 1.0,
            "state": "Dynamic"
        },
        {
            "enable": 1,
            "type": "Rigidbody2D",
            "velocity": { "x": 5.0, "y": 0.0 },
            "mass": 1.0,
            "restitution": 1.0,
            "useGravity": false,
            "gravityScale": 1.0,
            "freezeX": false,
            "freezeY": false
        },
        {
            "enable": 1,
            "type": "Script",
            "scripts": [
                {
                    "enable": true,
                    "name": "PhysicsObject"
                }
            ]
        }
    ],
    "guid": "unique-entity-guid-here",
    "name": "PhysicsBall"
}
```
