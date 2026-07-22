#include "DebugSceneGenerator.h"

#include <filesystem>
#include <fstream>
#include <string>

using namespace ONEngine;

void DebugSceneGenerator::GenerateDefaultDebugSceneIfNeeded() {
	std::string scenePath = "./Assets/Scene/Debug.scene";
	std::string camera3DPath = "./Assets/Scene/Debug/5777e8cd7fb89b14e6ba044b8f737e69.entity";
	std::string camera2DPath = "./Assets/Scene/Debug/476d1cadb33306de007290e0c4acae92.entity";

	if (!std::filesystem::exists(scenePath) ||
		!std::filesystem::exists(camera3DPath) ||
		!std::filesystem::exists(camera2DPath)) {
		std::filesystem::create_directories("./Assets/Scene/Debug");

		// Debug.scene の作成
		std::ofstream sceneFile(scenePath);
		if (sceneFile.is_open()) {
			sceneFile << R"({
    "entities": [
        {
            "guid": "5777e8cd7fb89b14e6ba044b8f737e69",
            "parentGuid": null,
            "path": "./Debug/5777e8cd7fb89b14e6ba044b8f737e69.entity"
        },
        {
            "guid": "476d1cadb33306de007290e0c4acae92",
            "parentGuid": null,
            "path": "./Debug/476d1cadb33306de007290e0c4acae92.entity"
        }
    ]
})";
			sceneFile.close();
		}

		// 3D DebugCamera entity の作成
		std::ofstream camera3DFile("./Assets/Scene/Debug/5777e8cd7fb89b14e6ba044b8f737e69.entity");
		if (camera3DFile.is_open()) {
			camera3DFile << R"({
    "active": true,
    "components": [
        {
            "cameraType": 0,
            "enable": 1,
            "farClip": 1000.0,
            "fogColor": {
                "x": 0.0,
                "y": 0.0,
                "z": 0.0
            },
            "fogEnd": 10000.0,
            "fogStart": 0.0,
            "fovY": 0.699999988079071,
            "isDrawFrustum": false,
            "isMainCamera": false,
            "nearClip": 0.10000000149011612,
            "type": "CameraComponent"
        },
        {
            "enable": 1,
            "matrixCalcFlags": 7,
            "position": {
                "x": -36.02433395385742,
                "y": 2.0796570777893066,
                "z": -117.26132202148438
            },
            "rotate": {
                "w": 0.9669580459594727,
                "x": 0.23663093149662018,
                "y": 0.09213835746049881,
                "z": -0.022547809407114983
            },
            "scale": {
                "x": 1.0,
                "y": 1.0,
                "z": 1.0
            },
            "type": "Transform"
        },
        {
            "DebugCamera": {
                "eulerAngles_": {
                    "x": 0.48000025749206543,
                    "y": 0.18999998271465302,
                    "z": 0.0
                },
                "isActive_": true,
                "moveSpeed_": 0.05000000074505806,
                "position_": {
                    "x": -34.11201477050781,
                    "y": -1.8702447414398193,
                    "z": -114.36476135253906
                },
                "velocity_": {
                    "x": 0.0,
                    "y": 0.0,
                    "z": 0.0
                }
            },
            "KingGeso": {
                "attackDuration": 100.0,
                "gesoMoveDuration": 10.25,
                "screenEdgeMargin": 128.5
            },
            "PlayerFollowCamera": {
                "maxSmoothSpeed_": 72.0
            },
            "type": "Variables"
        },
        {
            "enable": 1,
            "scripts": [
                {
                    "enable": false,
                    "name": "DebugCamera"
                }
            ],
            "type": "Script"
        }
    ],
    "guid": "5777e8cd7fb89b14e6ba044b8f737e69",
    "name": "DebugCamera",
    "parentGuid": null,
    "prefabName": ""
})";
			camera3DFile.close();
		}

		// 2D DebugCamera entity の作成
		std::ofstream camera2DFile("./Assets/Scene/Debug/476d1cadb33306de007290e0c4acae92.entity");
		if (camera2DFile.is_open()) {
			camera2DFile << R"({
    "active": true,
    "components": [
        {
            "cameraType": 1,
            "enable": 1,
            "farClip": 100.0,
            "fogColor": {
                "x": 0.0,
                "y": 0.0,
                "z": 0.0
            },
            "fogEnd": 10000.0,
            "fogStart": 0.0,
            "fovY": 0.6100000143051147,
            "isDrawFrustum": false,
            "isMainCamera": true,
            "nearClip": 0.4099999964237213,
            "type": "CameraComponent"
        },
        {
            "enable": 1,
            "matrixCalcFlags": 7,
            "position": {
                "x": -333.8069763183594,
                "y": -327.19378662109375,
                "z": 0.0
            },
            "rotate": {
                "w": 0.0,
                "x": 0.0,
                "y": 0.0,
                "z": 0.0
            },
            "scale": {
                "x": 0.12341528385877609,
                "y": 0.12341528385877609,
                "z": 0.12341528385877609
            },
            "type": "Transform"
        },
        {
            "DebugCamera2D": {
                "dragSensitivity_": 1.0,
                "isActive_": true,
                "moveSpeed_": 1.0,
                "position_": {
                    "x": -328.3455810546875,
                    "y": -85.59292602539063,
                    "z": 0.0
                },
                "scale_": {
                    "x": 0.1798168420791626,
                    "y": 0.1798168420791626,
                    "z": 0.1798168420791626
                },
                "velocity_": {
                    "x": 0.0,
                    "y": 0.0,
                    "z": 0.0
                }
            },
            "PlayerFollowCamera": {
                "maxSmoothSpeed_": 26.0,
                "smoothTime_": 0.8999999761581421
            },
            "type": "Variables"
        },
        {
            "enable": 1,
            "scripts": [
                {
                    "enable": true,
                    "name": "DebugCamera2D"
                }
            ],
            "type": "Script"
        }
    ],
    "guid": "476d1cadb33306de007290e0c4acae92",
    "name": "2DCamera",
    "parentGuid": null,
    "prefabName": ""
})";
			camera2DFile.close();
		}
	}
}
