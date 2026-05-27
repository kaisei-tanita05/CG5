#pragma once
#include <d3d12.h>
#include <string>

class Shader {

public:
	// シェーダーファイルを読み込み、コンパイル済みのデータを生成する
	void Load(const std::wstring& filepath, const std::string& shaderModel);

	// 生成したコンパイル済みのデータを取得する
	ID3DBlob* GetBlob();

	// コンストラクタ
	Shader();
	// デストラクタ
	~Shader();

private:
	ID3DBlob* blob_ = nullptr; // コンストラクタで初期化しなくていい　※C++11以降
};
