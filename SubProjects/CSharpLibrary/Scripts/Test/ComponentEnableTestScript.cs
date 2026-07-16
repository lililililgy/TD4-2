using System;
using ONEngine;

public class ComponentEnableTestScript : MonoScript
{
    public override void Update()
    {
        var transform = entity.GetComponent<Transform>();
        if (transform) transform.enable = 0;
        
        var mesh = entity.GetComponent<MeshRenderer>();
        if (mesh) mesh.enable = 0;
        
        var dissolve = entity.GetComponent<DissolveMeshRenderer>();
        if (dissolve) dissolve.enable = 0;
        
        var sprite = entity.GetComponent<SpriteRenderer>();
        if (sprite) sprite.enable = 0;
        
        var text = entity.GetComponent<TextRenderer>();
        if (text) text.enable = 0;
        
        var col2d = entity.GetComponent<BoxCollider2D>();
        if (col2d) col2d.enable = 0;
        
        var camera = entity.GetComponent<CameraComponent>();
        if (camera) camera.enable = 0;
        
        var agent = entity.GetComponent<AgentIntentComponent>();
        if (agent) agent.enable = 0;
        
        var uiGroup = entity.GetComponent<UIGroupComponent>();
        if (uiGroup) uiGroup.enable = 0;
        
        var uiElement = entity.GetComponent<UIElementComponent>();
        if (uiElement) uiElement.enable = 0;
        
        var bgm = entity.GetComponent<BGMPlayer>();
        if (bgm) bgm.enable = 0;
        
        var se = entity.GetComponent<SEPlayer>();
        if (se) se.enable = 0;
        
        var colBox = entity.GetComponent<BoxCollider>();
        if (colBox) colBox.enable = 0;
        
        var colCircle = entity.GetComponent<CircleCollider>();
        if (colCircle) colCircle.enable = 0;
        
        var colSphere = entity.GetComponent<SphereCollider>();
        if (colSphere) colSphere.enable = 0;
        
        var anim = entity.GetComponent<AnimationPlayer>();
        if (anim) anim.enable = 0;
        
        var smr = entity.GetComponent<SkinMeshRenderer>();
        if (smr) smr.enable = 0;

        this.enable = false;
    }
}