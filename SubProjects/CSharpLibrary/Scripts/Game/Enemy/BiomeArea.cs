using System.Collections.Generic;

// 矩形リージョンに紐づく Biome。エンティティの Transform.position を中心とした
// half-extents の矩形内かどうかを判定できる。
public class BiomeArea : MonoScript {
    [SerializeField] private List<SpawnEntry> spawnTable_ = new List<SpawnEntry> {
        new SpawnEntry { enemyType = "Test", weight = 1.0f },
    };
    // エリアの半サイズ。中心はエンティティの Transform.position
    [SerializeField] private Vector2 halfExtents_;

    private Biome biome_;

    public override void Initialize() {
        biome_ = new Biome(spawnTable_);
    }

    public override void Update() {
        BoxCollider2D boxCollider = entity.GetComponent<BoxCollider2D>();

        if (boxCollider == null) {
            return;
        }

        Vector2 scale = new Vector2(transform.scale.x, transform.scale.y);
        halfExtents_ = new Vector2(boxCollider.size.x * scale.x * 0.5f, boxCollider.size.y * scale.y * 0.5f);

    }

    public Biome GetBiome() {
        return biome_;
    }

    // pos（絶対ワールド座標）がこのエリアの矩形内にあるかどうかを判定する
    public bool Contains(Vector2 pos) {
        Vector2 center = new Vector2(transform.position.x, transform.position.y);
        Vector2 diff = pos - center;
        return System.Math.Abs(diff.x) <= halfExtents_.x && System.Math.Abs(diff.y) <= halfExtents_.y;
    }
}
