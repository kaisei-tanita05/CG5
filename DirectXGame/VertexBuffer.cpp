#include "VertexBuffer.h"
#include <KamataEngine.h>

#include <d3d12.h>	//ID3D~, D3D~
#include <cassert>	//assert

using namespace KamataEngine;

//生成
void VertexBuffer::Create(const UINT size, const UINT stride) {
	//クラス内でdxCommonを利用するために追加
	DirectXCommon* dxCommon = DirectXCommon::GetInstance();

	//頂点リソースの生成======================================
	//頂点リソース用のヒープの設定
	D3D12_HEAP_PROPERTIES uploadHeapProperties{};
	uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD; // CPUから書き込むヒープ
	//頂点リソースの設定
	D3D12_RESOURCE_DESC vertexResourceDesc{};
	vertexResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER; // リソースの種類はバッファ
	vertexResourceDesc.Width = size;                                // リソースのサイズ、引数sizeで受け取った値
	//バッファの場合はこれらは1にする決まり
	vertexResourceDesc.Height = 1;
	vertexResourceDesc.DepthOrArraySize = 1;
	vertexResourceDesc.MipLevels = 1;
	vertexResourceDesc.SampleDesc.Count = 1;
	//バッファの場合はこれにする決まり
	vertexResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	//実際に頂点リソースを生成する
	ID3D12Resource* vertexResource = nullptr;

	//★HRESULT　追加
	[[maybe_unused]] HRESULT hr = dxCommon->GetDevice()->CreateCommittedResource(
	    &uploadHeapProperties, // ヒープ設定
	    D3D12_HEAP_FLAG_NONE,
	    &vertexResourceDesc,               // リソース設定
	    D3D12_RESOURCE_STATE_GENERIC_READ, // GPU読み込み可能状態
	    nullptr, IID_PPV_ARGS(&vertexResource));
	assert(SUCCEEDED(hr));//うまくいかなかったときは起動できない

	//生成した頂点リソースをとっておく
	vertexBuffer_= vertexResource;

	// VertexBufferViewの作成======================================
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};
	//リソースの先頭アドレスから使う
	vertexBufferView.BufferLocation = vertexBuffer_->GetGPUVirtualAddress();
	//使用するリソースのサイズは頂点size分のサイズ
	vertexBufferView.SizeInBytes = size; //頂点リソースの全サイズ
	//1つの頂点サイズ
	vertexBufferView.StrideInBytes = stride; //頂点1つ分のサイズ

	//vertexBufferViewを取っておく
	vertexBufferView_ = vertexBufferView;
}

ID3D12Resource* VertexBuffer::Get() { return vertexBuffer_; }

D3D12_VERTEX_BUFFER_VIEW* VertexBuffer::GetView() { return &vertexBufferView_; }

//コンストラクタ
VertexBuffer::VertexBuffer() {}

//デストラクタ
VertexBuffer::~VertexBuffer() {
	if (vertexBuffer_) {
		vertexBuffer_->Release();
		vertexBuffer_ = nullptr;
	}
}