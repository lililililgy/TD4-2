using System;

public class ZoomWeight : MonoScript
{
    [SerializeField] private float weight_ = 1.0f; // 0〜1 の強さ。大きいほどカメラに寄る

    public float Weight => weight_;
}
