using System.Collections.Generic;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

/// <summary>
/// デバッグ描画用ギズモ（線）をフレーム単位でまとめてC++へ送信するユーティリティ。
/// ネイティブ束縛: GizmoBatch::Internal_SubmitLineBatch (AddInternalMethods.cpp)
/// </summary>
public static class GizmoBatch {
    /// <summary>C++ 側 ONEngine::Gizmo::LineData と同一レイアウト。</summary>
    [StructLayout(LayoutKind.Sequential)]
    public struct LineData {
        public Vector3 startPosition;
        public Vector3 endPosition;
        public Vector4 color;
        public float thickness;
    }

    private static readonly List<LineData> lines_ = new List<LineData>();

    public static void DrawLine(Vector3 start, Vector3 end, Vector4 color, float thickness = 1.0f) {
        lines_.Add(new LineData {
            startPosition = start,
            endPosition = end,
            color = color,
            thickness = thickness,
        });
    }

    /// <summary>貯めた線をC++へ一括送信しバッファをクリアする。</summary>
    public static void SubmitBatch() {
        if (lines_.Count == 0) return;
        LineData[] batch = lines_.ToArray();
        Internal_SubmitLineBatch(batch, batch.Length);
        lines_.Clear();
    }

    [MethodImpl(MethodImplOptions.InternalCall)]
    static extern void Internal_SubmitLineBatch(LineData[] batch, int count);
}
