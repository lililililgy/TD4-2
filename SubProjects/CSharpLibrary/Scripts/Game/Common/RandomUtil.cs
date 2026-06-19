
class RandomUtil
{
    static readonly System.Random random = new System.Random();

    public static float NextFloat()
    {
        return (float)random.NextDouble();
    }

    public static int NextInt()
    {
        return random.Next();
    }

    public static float NextFloat11()
    {
        return (float)(random.NextDouble() * 2.0 - 1.0);
    }

    public static Vector3 RandomVec3()
    {
        return new Vector3(NextFloat(), NextFloat(), NextFloat());
    }

    public static Vector2 RandomVec2()
    {
        return new Vector2(NextFloat(), NextFloat());
    }

    public static Vector3 RandomSphereSurface()
    {
        float cosTheta = NextFloat11();
        float sinTheta = Mathf.Sqrt(1 - cosTheta * cosTheta);
        float phi = NextFloat() * Mathf.PI * 2;
        return new Vector3(sinTheta * Mathf.Cos(phi), sinTheta * Mathf.Sin(phi), cosTheta);
    }

    public static Vector3 RandomSphereCap(Vector3 baseDirection, float maxAngle)
    {
        float theta = Mathf.Acos(1 - NextFloat() * (1 - Mathf.Cos(maxAngle)));
        float phi = NextFloat() * Mathf.PI * 2;
        float sinTheta = Mathf.Sin(theta);
        Quaternion rot = Quaternion.LookRotation(Vector3.right, baseDirection);
        Vector3 randVec = new Vector3(sinTheta * Mathf.Cos(phi), sinTheta * Mathf.Sin(phi), Mathf.Cos(theta));
        return  rot * randVec;
    }

    public static Vector3 RandomSphereInside()
    {
        Vector3 dir = RandomSphereSurface();
        float radius = Mathf.Pow(NextFloat(), 1.0f / 3.0f);
        return dir * radius;
    }

    public static Vector2 RandomCircumference()
    {
        float theta = NextFloat() * Mathf.PI * 2;
        return new Vector2(Mathf.Cos(theta), Mathf.Sin(theta));
    }

    public static Vector2 RandomCircle()
    {
        float radius = Mathf.Sqrt(NextFloat());
        return RandomCircumference() * radius;
    }
}
