using System;
public class FlapjackOctopus : MonoScript
{

    /* ----- ÉpÉâÉÅÅ[É^ ----- */
    [SerializeField] private float chaseInterval = 1.0f;
    [SerializeField] private float chasePower = 5.0f;
    [SerializeField] private float chaseTime = 0.5f;
    // charge
    [SerializeField] private float chargeTime = 5.0f;
    [SerializeField] private float chargeBackDistance = 5.0f;
    // animation
    [SerializeField] private float chargeScaleAnimationTime = 0.5f;
    [SerializeField] private float chargeScaleRate = 0.5f;
    [SerializeField] private float chaseScaleAnimationTime = 0.5f;
    [SerializeField] private float chaseScaleRate = 0.5f;
    [SerializeField] private int chaseScaleAnimationCount = 5;


    public override void Initialize()
    {

    }

    public override void Update()
    {


    }

}
