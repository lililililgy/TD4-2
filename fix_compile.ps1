$ecsPath = "Project\Engine\ECS\EntityComponentSystem\EntityComponentSystem.cpp"
$content = Get-Content $ecsPath -Raw

# Remove trailing garbage
$content = $content -replace '(?s)void ONEngine::MonoInternalMethods::Internal_OnBreakpointHit\(uint32_t nodeIdHash\) \{.*?\}.*', "void ONEngine::MonoInternalMethods::Internal_OnBreakpointHit(uint32_t nodeIdHash) {`r`n    // ブレークポイントヒット時にゲームを一時停止（デバッグ用）`r`n    ONEngine::DebugConfig::isPause = true; `r`n}`r`n"

# Replace remaining groupName variables
$content = $content -replace 'uint64_t MonoInternalMethods::InternalAddComponent\(int32_t entityId, MonoString\* monoTypeName, MonoString\* groupName, uint32_t\* compId\) \{.*?\r\n\tstd::string groupName = mono_string_to_utf8\(groupName\);', "uint64_t MonoInternalMethods::InternalAddComponent(int32_t entityId, MonoString* monoTypeName, MonoString* groupName, uint32_t* compId) {`r`n`tstd::string groupNameStr = mono_string_to_utf8(groupName);"
$content = $content -replace 'GameEntity\* entity = GetEntityById\(entityId, groupName\);', 'GameEntity* entity = GetEntityById(entityId, groupNameStr);'

$content = $content -replace 'uint64_t MonoInternalMethods::InternalGetComponent\(int32_t entityId, MonoString\* monoTypeName, MonoString\* groupName, uint32_t\* compId\) \{.*?\r\n\tstd::string groupName = mono_string_to_utf8\(groupName\);', "uint64_t MonoInternalMethods::InternalGetComponent(int32_t entityId, MonoString* monoTypeName, MonoString* groupName, uint32_t* compId) {`r`n`tstd::string groupNameStr = mono_string_to_utf8(groupName);"

$content = $content -replace 'bool MonoInternalMethods::InternalGetScript\(int32_t entityId, MonoString\* scriptName, MonoString\* groupName\) \{.*?\r\n\tstd::string groupName = mono_string_to_utf8\(groupName\);', "bool MonoInternalMethods::InternalGetScript(int32_t entityId, MonoString* scriptName, MonoString* groupName) {`r`n`tstd::string groupNameStr = mono_string_to_utf8(groupName);"
$content = $content -replace '\r\n\tstd::string scriptName\(cstr\);', "`r`n`tstd::string scriptNameStr(cstr);"
$content = $content -replace 'script->Contains\(scriptName\)', "script->Contains(scriptNameStr)"
$content = $content -replace 'Console::Log\(std::format\(\"Script \{\} found for Entity ID: \{\}\", scriptName, entityId\)\);', 'Console::Log(std::format("Script {} found for Entity ID: {}", scriptNameStr, entityId));'

$content = $content -replace 'void MonoInternalMethods::InternalCreateEntity\(int32_t\* entityId, MonoString\* prefabName, MonoString\* groupName\) \{.*?\r\n\tstd::string groupName = mono_string_to_utf8\(groupName\);.*?\r\n\tstd::string prefabName = mono_string_to_utf8\(prefabName\);', "void MonoInternalMethods::InternalCreateEntity(int32_t* entityId, MonoString* prefabName, MonoString* groupName) {`r`n`tstd::string groupNameStr = mono_string_to_utf8(groupName);`r`n`tstd::string prefabNameStr = mono_string_to_utf8(prefabName);"
$content = $content -replace 'Console::LogInfo\(\"\[SOURCE_DETECTOR\] C# requested CreateEntity: Prefab = \" \+ prefabName\);', 'Console::LogInfo("[SOURCE_DETECTOR] C# requested CreateEntity: Prefab = " + prefabNameStr);'
$content = $content -replace 'ECSGroup\* group = gECS->GetECSGroup\(groupName\);', 'ECSGroup* group = gECS->GetECSGroup(groupNameStr);'
$content = $content -replace 'GameEntity\* entity = group->GenerateEntityFromPrefab\(prefabName\);', 'GameEntity* entity = group->GenerateEntityFromPrefab(prefabNameStr);'
$content = $content -replace 'Console::LogWarning\(\"\[SOURCE_DETECTOR\] Prefab not found by name, creating blank entity: \" \+ prefabName\);', 'Console::LogWarning("[SOURCE_DETECTOR] Prefab not found by name, creating blank entity: " + prefabNameStr);'

$content = $content -replace 'int32_t MonoInternalMethods::InternalGetRootEntityCount\(MonoString\* groupName\) \{.*?\r\n\tstd::string groupName = mono_string_to_utf8\(groupName\);', "int32_t MonoInternalMethods::InternalGetRootEntityCount(MonoString* groupName) {`r`n`tstd::string groupNameStr = mono_string_to_utf8(groupName);"
$content = $content -replace 'int32_t MonoInternalMethods::InternalGetRootEntityId\(MonoString\* groupName, int32_t index\) \{.*?\r\n\tstd::string groupName = mono_string_to_utf8\(groupName\);', "int32_t MonoInternalMethods::InternalGetRootEntityId(MonoString* groupName, int32_t index) {`r`n`tstd::string groupNameStr = mono_string_to_utf8(groupName);"


[System.IO.File]::WriteAllText($ecsPath, $content, [System.Text.Encoding]::UTF8)

$scriptPath = "Project\Engine\ECS\Component\Components\ComputeComponents\Script\Script.cpp"
$content = Get-Content $scriptPath -Raw
$content = $content -replace 'script->RemoveScript\(script.scriptName\);', 'script->RemoveScript(scriptData.scriptName);'
[System.IO.File]::WriteAllText($scriptPath, $content, [System.Text.Encoding]::UTF8)

$smrPath = "Project\Engine\ECS\Component\Components\RendererComponents\SkinMesh\SkinMeshRenderer.cpp"
$content = Get-Content $smrPath -Raw
$content = $content -replace 'smr->SetMeshPath\(path\);', 'smr->SetMeshPath(pathStr);'
[System.IO.File]::WriteAllText($smrPath, $content, [System.Text.Encoding]::UTF8)
