public class KingGesoHomingProjectile : MonoScript
{
    [SerializeField] public float speed = 120.0f;
    [SerializeField] public float turnSpeed = 2.0f;
    [SerializeField] public float lifeTime = 8.0f;
    [SerializeField] public float damage = 1.0f;

    private Entity _target;
    private Vector3 _direction = Vector3.up;
    private float _elapsed;
    private bool _launched;

    public override void Initialize()
    {
        _target = null;
        _direction = Vector3.up;
        _elapsed = 0.0f;
        _launched = false;
    }

    public bool CommandLaunch(Entity target)
    {
        if (target == null || target.transform == null)
        {
            return false;
        }

        _target = target;
        Vector3 toTarget = _target.transform.worldPosition - transform.worldPosition;
        if (toTarget.LengthSq() > 0.0001f)
        {
            _direction = toTarget.Normalized();
        }

        _launched = true;
        ApplyRotation();

        AttackCollision attack = entity.GetScript<AttackCollision>();
        if (attack != null)
        {
            attack.Damage = damage;
        }
        return true;
    }

    public override void Update()
    {
        if (!_launched)
        {
            return;
        }

        _elapsed += Time.deltaTime;
        if (_elapsed >= lifeTime)
        {
            entity.Destroy();
            return;
        }

        if (_target != null && _target.transform != null)
        {
            Vector3 toTarget = _target.transform.worldPosition - transform.worldPosition;
            if (toTarget.LengthSq() > 0.0001f)
            {
                Quaternion current = Quaternion.LookRotation(-Vector3.forward, _direction);
                Quaternion desired = Quaternion.LookRotation(-Vector3.forward, toTarget.Normalized());
                float ratio = Mathf.Clamp(turnSpeed * Time.deltaTime, 0.0f, 1.0f);
                _direction = Quaternion.RotateVector(Quaternion.Slerp(current, desired, ratio), Vector3.up).Normalized();
            }
        }

        transform.position += _direction * speed * Time.deltaTime;
        ApplyRotation();
    }

    public override void OnCollisionEnter(Entity collision)
    {
        if (_launched && collision != null && collision.Id != entity.Id)
        {
            entity.Destroy();
        }
    }

    private void ApplyRotation()
    {
        float roll = Mathf.Atan2(_direction.x, _direction.y);
        transform.rotate = Quaternion.MakeFromAxis(Vector3.back, roll);
    }
}
