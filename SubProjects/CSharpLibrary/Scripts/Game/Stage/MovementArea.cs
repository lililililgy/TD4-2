using System;
using System.Collections.Generic;

/// <summary>
/// 移動可能な範囲
/// </summary>
public class MovementArea : MonoScript {


    [SerializeField] private Vector2 min_ = Vector2.zero;
    [SerializeField] private Vector2 max_ = Vector2.zero;
    [SerializeField] private float radius = 0f;

    [SerializeField] public float test = 0f;
    public Vector2 Min { get { return min_; } }
    public Vector2 Max { get { return max_; } }


    public override void Initialize() {

    }

    public override void Update() {
        
    }

};