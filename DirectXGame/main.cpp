#include <KamataEngine.h>
#include <Windows.h>
// #include <d3dcompiler.h>
#include "Shader.h"
#include "RootSignature.h"
#include "PipelineState.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "WorldTransformEx.h"
#include <cassert>

using namespace KamataEngine;
using namespace Microsoft::WRL;

//関数プロトタイプ宣言
void SetupPipelineState(PipelineState& pipelineState, RootSignature& rs, Shader& vs, Shader& ps) {
	// InputLayout
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[2] = {};
	inputElementDescs[0].SemanticName = "POSITION"; // 頂点シェーダー側で定義したセマンティクスと同じ名前を指定する
	inputElementDescs[0].SemanticIndex = 0;
	inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT; // Vector3の形式
	inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	inputElementDescs[1].SemanticName = "TEXCOORD"; // 頂点シェーダー側で定義したセマンティクスと同じ名前を指定する
	inputElementDescs[1].SemanticIndex = 0;
	inputElementDescs[1].Format = DXGI_FORMAT_R32G32_FLOAT; // Vector2の形式
	inputElementDescs[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
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

	// PSO(PipelineStateObject)の作成-----------------------
	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};
	graphicsPipelineStateDesc.pRootSignature = rs.Get();                                                    // RootSignature
	graphicsPipelineStateDesc.InputLayout = inputLayoutDesc;                                                // InputLayout
	graphicsPipelineStateDesc.VS = {vs.GetDxcBlob()->GetBufferPointer(), vs.GetDxcBlob()->GetBufferSize()}; // VertexShader
	graphicsPipelineStateDesc.PS = {ps.GetDxcBlob()->GetBufferPointer(), ps.GetDxcBlob()->GetBufferSize()}; // PixelShader
	graphicsPipelineStateDesc.BlendState = blendDesc;          // BlendState
	graphicsPipelineStateDesc.RasterizerState = rasterizerDesc;  // RasterizerState

	// 書き込むRTVの情報
	graphicsPipelineStateDesc.NumRenderTargets = 1;                            // 1つのRTVに書き込む
	graphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB; // RTVのフォーマット
	// 利用するトポロジ(形状)のタイプ	。三角形
	graphicsPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	// どのように画面に色を打ち込むかの設定
	graphicsPipelineStateDesc.SampleDesc.Count = 1;
	graphicsPipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	// 準備は整った。PSOの生成
	pipelineState.Create(graphicsPipelineStateDesc);
}

//RenderTargetResourceの生成
ID3D12Resource* CreateRenderTextureResource(ID3D12Device* device, int32_t width, int32_t height,DXGI_FORMAT format, const FLOAT* clearColor) {
	// DirectXCommonインスタンスの取得
	//DirectXCommon* dxCommon = DirectXCommon::GetInstance();
	// RenderTargetResourceの生成
	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Width = UINT(width);                                   // 幅
	resourceDesc.Height = UINT(height);                                 // 高さ
	resourceDesc.MipLevels = 1;                                   // ミップマップレベル数
	resourceDesc.DepthOrArraySize = 1;                            // 配列サイズ
	resourceDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;        //TextureのFormat
	resourceDesc.SampleDesc.Count = 1;                            // サンプル数
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;  // テクスチャの次元数。普段使っているのは2次元
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET; // レンダーターゲットとして利用可能

	//2.利用するHeapの設定
	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT; //VRAM上に作成する

	//3.ClearValueの用意
	D3D12_CLEAR_VALUE clearValue{};
	clearValue.Format = format;
	clearValue.Color[0] = clearColor[0];
	clearValue.Color[1] = clearColor[1];
	clearValue.Color[2] = clearColor[2];
	clearValue.Color[3] = clearColor[3];

	// RenderTargetResourceの生成
	ID3D12Resource* resource = nullptr;
	[[maybe_unused]] HRESULT hr =
	    device->CreateCommittedResource(
	    &heapProperties, 
		D3D12_HEAP_FLAG_NONE, 
		&resourceDesc,
	    D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
	    &clearValue, 
		IID_PPV_ARGS(&resource)
	);
	assert(SUCCEEDED(hr));
	return resource;
}

