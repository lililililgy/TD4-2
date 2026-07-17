using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

public class ExperiencePoint : MonoScript
{
    [SerializeField] private float experiencePoints;

    public float ExperiencePoints
    {
        get { return experiencePoints; }
        set { experiencePoints = value; }
    }
}