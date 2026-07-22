using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;


// スポーンする敵の種類と発生確率の分布を持つ。
// データの実体は SpawnEntry のリスト。確率は重みの相対比で表現する。
public class Biome {
    private readonly List<SpawnEntry> entries_;

    public Biome(List<SpawnEntry> entries) {
        entries_ = entries ?? new List<SpawnEntry>();
    }

    // 重み付き抽選で敵の種類を1つ返す。テーブルが空なら null。
    public string PickEnemyType() {
        float total = 0.0f;
        foreach (var e in entries_) {
            if (e.weight > 0.0f) total += e.weight;
        }
        if (total <= 0.0f) return null;

        float r = RandomUtil.RandomRange(0.0f, total);
        foreach (var e in entries_) {
            if (e.weight <= 0.0f) continue;
            r -= e.weight;
            if (r <= 0.0f) return e.enemyType;
        }
        // 浮動小数の誤差で抜けた場合のフォールバック
        return entries_[entries_.Count - 1].enemyType;
    }
}

// 敵のスポーンシステム
public class EnemySpawnSystem : MonoScript {
    [SerializeField] private bool isActive_ = false;

    // 稼働のON/OFF。フェーズごとの湧き制御は PhaseSpawnActivator がここを叩く。
    // エディタでの初期値は isActive_ でそのまま設定できる。
    public bool IsActive {
        get { return isActive_; }
        set { isActive_ = value; }
    }

    [SerializeField] private string originName_ = "EnemySpawnOrigin";
    [SerializeField] private string heatMapEntityName_ = "EnemyHeatMap";
    // スポーン範囲。画面の world half-extent(ScreenBounds) に対する倍率で表す
    [SerializeField] private float spawnAreaScale_ = 1.30f;
    // 除外範囲（スポーンしない範囲）。画面の world half-extent に対する倍率
    [SerializeField] private float exclusionAreaScale_ = 1.00f;
    // 除外矩形をさらに外側に広げるマージン(world単位)。敵の見た目半径ぶんの余裕が欲しい場合に使う。既定0で従来どおり。
    [SerializeField] private float exclusionMargin_ = 0.0f;
    // ズーム追従の基準にするカメラ Entity 名
    [SerializeField] private string cameraName_ = "MainCamera";
    // 敵数上限のズーム補正指数。1=線形、0=固定、2=面積比
    [SerializeField] private float maxEnemyCountZoomExponent_ = 1.0f;
    // スポーン間隔
    [SerializeField] private float spawnInterval_ = 5.0f;
    private float spawnTimer_ = 0.0f;

    // 1フレームでスポーンする最大数
    [SerializeField] private int maxSpawnCount_ = 10;
    // セル内の敵の数がこの値以上の場合、そのセルの重みを0にする
    [SerializeField] private int heatWeightThreshold_ = 5;
    // 重みの畳み込み範囲(セル数)。周囲のセルの重みを合成し、密集地帯の影響が近隣セルにも及ぶようにする
    [SerializeField] private int convolutionRadius_ = 1;
    // スポーン可能な敵の総数の上限。上限に達している場合はスポーンしない
    [SerializeField] private int maxEnemyCount_ = 100;

    // 現在のバイオームの出現テーブル。エディタで設定する。
    [SerializeField]
    private List<SpawnEntry> defaultSpawnTable_ = new List<SpawnEntry> {
        new SpawnEntry { enemyType = "MorayEel",  weight = 1.0f },
        new SpawnEntry { enemyType = "SpikeFish", weight = 1.0f },
    };

    // 矩形リージョン方式のバイオームエリア。名前はシーンのエンティティ名と一致させる。
    // リスト順＝優先度（重なりは先勝ち）
    [SerializeField] private List<string> biomeNames_ = new List<string>();
    // biomeNames_ から解決済みの BiomeArea のキャッシュ
    private List<BiomeArea> biomeAreas_ = new List<BiomeArea>();

    // 抽選候補のセルとその重み。畳み込み後の重みは実数になるため、セルIDと組にしてリストで保持する。
    private struct WeightedCell {
        public Vector2Int cellId;
        public float weight;
    }
    private List<WeightedCell> spawnCellWeights_ = new List<WeightedCell>();

    // CalculateSpawnCellWeights で求めた world 矩形。同一フレーム内で CalculateSpawnPos から参照する。
    private Vector2 spawnLo_;
    private Vector2 spawnHi_;
    private Vector2 exclusionLo_;
    private Vector2 exclusionHi_;
    private bool hasExclusionArea_;