// DepthStencilResourceの生成
ID3D12Resource* CreateDepthStencilTextureResource(ID3D12Device* device, int32_t width, int32_t height) {
	// DepthStencilResourceの生成
	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Width = width;   // 幅
	resourceDesc.Height = height; // 高さ
	resourceDesc.MipLevels = 1;         // ミップマップレベル数
	resourceDesc.DepthOrArraySize = 1;  // 配列サイズ
	resourceDesc.Format = DXGI_FORMAT_D32_FLOAT;       // TextureのFormat
	resourceDesc.SampleDesc.Count = 1;  // サンプル数
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;  // テクスチャの次元数。普段使っているのは2次元
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL; // デプスステンシルとして利用可能

	// 2.利用するHeapの設定
	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT; // VRAM上に作成する

	//深度値のクリア設定
	D3D12_CLEAR_VALUE depthClearValue{};
	depthClearValue.DepthStencil.Depth = 1.0f; // 深度値の初期値
	depthClearValue.Format = DXGI_FORMAT_D32_FLOAT;


	// 3.Resourceの生成
	ID3D12Resource* resource = nullptr;
	[[maybe_unused]] HRESULT hr = device->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&depthClearValue,
		IID_PPV_ARGS(&resource)
	);
	assert(SUCCEEDED(hr));

	return resource;
}

