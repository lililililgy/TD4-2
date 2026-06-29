using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

public class SpikeFishNeedle : MonoScript
{
    /* ----- パラメータ ----- */
    [SerializeField] private float firePower = 0.0f;
    [SerializeField] private float rotateZSpeed = 0.0f;
    [SerializeField] private float fireInterval = 0.0f;

    // スケーリング演出
    [SerializeField] private Vector3 fireShrinkScale = new Vector3(0.7f, 0.7f, 0.7f);
    [SerializeField] private Vector3 fireExpandScale = new Vector3(2.0f, 2.0f, 2.0f);
    [SerializeField] private float fireShrinkDuration = 0.1f;
    [SerializeField] private float fireExpandDuration = 0.2f;
    [SerializeField] private float fireReturnDuration = 0.2f;

    /* ----- 実行時状態 ----- */


    public override void Initialize()
    {



    }

    public override void Update()
    {

    }



}