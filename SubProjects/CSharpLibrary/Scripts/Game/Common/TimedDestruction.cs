using System;

/// <summary>
/// 一定時間経過後にエンティティを破棄するスクリプト。
/// </summary>
public class TimedDestruction : MonoScript {
    [SerializeField]
    public float lifeTime = 5.0f;

    [SerializeField]
    public float remainingTime = 0.0f;

    private float timer = 0.0f;

    public override void Initialize() {
        timer = 0.0f;
        remainingTime = lifeTime;
    }

    public override void Update() {
        timer += Time.deltaTime;
        remainingTime = Math.Max(0.0f, lifeTime - timer);
        if (timer >= lifeTime) {
            entity.Destroy();
        }
    }
}
