using Newtonsoft.Json.Linq;
using System;
using System.IO;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

public class MonoScript {
    ///////////////////////////////////////////////////////////////////////////////////////////
    /// objects
    ///////////////////////////////////////////////////////////////////////////////////////////

    /// Behaviorの生成
    public void CreateBehavior(int entityId, string name, ECSGroup ecsGroup) {
        if (!ecsGroup) {
            Debug.LogError("MonoBehavior.CreateBehavior - ECSGroup is null. Cannot create MonoBehavior for Entity ID: " + entityId);
            return;
        }

        name_ = name;
        this.ecsGroup = ecsGroup;
        entity = this.ecsGroup.GetEntity(entityId);

        Debug.Log("MonoBehavior created for Entity ID: " + entityId + ", Script Name: " + name + ", Group Name: " + ecsGroup.groupName);
    }


    /// この behavior が所属するECSGroup
    public ECSGroup ecsGroup {
        get; internal set;
    }

    private string name_;
    public bool enable = true;

    public Entity entity {
        get; internal set;
    }

    public Transform transform {
        get {
            if (entity == null) {
                Debug.LogError("MonoBehavior.transform - Entity is not initialized. Please call InternalInitialize first.");
                return null;
            }

            if (entity.transform == null) {
                Debug.LogError("MonoBehavior.transform - Transform component is not initialized for Entity ID: " + entity.Id);
                return null;
            }

            return entity.transform;
        }
    }



    ///////////////////////////////////////////////////////////////////////////////////////////
    /// methods
    ///////////////////////////////////////////////////////////////////////////////////////////

    public virtual void Awake() { }
    public virtual void Initialize() { }
    public virtual void Update() { }

    public virtual void OnCollisionEnter(Entity collision) { }
    public virtual void OnCollisionExit(Entity collision) { }
    public virtual void OnCollisionStay(Entity collision) { }

    ///////////////////////////////////////////////////////////////////////////////////////////
    /// operators
    ///////////////////////////////////////////////////////////////////////////////////////////
    public static implicit operator bool(MonoScript monoBehavior) {
        return monoBehavior != null;
    }

}
