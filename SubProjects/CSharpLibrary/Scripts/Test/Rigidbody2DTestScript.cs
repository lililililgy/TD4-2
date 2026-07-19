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
            // C# 側で Rigidbody2D の速度に基づいて位置を更新する
            Vector2 vel = rb.velocity;
            Vector3 pos = transform.position;
            pos.x += vel.x * Time.deltaTime;
            pos.y += vel.y * Time.deltaTime;
            transform.position = pos;
        }
    }
}