    // Biome（実行時にテーブルから構築する）
    private Biome defaultBiome_;

    // カメラズームに追従した画面 world half-extent を提供する
    private ScreenBounds screenBounds_;

    public override void Initialize() {
        defaultBiome_ = new Biome(defaultSpawnTable_);
        screenBounds_ = new ScreenBounds(ecsGroup, cameraName_);

        biomeAreas_.Clear();
        foreach (var name in biomeNames_) {
            Entity biomeEntity = ecsGroup.FindEntity(name);
            if (biomeEntity == null) continue;

            BiomeArea biomeArea = biomeEntity.GetScript<BiomeArea>();
            if (biomeArea == null) continue;

            biomeAreas_.Add(biomeArea);
        }
    }

    public override void Update() {
        if (!isActive_) return;

        spawnTimer_ += Time.deltaTime;

        // スポーンセルの重みをクリアする
        spawnCellWeights_.Clear();

        // スポーン可能な敵の総数が上限に達している場合はスポーンしない
        Entity heatMapEnt = ecsGroup.FindEntity(heatMapEntityName_);
        EnemyHeatMap heatMap = heatMapEnt?.GetScript<EnemyHeatMap>();
        if (heatMap == null) return;

        CalculateSpawnCellWeights(heatMap);

        // ズームに応じて敵数上限を線形補正する（指数1.0=線形、0=固定、2=面積比）。下限は1でクランプ。
        int effectiveMax = (int)(maxEnemyCount_ * Math.Pow(screenBounds_.ZoomRatio(), maxEnemyCountZoomExponent_));
        if (effectiveMax < 1) effectiveMax = 1;
        if (heatMap.GetEnemyCount() >= effectiveMax) return;

        int spawnCount = 0;
        // スポーン間隔を超えた場合、スポーン処理を行う
        while (spawnTimer_ > spawnInterval_ && spawnCount < maxSpawnCount_) {
            spawnTimer_ -= spawnInterval_;
            spawnCount++;

            // 敵の発生処理
            Vector2 spawnPos = CalculateSpawnPos(heatMap);
            Biome biome = ResolveBiome(spawnPos);
            if (biome == null) continue;

            string enemyType = biome.PickEnemyType();
            if (enemyType == null) continue;

            SpawnEnemy(enemyType, spawnPos);
        }
    }

    public override void OnCollisionStay(Entity collision) {
        DespawnTimer despawnTimer = collision.GetScript<DespawnTimer>();
        if (despawnTimer == null) return;
        despawnTimer.ResetTimer();
    }

    // スポーン地点に適用するバイオームを解決する。
    // biomeAreas_ をリスト順（＝優先度）に走査し、最初にヒットしたエリアの Biome を返す。
    // 該当エリアが無ければ defaultBiome_（defaultSpawnTable_）にフォールバックする。
    private Biome ResolveBiome(Vector2 spawnPos) {
        foreach (var biomeArea in biomeAreas_) {
            if (biomeArea.Contains(spawnPos)) {
                return biomeArea.GetBiome();
            }
        }
        return defaultBiome_;
    }

