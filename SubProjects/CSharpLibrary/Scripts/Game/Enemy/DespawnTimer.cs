using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;


public class DespawnTimer : MonoScript {
    [SerializeField] private float despawnTime_ = 10.0f; // デスポーンまでの時間（秒）
    private float timer_ = 0.0f;
    
    public override void Initialize() {
        timer_ = 0.0f;
    }

    public override void Update() {
        timer_ += Time.deltaTime;
        if (timer_ >= despawnTime_) {
            entity.Destroy();
        }
    }

    public void ResetTimer() {
        timer_ = 0.0f;
    }

}