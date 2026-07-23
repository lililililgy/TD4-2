internal sealed class KingGesoInkBarrageAttack : IKingGesoAttack
{
    private float waveElapsed_;
    private float recoveryElapsed_;
    private float currentAngle_;
    private int firedWaveCount_;
    private int rotationDirection_;

    public void Enter(KingGeso owner)
    {
        waveElapsed_ = 0.0f;
        recoveryElapsed_ = 0.0f;
        currentAngle_ = 0.0f;
        firedWaveCount_ = 0;
        rotationDirection_ = 1;

        FireWave(owner);
    }

    public bool Update(KingGeso owner)
    {
        if (firedWaveCount_ < owner.InkBulletWaveCount)
        {
            waveElapsed_ += Time.deltaTime;
            float interval = owner.InkBulletWaveInterval;
            while (firedWaveCount_ < owner.InkBulletWaveCount && waveElapsed_ >= interval)
            {
                waveElapsed_ -= interval;
                FireWave(owner);
            }
            return false;
        }

        recoveryElapsed_ += Time.deltaTime;
        return recoveryElapsed_ >= owner.InkBarrageRecoveryDuration;
    }

    public void Exit(KingGeso owner)
    {
    }

    private void FireWave(KingGeso owner)
    {
        int bulletCount = owner.InkBulletCountPerWave;
        float angleStep = Mathf.PI * 2.0f / bulletCount;
        bool firedAny = false;

        for (int i = 0; i < bulletCount; i++)
        {
            float angle = currentAngle_ + angleStep * i;
            Vector2 direction = new Vector2(Mathf.Cos(angle), Mathf.Sin(angle));
            if (owner.FireInkBullet(direction))
            {
                firedAny = true;
            }
        }

        if (firedAny)
        {
            owner.PlayInkBulletWaveSe();
        }

        firedWaveCount_++;
        if (owner.ReverseInkBarrageHalfway &&
            firedWaveCount_ >= (owner.InkBulletWaveCount + 1) / 2)
        {
            rotationDirection_ = -1;
        }

        currentAngle_ += owner.InkBulletAngleOffset * rotationDirection_;
    }
}
