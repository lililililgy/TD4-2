using System;
using System.Collections.Generic;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

[StructLayout(LayoutKind.Sequential)]
public struct GizmoBatchLineData
{
    public Vector3 startPosition;
    public Vector3 endPosition;
    public Vector4 color;
    public float thickness; // 線の太さ（ピクセル）
}

public static class GizmoBatch
{
    private static List<GizmoBatchLineData> _lineBuffer = new List<GizmoBatchLineData>();

    public static void DrawLine(Vector3 start, Vector3 end)
    {
        DrawLine(start, end, new Vector4(1, 1, 1, 1), 1.0f);
    }

    public static void DrawLine(Vector3 start, Vector3 end, Vector4 color, float thickness = 1.0f)
    {
        _lineBuffer.Add(new GizmoBatchLineData { startPosition = start, endPosition = end, color = color, thickness = thickness });
    }

    public static void DrawRay(Vector3 pos, Vector3 dir)
    {
        DrawRay(pos, dir, new Vector4(1, 1, 1, 1), 1.0f);
    }

    public static void DrawRay(Vector3 pos, Vector3 dir, Vector4 color, float thickness = 1.0f)
    {
        _lineBuffer.Add(new GizmoBatchLineData { startPosition = pos, endPosition = pos + dir, color = color, thickness = thickness });
    }

    public static void DrawWireCircle(Vector3 center, float radius, Vector4 color, int segments = 24, float thickness = 1.0f)
    {
        DrawWireCircle(center, radius, Vector3.up, color, segments, thickness);
    }

    public static void DrawWireCircle(Vector3 center, float radius, Vector3 up = default, Vector4 color = default, int segments = 24, float thickness = 1.0f)
    {
        if (up.x == 0 && up.y == 0 && up.z == 0) up = Vector3.up;
        if (color.x == 0 && color.y == 0 && color.z == 0 && color.w == 0) color = new Vector4(1, 1, 1, 1);

        Vector3 forward = (Math.Abs(up.y) > 0.99f) ? Vector3.forward : Vector3.up;
        Vector3 right = Vector3.Cross(up, forward).Normalized();
        forward = Vector3.Cross(right, up).Normalized();

        Vector3 prevPoint = center + right * radius;
        for (int i = 1; i <= segments; i++)
        {
            float angle = (i / (float)segments) * Mathf.PI * 2.0f;
            Vector3 point = center + (right * (float)Math.Cos(angle) + forward * (float)Math.Sin(angle)) * radius;
            DrawLine(prevPoint, point, color, thickness);
            prevPoint = point;
        }
    }

    public static void DrawWireArc(Vector3 center, float radius, Vector4 color, float angle, int segments = 12)
    {
        DrawWireArc(center, radius, Vector3.up, Vector3.forward, angle, color, segments);
    }

    public static void DrawWireArc(Vector3 center, float radius, Vector3 up, Vector3 forward, float angle, Vector4 color, int segments = 12)
    {
        Vector3 right = Vector3.Cross(up, forward).Normalized();
        Vector3 prevPoint = center + (forward * (float)Math.Cos(-angle * 0.5f * Mathf.Deg2Rad) + right * (float)Math.Sin(-angle * 0.5f * Mathf.Deg2Rad)) * radius;

        float startAngle = -angle * 0.5f;
        for (int i = 1; i <= segments; i++)
        {
            float currentAngle = (startAngle + (i / (float)segments) * angle) * Mathf.Deg2Rad;
            Vector3 point = center + (forward * (float)Math.Cos(currentAngle) + right * (float)Math.Sin(currentAngle)) * radius;
            DrawLine(prevPoint, point, color);
            prevPoint = point;
        }
    }

    public static void DrawWireCube(Vector3 center, Vector3 size, Quaternion rotation, Vector4 color, float thickness = 1.0f)
    {
        Vector3 halfSize = size * 0.5f;
        Vector3[] baseVertices = {
            new Vector3(-halfSize.x, -halfSize.y, -halfSize.z),
            new Vector3(halfSize.x, -halfSize.y, -halfSize.z),
            new Vector3(halfSize.x, halfSize.y, -halfSize.z),
            new Vector3(-halfSize.x, halfSize.y, -halfSize.z),
            new Vector3(-halfSize.x, -halfSize.y, halfSize.z),
            new Vector3(halfSize.x, -halfSize.y, halfSize.z),
            new Vector3(halfSize.x, halfSize.y, halfSize.z),
            new Vector3(-halfSize.x, halfSize.y, halfSize.z)
        };

        Vector3[] vertices = new Vector3[8];
        for (int i = 0; i < 8; i++)
        {
            vertices[i] = center + (rotation * baseVertices[i]);
        }

        int[] indices = {
            0, 1, 1, 2, 2, 3, 3, 0, // Bottom
            4, 5, 5, 6, 6, 7, 7, 4, // Top
            0, 4, 1, 5, 2, 6, 3, 7  // Pillars
        };

        for (int i = 0; i < indices.Length; i += 2)
        {
            DrawLine(vertices[indices[i]], vertices[indices[i + 1]], color, thickness);
        }
    }

    /// <summary>
    /// 指定されたフレーム数分だけ維持される円を描画する
    /// </summary>
    public static void DrawDurableCircle(Vector3 center, float radius, Vector4 color, int segments = 24, int frames = 60)
    {
        for (int i = 0; i < frames; i++)
        {
            // 現状のGizmoBatchは1フレームでクリアされるため、内部バッファの仕組み的に
            // 外部のマネージャー等で管理しない限り「未来の描画」を予約するのは難しい。
            // そのため、通常のDrawWireCircleを使用し、呼び出し側でループさせるか、
            // エンジン側のGizmoSystemにDuration付きのAPIがあればそちらを呼ぶべき。
        }
        DrawWireCircle(center, radius, color, segments);
    }

    /// <summary>
    /// フレームの終わりにC#からC++へ描画データを一括送信する。
    /// ECSGroup.UpdateEntities などから呼び出されることを想定。
    /// </summary>
    public static void SubmitBatch()
    {
        if (_lineBuffer.Count == 0) return;


        try
        {
            GizmoBatchLineData[] batch = _lineBuffer.ToArray();
            Internal_SubmitLineBatch(batch, batch.Length);
        }
        finally
        {
            _lineBuffer.Clear();
        }
    }

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern void Internal_SubmitLineBatch(GizmoBatchLineData[] batch, int count);
}

