//==========================================================
// キングクラゲの攻撃範囲予告表示
//==========================================================
public class JellyfishAttackTelegraph : MonoScript
{
    private SpriteRenderer renderer_;

    public override void Initialize()
    {
        ResolveRenderer();
    }

    public void ConfigureLine(
        Vector2 localOrigin,
        Vector2 direction,
        float length,
        float width,
        Vector4 color)
    {
        Vector2 normalized = direction.Normalized();
        if (normalized.LengthSq() <= 0.001f)
        {
            normalized = Vector2.up;
        }

        float safeLength = length > 0.0f ? length : 1.0f;
        float safeWidth = width > 0.0f ? width : 1.0f;
        Vector2 center = localOrigin + normalized * (safeLength * 0.5f);
        float angle = Mathf.Atan2(normalized.x, normalized.y);

        transform.position = new Vector3(center.x, center.y, 0.0f);
        transform.rotation = Quaternion.MakeFromAxis(Vector3.back, angle);
        transform.scale = new Vector3(safeWidth, safeLength, 1.0f);
        SetColor(color);
    }

    public void ConfigureArea(Vector2 localCenter, float radius, Vector4 color)
    {
        float diameter = radius > 0.0f ? radius * 2.0f : 1.0f;
        transform.position = new Vector3(localCenter.x, localCenter.y, 0.0f);
        transform.rotation = Quaternion.identity;
        transform.scale = new Vector3(diameter, diameter, 1.0f);
        SetColor(color);
    }

    private void SetColor(Vector4 color)
    {
        ResolveRenderer();
        if (renderer_ != null)
        {
            renderer_.color = color;
        }
    }

    private void ResolveRenderer()
    {
        if (renderer_ == null)
        {
            renderer_ = entity.GetComponent<SpriteRenderer>();
        }
    }
}
