import re
import os

def refactor_file(filepath):
    if not os.path.exists(filepath):
        print(f"File not found: {filepath}")
        return

    with open(filepath, 'r', encoding='utf-8') as f:
        content = f.read()

    # Pattern: \b_([a-z]\w*)\b
    # This matches words starting with _ followed by a lowercase letter.
    # We want to avoid matching things like __FILE__ or SAL annotations like _In_ (usually uppercase).
    
    # Let's find all such names to see if any are problematic
    potential_args = set(re.findall(r'\b_([a-z]\w*)\b', content))
    
    # List of common things to avoid if they appear with leading underscore
    # Actually, in this project's Core, it's very likely they are all arguments.
    avoid = {'_'} # just an underscore is sometimes used for ignored params

    new_content = content
    for arg_name in potential_args:
        if arg_name in avoid:
            continue
        
        old_full = '_' + arg_name
        new_full = arg_name
        
        # Replace only if it's a full word
        new_content = re.sub(r'\b' + re.escape(old_full) + r'\b', new_full, new_content)

    if new_content != content:
        with open(filepath, 'w', encoding='utf-8') as f:
            f.write(new_content)
        print(f"Refactored: {filepath}")
    else:
        print(f"No changes: {filepath}")

files = [
    r"C:\Users\k023g\source\repos\TD4-2\Project\Engine\Core\Window\WindowManager.h",
    r"C:\Users\k023g\source\repos\TD4-2\Project\Engine\Core\Window\WindowManager.cpp",
    r"C:\Users\k023g\source\repos\TD4-2\Project\Engine\Core\Window\Window.h",
    r"C:\Users\k023g\source\repos\TD4-2\Project\Engine\Core\Window\Window.cpp",
    r"C:\Users\k023g\source\repos\TD4-2\Project\Engine\Core\Utility\Utility.h",
    r"C:\Users\k023g\source\repos\TD4-2\Project\Engine\Core\Utility\Tools\StringHash.h",
    r"C:\Users\k023g\source\repos\TD4-2\Project\Engine\Core\Utility\Tools\Random.h",
    r"C:\Users\k023g\source\repos\TD4-2\Project\Engine\Core\Utility\Tools\Random.cpp",
    r"C:\Users\k023g\source\repos\TD4-2\Project\Engine\Core\Utility\Tools\Log.h",
    r"C:\Users\k023g\source\repos\TD4-2\Project\Engine\Core\Utility\Tools\Log.cpp",
    r"C:\Users\k023g\source\repos\TD4-2\Project\Engine\Core\Utility\Tools\Gizmo.h",
    r"C:\Users\k023g\source\repos\TD4-2\Project\Engine\Core\Utility\Tools\Gizmo.cpp",
    r"C:\Users\k023g\source\repos\TD4-2\Project\Engine\Core\Utility\Tools\Ease.h",
    r"C:\Users\k023g\source\repos\TD4-2\Project\Engine\Core\Utility\Tools\Ease.cpp",
    r"C:\Users\k023g\source\repos\TD4-2\Project\Engine\Core\Utility\Tools\Assert.h",
    r"C:\Users\k023g\source\repos\TD4-2\Project\Engine\Core\Utility\Time\Time.h",
    r"C:\Users\k023g\source\repos\TD4-2\Project\Engine\Core\Utility\Time\Time.cpp",
    r"C:\Users\k023g\source\repos\TD4-2\Project\Engine\Core\Utility\Time\CPUTimeStampID.h",
    r"C:\Users\k023g\source\repos\TD4-2\Project\Engine\Core\Utility\Time\CPUTimeStamp.h",
    r"C:\Users\k023g\source\repos\TD4-2\Project\Engine\Core\Utility\Time\CPUTimeStamp.cpp",
    r"C:\Users\k023g\source\repos\TD4-2\Project\Engine\Core\Utility\Math\Vector4T.h",
    r"C:\Users\k023g\source\repos\TD4-2\Project\Engine\Core\Utility\Math\Vector4.h",
    r"C:\Users\k023g\source\repos\TD4-2\Project\Engine\Core\Utility\Math\Vector3T.h",
    r"C:\Users\k023g\source\repos\TD4-2\Project\Engine\Core\Utility\Math\Vector3.h",
    r"C:\Users\k023g\source\repos\TD4-2\Project\Engine\Core\Utility\Math\Vector2T.h",
    r"C:\Users\k023g\source\repos\TD4-2\Project\Engine\Core\Utility\Math\Vector2.h",
    r"C:\Users\k023g\source\repos\TD4-2\Project\Engine\Core\Utility\Math\Quaternion.h",
    r"C:\Users\k023g\source\repos\TD4-2\Project\Engine\Core\Utility\Math\Quaternion.cpp",
    r"C:\Users\k023g\source\repos\TD4-2\Project\Engine\Core\Utility\Math\Primitive.h",
    r"C:\Users\k023g\source\repos\TD4-2\Project\Engine\Core\Utility\Math\Primitive.cpp",
    r"C:\Users\k023g\source\repos\TD4-2\Project\Engine\Core\Utility\Math\Matrix4x4.h",
    r"C:\Users\k023g\source\repos\TD4-2\Project\Engine\Core\Utility\Math\Matrix4x4.cpp",
    r"C:\Users\k023g\source\repos\TD4-2\Project\Engine\Core\Utility\Math\Math.h",
    r"C:\Users\k023g\source\repos\TD4-2\Project\Engine\Core\Utility\Math\Math.cpp",
    r"C:\Users\k023g\source\repos\TD4-2\Project\Engine\Core\Utility\Math\Interpolation.h",
    r"C:\Users\k023g\source\repos\TD4-2\Project\Engine\Core\Utility\Math\Color.h",
    r"C:\Users\k023g\source\repos\TD4-2\Project\Engine\Core\Utility\Math\Color.cpp",
    r"C:\Users\k023g\source\repos\TD4-2\Project\Engine\Core\Utility\Input\Mouse.h",
    r"C:\Users\k023g\source\repos\TD4-2\Project\Engine\Core\Utility\Input\Mouse.cpp",
    r"C:\Users\k023g\source\repos\TD4-2\Project\Engine\Core\Utility\Input\Keyboard.h",
    r"C:\Users\k023g\source\repos\TD4-2\Project\Engine\Core\Utility\Input\Keyboard.cpp",
    r"C:\Users\k023g\source\repos\TD4-2\Project\Engine\Core\Utility\Input\InputSystem.h",
    r"C:\Users\k023g\source\repos\TD4-2\Project\Engine\Core\Utility\Input\InputSystem.cpp",
    r"C:\Users\k023g\source\repos\TD4-2\Project\Engine\Core\Utility\Input\Input.h",
    r"C:\Users\k023g\source\repos\TD4-2\Project\Engine\Core\Utility\Input\Input.cpp",
    r"C:\Users\k023g\source\repos\TD4-2\Project\Engine\Core\Utility\Input\Gamepad.h",
    r"C:\Users\k023g\source\repos\TD4-2\Project\Engine\Core\Utility\Input\Gamepad.cpp",
    r"C:\Users\k023g\source\repos\TD4-2\Project\Engine\Core\Utility\FileSystem\FileSystem.h",
    r"C:\Users\k023g\source\repos\TD4-2\Project\Engine\Core\Utility\FileSystem\FileSystem.cpp",
    r"C:\Users\k023g\source\repos\TD4-2\Project\Engine\Core\Threading\ThreadPool.h",
    r"C:\Users\k023g\source\repos\TD4-2\Project\Engine\Core\Threading\ThreadPool.cpp",
    r"C:\Users\k023g\source\repos\TD4-2\Project\Engine\Core\Config\EngineConfig.h",
    r"C:\Users\k023g\source\repos\TD4-2\Project\Engine\Core\Config\EngineConfig.cpp"
]

for f in files:
    refactor_file(f)
