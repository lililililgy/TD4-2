using System;
using ONEngine;

public class Rigidbody2DTestScript : MonoScript
{
    private Rigidbody2D rb;

    public override void Initialize()
    {
        rb = entity.GetComponent<Rigidbody2D>();
        System.Console.WriteLine($"[Rigidbody2DTestScript] Initialized on Entity {entity.Id}");
    }

    public override void Update()
    {
        if (rb && transform)
        {
            // 自動テストの安定化のため、固定の dt (1/60 秒) を用いて位置を更新する
            float fixedDt = 10.0f / 60.0f;
            Vector2 vel = rb.velocity;
            Vector3 pos = transform.position;
            pos.x += vel.x * fixedDt;
            pos.y += vel.y * fixedDt;
            transform.position = pos;
        }
    }
}
