using System;

public class SmallFryChaseMove : MonoScript
{
    private ChaseController chase_;

    public override void Initialize()
    {
        if (entity != null)
        {
            chase_ = entity.GetScript<ChaseController>();
        }
    }

    public void StartChase()
    {
        if (chase_ != null)
        {
            chase_.StartChase();
        }
    }
}