// シェーダーコンパイル関数



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
	RootSignature rs;
	rs.Create();

	

	//// コンパイル済みのShader、エラー時情報の格納場所の用意
	// 頂点シェーダーの読み込みとコンパイル
	Shader vs;
	vs.LoadDxc(L"Resources/shaders/TestVS.hlsl", L"vs_6_0");
	assert(vs.GetDxcBlob() != nullptr);

	// ピクセルシェーダーの読み込みとコンパイル
	Shader ps;
	ps.LoadDxc(L"Resources/shaders/TestPS.hlsl", L"ps_6_0");
	assert(ps.GetDxcBlob() != nullptr);

	PipelineState pipelineState;
	SetupPipelineState(pipelineState, rs, vs, ps);



	//リソースの確保含め、頂点情報を柔軟に対応できるようにVertexData構造体を新たに作成する
	//Vertex4 ⇒VertexDataに変更して利用する
	struct VertexData {
		Vector4 position;
		Vector2 texcoord;
	};

	//頂点データの準備 ★00_07 追加
	VertexData vertices[] = {
	    // position                         // uv
	    {{-1.0f, 1.0f, 0.0f, 1.0f},  {0.0f, 0.0f}}, // 左上
	    {{1.0f, 1.0f, 0.0f, 1.0f},   {1.0f, 0.0f}}, // 右上
	    {{-1.0f, -1.0f, 0.0f, 1.0f}, {0.0f, 1.0f}}, // 左下
	    {{1.0f, -1.0f, 0.0f, 1.0f},  {1.0f, 1.0f}}, // 右下
	};

	VertexBuffer vb;

	vb.Create(sizeof(vertices), sizeof(vertices[0]));

	// 頂点リソースにデータを書き込む
	VertexData* pGpuVertices = nullptr;
	vb.Get()->Map(0, nullptr, reinterpret_cast<void**>(&pGpuVertices));

	for (int i = 0; i < _countof(vertices); ++i) {
		pGpuVertices[i] = vertices[i];
	}

	uint16_t indices[] = {
	    0, 1, 2,
		2, 1, 3,
	};

	// IndexBuffer(IndexResource, IndexResourceView)の生成
	IndexBuffer ib;
	ib.Create(sizeof(indices), sizeof(indices[0]));

	// 頂点インデックスリソースにデータを書き込む
	uint16_t* pGpuIndices = nullptr;

	ib.Get()->Map(0, nullptr, reinterpret_cast<void**>(&pGpuIndices));

	for (int i = 0; i < _countof(indices); ++i) {
		pGpuIndices[i] = indices[i];
	}


	//------------------------------------------------------------
	// Resource生成、Heap生成、View生成 で再利用される変数の準備
	//------------------------------------------------------------

	ID3D12Device* device = dxCommon->GetDevice();
	HRESULT hr;

	//------------------------------------------------------------
	// RenderTexture関係　　★00_09 追加
	//------------------------------------------------------------

	//------------------------------------------------------------
	// 0. RenderTextureResourceの作成
	//------------------------------------------------------------

	// 画面クリア色 ※分かりやすいように赤とする
	const FLOAT kRenderTargetClearColor[4] = {1.0f, 0.0f, 0.0f, 1.0f};

	ID3D12Resource* renderTextureResource = CreateRenderTextureResource(
		device, WinApp::kWindowWidth, 
		WinApp::kWindowHeight, 
		DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
		kRenderTargetClearColor);


	//------------------------------------------------------------
	// 1. RTV用の DescriptorHeapを作成する
	//------------------------------------------------------------

	ID3D12DescriptorHeap* rtvDescriptorHeap = nullptr;

	D3D12_DESCRIPTOR_HEAP_DESC rtvDescriptorHeapDesc{};
	rtvDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV; // RTV
	rtvDescriptorHeapDesc.NumDescriptors = 1;                    // Descriptorの個数は1

	hr = device->CreateDescriptorHeap(&rtvDescriptorHeapDesc, IID_PPV_ARGS(&rtvDescriptorHeap));
	assert(SUCCEEDED(hr));

	// CPU側からみたHANDLEを取得しておく
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandleCPU = rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();


	//------------------------------------------------------------
	// 2. RTV用の Viewの生成
	//------------------------------------------------------------

	device->CreateRenderTargetView(
	    renderTextureResource, // Viewと関連付けたいリソース
	    nullptr,               // RTVの詳細情報(Desc)
	                           // ※RTVの場合 nullptr にするとDirectX12が自動で推測してくれる
	    rtvHandleCPU           // RTV用ディスクリプタヒープのCPU Handle
	);


	//------------------------------------------------------------
	// DepthStencilTexture関係　　★00_09 追加
	//------------------------------------------------------------

	//------------------------------------------------------------
	// 0. DepthStencilTextureResourceの作成
	//------------------------------------------------------------

	ID3D12Resource* depthStencilResource = CreateDepthStencilTextureResource(device, WinApp::kWindowWidth, WinApp::kWindowHeight);


	//------------------------------------------------------------
	// 1. DSV用の DescriptorHeapを作成
	//------------------------------------------------------------

	ID3D12DescriptorHeap* dsvDescriptorHeap = nullptr;

	D3D12_DESCRIPTOR_HEAP_DESC dsvDescriptorHeapDesc{};
	dsvDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;   // Heap Type
	dsvDescriptorHeapDesc.NumDescriptors = 1;                      // Heap Typeの個数
	dsvDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE; // DSVはShaderで触らないとする

	hr = device->CreateDescriptorHeap(&dsvDescriptorHeapDesc, IID_PPV_ARGS(&dsvDescriptorHeap));
	assert(SUCCEEDED(hr));

	// CPU側からみたHANDLEを取得しておく
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandleCPU = dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();


	//------------------------------------------------------------
	// 2. DSV用の Viewの生成
	//------------------------------------------------------------

	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};

	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;                // 基本的にResourceに合わせる
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D; // 2D Texture

	// DSVHeapの先頭にDSVを作る
	device->CreateDepthStencilView(depthStencilResource, &dsvDesc, dsvHandleCPU);


	//=============================================================================
	// SRV(Shader Resource View)を準備する  ※ PixelShaderと連携をとるようにするため  ★00_09 追加

	//-----------------------------------------------------------------------------
	// 1. SRV用の DescriptorHeapの作成
	ID3D12DescriptorHeap* srvDescriptorHeap = nullptr;

	D3D12_DESCRIPTOR_HEAP_DESC srvDescriptorHeapDesc = {};
	srvDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;     // SRV
	srvDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE; // PixelShader から見える
	srvDescriptorHeapDesc.NumDescriptors = 1;

	hr = device->CreateDescriptorHeap(&srvDescriptorHeapDesc, IID_PPV_ARGS(&srvDescriptorHeap));
	assert(SUCCEEDED(hr));

	// CPU側からみたHANDLE、GPU側からみたHANDLEを取得しておく
	D3D12_CPU_DESCRIPTOR_HANDLE srvHandleCPU = srvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	D3D12_GPU_DESCRIPTOR_HANDLE srvHandleGPU = srvDescriptorHeap->GetGPUDescriptorHandleForHeapStart();

	//-----------------------------------------------------------------------------
	// 2. SRV(Shader Resource View)の作成
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;                           // RenderTargetResource と同じにする
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING; // RGBA値をそのまま Shaderに対応させる
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;                      // 2Dテクスチャ
	srvDesc.Texture2D.MipLevels = 1;                                            // MipLevel は 1 しかない

	device->CreateShaderResourceView(
	    renderTextureResource, // Viewと関連付けたいリソース
	    &srvDesc,              // SRVの詳細情報(Desc:Description、構成内容の記述)
	    srvHandleCPU           // SRV用ディスクリプタヒープの CPU Handle
	);


	//アプリで利用する3Dモデル=================================================--
	//被写体の準備
	Model* model = Model::CreateFromOBJ("terrain");

	WorldTransformEx worldTransform;
	worldTransform.Initialize();
	worldTransform.scale_ = Vector3(1.0f, 1.0f, 1.0f);

	//カメラの準備
	Camera camera;
	camera.Initialize();
	camera.translation_ = Vector3(0.0f, 1.0f, 0.0f);


	// メインループ
	while (true) {
		// エンジンの更新
		if (Update()) {
			break;
		}

		//world変換行列の定数バッファへの転送
		worldTransform.rotation_.y += 0.005f; //適当な回転角度(ラジアン)
		worldTransform.UpdateMatrix();

		camera.UpdateMatrix();

		// 描画開始




		dxCommon->PreDraw();
		// // TransitionBarrierを SRV ⇒ RTV に設定する
		D3D12_RESOURCE_BARRIER barrier{};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;                       // TransitionBarrierの設定
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;                            // フラグは None にしておく
		barrier.Transition.pResource = renderTextureResource;                        // バリアを張る対象のリソース
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE; // 遷移前
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;          // 遷移後
		commandList->ResourceBarrier(1, &barrier);                                   // バリアを張る

		// // 描画先の RTV と DSV を設定する
		commandList->OMSetRenderTargets(1, &rtvHandleCPU, false, &dsvHandleCPU);

		// // Viewportの設定
		D3D12_VIEWPORT viewport{};
		viewport.Width = WinApp::kWindowWidth;
		viewport.Height = WinApp::kWindowHeight;
		viewport.TopLeftX = 0;
		viewport.TopLeftY = 0;
		viewport.MinDepth = 0.0f; // 深度の最小値
		viewport.MaxDepth = 1.0f; // 深度の最大値

		commandList->RSSetViewports(1, &viewport);

		// // Scissorの設定
		D3D12_RECT scissorRect{};
		// // 基本的にビューポートと同じ矩形が構成されるようにする
		scissorRect.left = 0;
		scissorRect.right = WinApp::kWindowWidth;
		scissorRect.top = 0;
		scissorRect.bottom = WinApp::kWindowHeight;

		commandList->RSSetScissorRects(1, &scissorRect);

		// // 全画面クリア
		commandList->ClearRenderTargetView(rtvHandleCPU, kRenderTargetClearColor, 0, nullptr);
		// // 指定した深度で画面全体をクリアする
		commandList->ClearDepthStencilView(dsvHandleCPU, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

		//描画
		Model::PreDraw();
		model->Draw(worldTransform, camera);
		Model::PostDraw();

		// =========================================================================
		//  ここにゲームの 3Dシーン の描画処理を置く ※次回
		// =========================================================================

		// // TransitionBarrierを元に戻し、PixelShaderが扱えるようにする
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;                      // TransitionBarrierの設定
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;                           // フラグは None にしておく
		barrier.Transition.pResource = renderTextureResource;                       // バリアを張る対象のリソース
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;        // 遷移前
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE; // 遷移後
		commandList->ResourceBarrier(1, &barrier);                                  // バリアを張る

		dxCommon->PreDraw();

		// コマンドを積む
		commandList->SetGraphicsRootSignature(rs.Get());       // RootSignatureの設定
		commandList->SetPipelineState(pipelineState.Get()); // PSOの設定
		commandList->IASetVertexBuffers(0, 1, vb.GetView());   // VBVの設定
		commandList->IASetIndexBuffer(ib.GetView()); //★IBVを設定する

		// トポロジの設定
		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		// 使用するディスクリプターヒープの設定
		commandList->SetDescriptorHeaps(srvDescriptorHeap->GetDesc().NumDescriptors, &srvDescriptorHeap);

		// SRVののDescriptorTableのGPU側のハンドルをセットする
		commandList->SetGraphicsRootDescriptorTable(0, srvHandleGPU);


		// 頂点数、インデックス数、インデックスの開始位置、インデックスのオフセット
		//commandList->DrawInstanced(3, 1, 0, 0);
		commandList->DrawIndexedInstanced(_countof(indices), 1, 0, 0, 0);

		// 描画終了
		dxCommon->PostDraw();
	}

	delete model;

	// 解放処理
	renderTextureResource->Release();
	srvDescriptorHeap->Release();
	rtvDescriptorHeap->Release();

	depthStencilResource->Release();
	dsvDescriptorHeap->Release();

	// エンジンの終了処理
	Finalize();

	return 0;
}
