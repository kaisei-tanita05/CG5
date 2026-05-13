#include <KamataEngine.h>
#include <Windows.h>
#include <d3dcompiler.h>

using namespace KamataEngine;
using namespace Microsoft::WRL;

// ComPtr<ID3D12Resource> CreateRenderTextureResource(ComPtr<ID3D12Device> device, uint32_t width, uint32_t height, DXGI_FORMAT format, const Vector4& clearColor) {
//	D3D12_RESOURCE_DESC resourceDesc{};
//	resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;//RenderTargetとして利用可能にする
//
//	D3D12_HEAP_PROPERTIES heapProperties{};
//	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT; // 当然VRAM状に作る
//
//	// ClearValueの設定
//	D3D12_CLEAR_VALUE clearValue{};
//	clearValue.Format = format;
//	clearValue.Color[0] = clearColor.x;
//	clearValue.Color[1] = clearColor.y;
//	clearValue.Color[2] = clearColor.z;
//	clearValue.Color[3] = clearColor.w;
//
//	ComPtr<ID3D12Resource> resource = nullptr;
//
//	device->CreateCommittedResource(
//		&heapProperties,
//		D3D12_HEAP_FLAG_NONE,
//		&resourceDesc,
//		D3D12_RESOURCE_STATE_RENDER_TARGET, // RenderTargetとして利用するので、初期状態はRenderTarget
//		&clearValue, // ClearValueを指定
//		IID_PPV_ARGS(&resource));
//	return resource;
//
//	int kClientWidth = 1280;
//	int kClientHeight = 720;
//
//	const Vector4 kRenderTargetClearValue{1.0f, 0.0f, 0.0f, 1.0f}; // 一旦分かりやすいように赤色でクリアするようにしてみる
//	auto renderTextureResource = CreateRenderTextureResource(device, kClientWidth, kClientHeight, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, kRenderTargetClearValue);
// }

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {

	// エンジンの初期化
	Initialize(L"LE3D_17_タニタ_カイセイ");

	// DirectXCommonインスタンスの取得
	DirectXCommon* dxCommon = DirectXCommon::GetInstance();

	// DirectXCommonクラスが管理している、ウィンドウの幅と高さを取得
	int32_t w = dxCommon->GetBackBufferWidth();
	int32_t h = dxCommon->GetBackBufferHeight();
	DebugText::GetInstance()->ConsolePrintf(std::format("width: {}, height: {}\n", w, h).c_str());

	// DirectXCommonクラスが管理している、コマンドリストの取得
	ComPtr<ID3D12GraphicsCommandList> commandList = dxCommon->GetCommandList();

	// RootSignatureの作成----------------------------------------------
	// 後でSpriteCommonクラスを作るときに、RootSignatureもまとめて管理する
	// 構造体にデータを用意する
	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
	descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT; // InputLayoutを利用するためのフラグ
	ID3DBlob* signatureBlob = nullptr;
	ID3DBlob* errorBlog = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&descriptionRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlog);
	if (FAILED(hr)) {
		DebugText::GetInstance()->ConsolePrintf(reinterpret_cast<char*>(errorBlog->GetBufferPointer()));
		assert(false);
	}

	
	// バイナリをもとに生成
	ID3D12RootSignature* rootSignature = nullptr;
	hr = dxCommon->GetDevice()->CreateRootSignature(
		0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), 
		IID_PPV_ARGS(&rootSignature));
	assert(SUCCEEDED(hr));

	// InputLayout
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[1] = {};
	inputElementDescs[0].SemanticName = "POSITION"; // 頂点シェーダー側で定義したセマンティクスと同じ名前を指定する
	inputElementDescs[0].SemanticIndex = 0;
	inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT; // Vector3の形式
	inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
	inputLayoutDesc.pInputElementDescs = inputElementDescs;
	inputLayoutDesc.NumElements = _countof(inputElementDescs);

	// BlendState-------------------------------------今回は不透明
	D3D12_BLEND_DESC blendDesc{};
	// すべての色要素を書き込む
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	// RasterizerState-----------------------------------
	D3D12_RASTERIZER_DESC rasterizerDesc{};
	// 裏面をカリングする
	rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	// 塗りつぶしモードをリソッドにする(ワイヤーフレームになら D3D12_FILL_MODE_WIREFRAME)
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;

	// コンパイル済みのShader、エラー時情報の格納場所の用意
	ComPtr<ID3DBlob> vsBlob = nullptr;    // 頂点シェーダーオブジェクト
	ComPtr <ID3DBlob> psBlob = nullptr; // ピクセルシェーダーオブジェクト
	ID3DBlob* errorBlob = nullptr; // エラー情報オブジェクト

	std::wstring vsFile = L"Resources/shaders/TestVS.hlsl";
	hr = D3DCompileFromFile(
		vsFile.c_str(), 
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"main", "vs_5_0", 
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 
		0, &vsBlob, &errorBlob);
	if (FAILED(hr)) {
		DebugText::GetInstance()->ConsolePrintf(std::system_category().message(hr).c_str());
		if (errorBlob) {
			DebugText::GetInstance()->ConsolePrintf(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
		}
		assert(false);
	}

	// 頂点シェーダーの読み込みとコンパイル
	std::wstring psFile = L"Resources/shaders/TestPS.hlsl";
	hr = D3DCompileFromFile(
	    psFile.c_str(), // シェーダーファイル名
	    nullptr,
	    D3D_COMPILE_STANDARD_FILE_INCLUDE // インクルード可能にする
	    ,
	    "main", "ps_5_0",                                // エントリーポイント名、シェーダーモデル指定
	    D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, // デバッグ用設定
	    0, &psBlob, &errorBlob);
	if (FAILED(hr)) {
		DebugText::GetInstance()->ConsolePrintf(std::system_category().message(hr).c_str());
		if (errorBlob) {
			DebugText::GetInstance()->ConsolePrintf(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
		}
		assert(false);
	}



	// PSO(PipelineStateObject)の作成-----------------------
	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};
	graphicsPipelineStateDesc.pRootSignature = rootSignature;                             // RootSignature
	graphicsPipelineStateDesc.InputLayout = inputLayoutDesc;                              // InputLayout
	graphicsPipelineStateDesc.VS = {vsBlob->GetBufferPointer(), vsBlob->GetBufferSize()}; // VertexShader
	graphicsPipelineStateDesc.PS = {psBlob->GetBufferPointer(), psBlob->GetBufferSize()}; // PixelShader
	graphicsPipelineStateDesc.BlendState = blendDesc;                                     // BlendState
	graphicsPipelineStateDesc.RasterizerState = rasterizerDesc;                           // RasterizerState
	// 書き込むRTVの情報
	graphicsPipelineStateDesc.NumRenderTargets = 1;                            // 1つのRTVに書き込む
	graphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB; // RTVのフォーマット
	// 利用するトポロジ(形状)のタイプ	。三角形
	graphicsPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	// どのように画面に色を打ち込むかの設定
	graphicsPipelineStateDesc.SampleDesc.Count = 1;
	graphicsPipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	// 準備は整った。PSOの生成
	ComPtr<ID3D12PipelineState> graphicsPipelineState = nullptr;
	hr = dxCommon->GetDevice()->CreateGraphicsPipelineState(&graphicsPipelineStateDesc, IID_PPV_ARGS(&graphicsPipelineState));
	assert(SUCCEEDED(hr));

	// VertexResourceの生成
	// 頂点リソース用のヒープの設定
	D3D12_HEAP_PROPERTIES uploadHeapProperties{};
	uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD; // CPUから書き込むためのヒープ
	// 頂点リソースの設定
	D3D12_RESOURCE_DESC vertexResourceDesc{};
	vertexResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER; // バッファ
	vertexResourceDesc.Width = sizeof(Vector4) * 3;                 // リソースのサイズ。今回はVector4の頂点が3つ分
	// バッファの場合はこれら1にする決まり
	vertexResourceDesc.Height = 1;
	vertexResourceDesc.DepthOrArraySize = 1;
	vertexResourceDesc.MipLevels = 1;
	vertexResourceDesc.SampleDesc.Count = 1;
	// バッファの場合はこれにする決まり
	vertexResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	// 実際に頂点リソースを生成
	ID3D12Resource* vertexResource = nullptr;
	hr = dxCommon->GetDevice()->CreateCommittedResource(&uploadHeapProperties, D3D12_HEAP_FLAG_NONE, &vertexResourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&vertexResource));
	assert(SUCCEEDED(hr));

	// VertexBufferViewの作成------------------------
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};
	// リソースの先頭からアドレスを使う
	vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
	// 使用するリソースのサイズは頂点3つ分のサイズ
	vertexBufferView.SizeInBytes = sizeof(Vector4) * 3;
	// 1つの頂点のサイズ
	vertexBufferView.StrideInBytes = sizeof(Vector4);

	// 頂点リソースにデータを書き込む
	Vector4* vertexData = nullptr;
	vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	vertexData[0] = {-0.5f, -0.5f, 0.0f, 1.0f}; // 左下
	vertexData[1] = {0.0f, 0.5f, 0.0f, 1.0f};   // 上
	vertexData[2] = {0.5f, -0.5f, 0.0f, 1.0f};  // 右下
	// 頂点リソースのマップを削除する
	vertexResource->Unmap(0, nullptr);

	// メインループ
	while (true) {
		// エンジンの更新
		if (Update()) {
			break;
		}

		// 描画開始
		dxCommon->PreDraw();

		// コマンドを積む
		commandList->SetGraphicsRootSignature(rootSignature);       // RootSignatureの設定
		commandList->SetPipelineState(graphicsPipelineState.Get()); // PSOの設定
		commandList->IASetVertexBuffers(0, 1, &vertexBufferView);   // VBVの設定
		// トポロジの設定
		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		// 頂点数、インデックス数、インデックスの開始位置、インデックスのオフセット
		commandList->DrawInstanced(3, 1, 0, 0);

		// 描画終了
		dxCommon->PostDraw();
	}

	// 解放処理
	vertexResource->Release();
	signatureBlob->Release();
	if (errorBlob) {
		errorBlob->Release();
	}
	rootSignature->Release();
	//vsBlob->Release();
	//psBlob->Release();

	// エンジンの終了処理
	Finalize();

	return 0;
}