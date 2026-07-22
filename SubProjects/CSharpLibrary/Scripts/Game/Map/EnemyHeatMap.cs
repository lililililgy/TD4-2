using System;
using System.Collections.Generic;
using System.Linq;
using System.Security.Cryptography.X509Certificates;
using System.Text;
using System.Threading.Tasks;

public class EnemyHeatMap
    : MonoScript {

    [SerializeField] private Vector2 cellSize_ = default;

    // 2D グリッドのヒートマップ。各セルは敵の出現回数をカウントする。
    private Dictionary<int, int> heatMap_;
    // 現在の敵の総数。ヒートマップの合計値と一致するはず。
    [SerializeField] private int enemyCount_ = 0;
    [SerializeField] private List<string> enemies_ = new List<string>();

    public override void Initialize() {
        heatMap_ = new Dictionary<int, int>();
    }

    // 実行順序に気をつけること
    public override void Update() {
        enemyCount_ = 0;
        heatMap_.Clear();
        enemies_.Clear();
    }

    public override void OnCollisionStay(Entity collision) {
        Transform enemyT = collision.GetComponent<Transform>();

        Vector2 enemyPos = new Vector2(enemyT.position.x, enemyT.position.y);

        AddHeat(enemyPos);

        enemies_.Add(collision.name);

        enemyCount_++;
    }

    public int GetHeat(Vector2 position) {
        Vector2Int cellId = GetCellId(position);
        int hash = CalclateHash(cellId);
        if (heatMap_.ContainsKey(hash)) {
            return heatMap_[hash];
        } else {
            return 0;
        }
    }

    public void AddHeat(Vector2 position) {
        Vector2Int cellId = GetCellId(position);
        int hash = CalclateHash(cellId);
        if (heatMap_.ContainsKey(hash)) {
            heatMap_[hash]++;
        } else {
            heatMap_[hash] = 1;
        }
    }

    public int GetHeatByCellId(Vector2Int cellId) {
        int hash = CalclateHash(cellId);
        if (heatMap_.ContainsKey(hash)) {
            return heatMap_[hash];
        } else {
            return 0;
        }
    }

    public int CalclateHash(Vector2Int cellId) {
        return cellId.x * 73856093 ^ cellId.y * 19349663;
    }

    public int GetEnemyCount() {
        return enemyCount_;
    }

    public Vector2 GetCellSize() {
        return cellSize_;
    }

    // 負座標でも正しくセル境界を跨ぐよう floor で丸める。
    // (int)キャストは 0 方向への切り捨てになり、セル0だけ倍幅になってしまう。
    public Vector2Int GetCellId(Vector2 position) {
        return new Vector2Int(
            (int)Math.Floor(position.x / cellSize_.x),
            (int)Math.Floor(position.y / cellSize_.y)
        );
    }
}