    // 敵のスポーン位置を計算する。セルを重みに応じて抽選し、そのセル範囲内のランダムな座標を返す。
    private Vector2 CalculateSpawnPos(EnemyHeatMap heatMap) {
        float total = 0.0f;
        foreach (var c in spawnCellWeights_) {
            if (c.weight > 0.0f) total += c.weight;
        }

        if (total <= 0.0f) {
            // 候補セルが無い場合は原点にスポーンさせる
            Vector2 originePos = new Vector2();
            Entity origineEntity = ecsGroup.FindEntity(originName_);
            if (origineEntity != null) {
                Transform origineT = origineEntity.GetComponent<Transform>();
                originePos = new Vector2(origineT.position.x, origineT.position.y);
            }
            return originePos;
        }

        // 浮動小数の誤差で最後まで抜けた場合に備え、末尾の候補をフォールバックにしておく
        Vector2Int selectedCellId = spawnCellWeights_[spawnCellWeights_.Count - 1].cellId;
        float r = RandomUtil.RandomRange(0.0f, total);
        foreach (var c in spawnCellWeights_) {
            if (c.weight <= 0.0f) continue;
            r -= c.weight;
            if (r <= 0.0f) {
                selectedCellId = c.cellId;
                break;
            }
        }

        // 選ばれたセルの範囲内でランダムな座標を返す
        Vector2 cellSize = heatMap.GetCellSize();
        Vector2 cellMin = new Vector2(selectedCellId.x * cellSize.x, selectedCellId.y * cellSize.y);
        Vector2 spawnPos = new Vector2(
            RandomUtil.RandomRange(cellMin.x, cellMin.x + cellSize.x),
            RandomUtil.RandomRange(cellMin.y, cellMin.y + cellSize.y)
        );

        if (!hasExclusionArea_) {
            return spawnPos;
        }

        // 境界セルは spawn/除外の両矩形をまたぐことがあるため、セル内一様サンプリングだけでは
        // 除外矩形内の座標を返してしまうことがある。除外矩形外に出るまで再サンプリングして塞ぐ。
        const int maxResampleCount = 8;
        bool accepted = !IsInsideExclusion(spawnPos);
        for (int i = 0; i < maxResampleCount && !accepted; i++) {
            spawnPos = new Vector2(
                RandomUtil.RandomRange(cellMin.x, cellMin.x + cellSize.x),
                RandomUtil.RandomRange(cellMin.y, cellMin.y + cellSize.y)
            );
            accepted = !IsInsideExclusion(spawnPos);
        }

        if (!accepted) {
            // 再サンプリングで避けられなかった場合、貫入量が最小の軸方向へ矩形外に押し出す。
            // (押し出しのみだと境界線上にスポーンが集中して不自然な帯になるため再サンプリングを優先する)
            spawnPos = PushOutOfExclusion(spawnPos);

            // exclusionAreaScale_ >= spawnAreaScale_ の誤設定時の保険として spawn 矩形内にクランプする
            spawnPos.x = Mathf.Clamp(spawnPos.x, spawnLo_.x, spawnHi_.x);
            spawnPos.y = Mathf.Clamp(spawnPos.y, spawnLo_.y, spawnHi_.y);
        }

        return spawnPos;
    }

    // 座標が除外矩形(exclusionMargin_ 込み)の内側かどうか
    private bool IsInsideExclusion(Vector2 pos) {
        return pos.x > exclusionLo_.x && pos.x < exclusionHi_.x &&
               pos.y > exclusionLo_.y && pos.y < exclusionHi_.y;
    }

    // 除外矩形内の座標を、貫入量(矩形の各辺までの距離)が最小の軸方向へ押し出す
    private Vector2 PushOutOfExclusion(Vector2 pos) {
        float distLeft = pos.x - exclusionLo_.x;
        float distRight = exclusionHi_.x - pos.x;
        float distBottom = pos.y - exclusionLo_.y;
        float distTop = exclusionHi_.y - pos.y;

        float minDist = distLeft;
        Vector2 result = pos;
        result.x = exclusionLo_.x;

        if (distRight < minDist) {
            minDist = distRight;
            result = pos;
            result.x = exclusionHi_.x;
        }
        if (distBottom < minDist) {
            minDist = distBottom;
            result = pos;
            result.y = exclusionLo_.y;
        }
        if (distTop < minDist) {
            minDist = distTop;
            result = pos;
            result.y = exclusionHi_.y;
        }

        return result;
    }

    private void SpawnEnemy(string enemyType, Vector2 spawnPos) {
        // 敵の生成処理
        Entity spawnedEnemy = ecsGroup.CreateEntity(enemyType);
        Transform enemyT = spawnedEnemy.GetComponent<Transform>();
        enemyT.position = new Vector3(spawnPos.x, spawnPos.y, 0.0f);
    }

