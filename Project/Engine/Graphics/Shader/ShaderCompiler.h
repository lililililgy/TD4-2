#pragma once

/// windows
#include <Windows.h>

/// directx
#include <wrl/client.h>
#include <dxcapi.h>

/// std
#include <string>

/// engine
#include "Engine/Core/DirectX12/ComPtr/ComPtr.h"


/// ///////////////////////////////////////////////////
/// hlslシェーダーコンパイラー
/// ///////////////////////////////////////////////////
namespace ONEngine {

class ShaderCompiler final {
public:

	/// ===================================================
	/// public : methods
	/// ===================================================

	ShaderCompiler();
	~ShaderCompiler();

	void Initialize();	

	/// @brief HLSLシェーダーのコンパイル
	/// @param filePath HLSLファイルのパス
	/// @param profile HLSLプロファイル
	/// @param entryPoint エントリーポイント関数名 
	/// @return コンパイル後のシェーダーブロブ
	ComPtr<IDxcBlob> CompileShader(const std::wstring& filePath, const wchar_t* profile, const std::wstring& entryPoint);


private:

	/// ===================================================
	/// private : objects
	/// ===================================================

	ComPtr<IDxcUtils>          dxcUtils_       = nullptr;
	ComPtr<IDxcCompiler3>      dxcCompiler_    = nullptr;
	ComPtr<IDxcIncludeHandler> includeHandler_ = nullptr;

};


} /// ONEngine
