#pragma once
#include <d3d12.h>
#include <string>
#include <d3dcompiler.h>
#include <dxcapi.h>

class Shader {

public:
	// シェーダーファイルを読み込み、コンパイル済みのデータを生成する
	void Load(const std::wstring& filepath, const std::wstring& shaderModel);
	void LoadDxc(const std::wstring& filepath, const std::wstring& shaderModel);

	// 生成したコンパイル済みのデータを取得する
	ID3DBlob* GetBlob();
	IDxcBlob* GetDxcBlob();

	// コンストラクタ
	Shader();
	// デストラクタ
	~Shader();

private:
	ID3DBlob* blob_ = nullptr; // コンストラクタで初期化しなくていい　※C++11以降
	IDxcBlob* dxcBlob_ = nullptr; // コンパイル済みのシェーダーデータ　※外部コンパイル版
};
