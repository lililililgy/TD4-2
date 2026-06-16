#include "Shader.h"

using namespace ONEngine;

/// std
#include <vector>

/// engine
#include "ShaderCompiler.h"
#include "Shader.h"

Shader::Shader() = default;
Shader::~Shader() = default;

void Shader::Initialize(ShaderCompiler* compiler) {
	pShaderCompiler_ = compiler;
}

bool Shader::CompileShader(const std::wstring& filePath, const wchar_t* profile, Type type, const std::wstring& entryPoint) {
	/// ----- Typeごとにコンパイル結果を保存 ----- ///

	ComPtr<IDxcBlob> shader = pShaderCompiler_->CompileShader(filePath, profile, entryPoint);

	switch (type) {
	case Shader::Type::vs:
		vs_ = shader;
		return true;
	case Shader::Type::ps:
		ps_ = shader;
		return true;
	case Shader::Type::cs:
		cs_ = shader;
		return true;
	case Shader::Type::ms:
		ms_ = shader;
		return true;
	case Shader::Type::as:
		as_ = shader;
		return true;
	}

	return false;
}
