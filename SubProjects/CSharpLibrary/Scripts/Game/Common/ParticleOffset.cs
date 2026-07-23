using System;

/// <summary>
/// この敵が生成するパーティクルの発生位置オフセットを、敵ごとにインスペクタで調整するためのスクリプト。
/// </summary>
public class ParticleOffset : MonoScript
{
    [SerializeField] public Vector3 offset = Vector3.zero;
}
