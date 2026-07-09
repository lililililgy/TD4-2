using System;
using System.Collections.Generic;

/// <summary>
/// 移動可能な範囲
/// </summary>
public class MovementArea : MonoScript {
    [SerializeField] private bool fixedCollisionBounds_ = false; // min/max に Colliderの大きさを反映するかどうか
    [SerializeField] private Vector2 min_ = Vector2.zero;
    [SerializeField] private Vector2 max_ = Vector2.zero;

    public Vector2 Min { get { return min_; } }
    public Vector2 Max { get { return max_; } }

    public override void Update() {
        if (!fixedCollisionBounds_) {
            return;
        }

        Transform transform = entity.GetComponent<Transform>();
        BoxCollider2D boxCollider2D = entity.GetComponent<BoxCollider2D>();

        if (transform == null || boxCollider2D == null) {
            return;
        }

        Vector2 halfSize = new Vector2(transform.scale.x * boxCollider2D.size.x, transform.scale.y * boxCollider2D.size.y) * 0.5f;

        min_ = new Vector2(transform.position.x - halfSize.x, transform.position.y - halfSize.y);
        max_ = new Vector2(transform.position.x + halfSize.x, transform.position.y + halfSize.y);
    }

};