    private void CalculateSpawnCellWeights(EnemyHeatMap heatMap) {
        // cell の候補を求める。
        Vector2 originePos = new Vector2();
        Entity origineEntity = ecsGroup.FindEntity(originName_);
        if (origineEntity != null) {
            Transform origineT = origineEntity.GetComponent<Transform>();
            originePos = new Vector2(origineT.position.x, origineT.position.y);
        }

        // 画面の world half-extent(カメラズーム追従)に係数を掛けて half-extent を求め、
        // origine 中心の world 矩形を直接作る。セル数への丸めを経由しないことで、
        // ズーム/プレイヤー移動に対して連続的に境界が動くようにする。
        Vector2 half = screenBounds_.HalfExtent();
        Vector2 spawnHalf = half * spawnAreaScale_;
        Vector2 exclusionHalf = half * exclusionAreaScale_;
        Vector2 marginVec = new Vector2(exclusionMargin_, exclusionMargin_);

        spawnLo_ = originePos - spawnHalf;
        spawnHi_ = originePos + spawnHalf;

        // 除外範囲が未設定(サイズ0)なら除外そのものを行わない。
        // 0 のまま矩形判定に入れると原点セルだけが不作為に除外されてしまうため。
        hasExclusionArea_ = exclusionHalf.x > 0.0f && exclusionHalf.y > 0.0f;
        exclusionLo_ = originePos - exclusionHalf - marginVec;
        exclusionHi_ = originePos + exclusionHalf + marginVec;

        // ループ範囲は world 角座標からセル ID を引いて求める。GetCellId が floor 化されているので、
        // これで spawn 矩形に交差する全セルを漏れなく覆える。
        Vector2Int ltCellId = heatMap.GetCellId(spawnLo_);
        Vector2Int rbCellId = heatMap.GetCellId(spawnHi_);

        Vector2 cellSize = heatMap.GetCellSize();

        // 2次元の畳み込みをそのまま行うと1セルあたり (2r+1)^2 回の参照が必要になるため、
        // カーネルを dx 方向 × dy 方向に分解できる形にし、横→縦の1次元畳み込み2回
        // (合計 2*(2r+1) 回の参照)に落とすことでループ回数を削減する。
        float kernelSum = ComputeKernelSum();

        // 横方向(x)の1次元畳み込み。縦方向のパスで参照するぶん、y は畳み込み半径ぶん広げて計算しておく。
        Dictionary<int, float> horizontalBlur = new Dictionary<int, float>();
        for (int y = ltCellId.y - convolutionRadius_; y <= rbCellId.y + convolutionRadius_; y++) {
            for (int x = ltCellId.x; x <= rbCellId.x; x++) {
                float sum = 0.0f;
                for (int dx = -convolutionRadius_; dx <= convolutionRadius_; dx++) {
                    sum += BaseWeight(heatMap, new Vector2Int(x + dx, y)) * KernelWeight(dx);
                }
                horizontalBlur[heatMap.CalclateHash(new Vector2Int(x, y))] = sum / kernelSum;
            }
        }

        for (int x = ltCellId.x; x <= rbCellId.x; x++) {
            for (int y = ltCellId.y; y <= rbCellId.y; y++) {
                // セルの world 矩形
                Vector2 cellMin = new Vector2(x * cellSize.x, y * cellSize.y);
                Vector2 cellMax = cellMin + cellSize;

                // spawn 矩形と交差しないセルはスキップする(floor 丸めで角に含まれた余剰セル)
                bool outsideSpawn = cellMax.x <= spawnLo_.x || cellMin.x >= spawnHi_.x ||
                                     cellMax.y <= spawnLo_.y || cellMin.y >= spawnHi_.y;
                if (outsideSpawn) continue;

                // 除外範囲に完全に内包されるセルだけをスキップする。またぐセルは候補に残す。
                if (hasExclusionArea_ &&
                    cellMin.x >= exclusionLo_.x && cellMax.x <= exclusionHi_.x &&
                    cellMin.y >= exclusionLo_.y && cellMax.y <= exclusionHi_.y) {
                    continue;
                }

                // 縦方向(y)の1次元畳み込み。横方向の結果を再利用する。
                float sum = 0.0f;
                for (int dy = -convolutionRadius_; dy <= convolutionRadius_; dy++) {
                    sum += horizontalBlur[heatMap.CalclateHash(new Vector2Int(x, y + dy))] * KernelWeight(dy);
                }

                Vector2Int cellId = new Vector2Int(x, y);
                spawnCellWeights_.Add(new WeightedCell { cellId = cellId, weight = sum / kernelSum });
            }
        }
    }

    // カーネル(距離が離れるほど影響を弱める)の総和。畳み込み結果の正規化に使う。
    private float ComputeKernelSum() {
        float sum = 0.0f;
        for (int d = -convolutionRadius_; d <= convolutionRadius_; d++) {
            sum += KernelWeight(d);
        }
        return sum;
    }

    // 1次元方向のカーネル重み。dx, dy それぞれに独立して適用することで2次元カーネルを分解する。
    private float KernelWeight(int d) {
        return 1.0f / (1.0f + Math.Abs(d));
    }

    // セル単体の基礎重み。敵の数が heatWeightThreshold_ 以上なら 0 にする
    private float BaseWeight(EnemyHeatMap heatMap, Vector2Int cellId) {
        int heat = heatMap.GetHeatByCellId(cellId);
        return (heat >= heatWeightThreshold_) ? 0.0f : 1.0f;
    }
}
