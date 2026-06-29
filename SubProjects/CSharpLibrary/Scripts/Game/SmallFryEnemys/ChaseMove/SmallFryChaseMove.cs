using System;

public class SmallFryChaseMove : MonoScript
{
    private ChaseController chase_;

    public override void Initialize()
    {
        chase_ = entity.GetScript<ChaseController>();
    }

    public void StartChase()
    {
        chase_.StartChase();
    }
}
