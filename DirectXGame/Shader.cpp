#include "Shader.h"
#include <d3dcompiler.h> //D3DCompileFromFile
#include <cassert>

// コンパイル済みのシェーダーを返す ※未コンパイルの場合はnullptr

ID3DBlob* Shader::GetBlob() { 
	return blob_;

}

//コンストラクタ
Shader::Shader() {

}

// デストラクタ
Shader::~Shader() {
	if (blob_ != nullptr) {
		blob_->Release();
		blob_ = nullptr;
	}
}

void Shader::Load(const std::wstring& filepath, const std::string& shaderModel) {
	ID3DBlob* shaderBlob = nullptr;
	ID3DBlob* errorBlob = nullptr;


	HRESULT hr = D3DCompileFromFile(
	    filepath.c_str(), // シェーダーファイル名
	    nullptr,
	    D3D_COMPILE_STANDARD_FILE_INCLUDE,               // インクルード可能にする
	    "main", shaderModel.c_str(),                     // エントリーポイント名、シェーダーモデル指定
	    D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, // デバッグ用設定
	    0, &shaderBlob, &errorBlob);
	// エラーが発生した場合、止める
	if (FAILED(hr)) {
		if (errorBlob) {
			OutputDebugStringA(reinterpret_cast<const char*>(errorBlob->GetBufferPointer()));
			errorBlob->Release();
		}
		assert(false);
	}
	// 生成したshaderBlobをとっておく
	blob_ = shaderBlob;
}


//コンパイル済みのシェーダーを