#include "PipelineState.h"
#include <KamataEngine.h>
#include <cassert>

using namespace KamataEngine;

// PipelineStateを生成する
void PipelineState::Create(D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStatedesc) {
	// クラス内で取得するために追加
	DirectXCommon* dxCommon = DirectXCommon::GetInstance();

	ID3D12PipelineState* graphicsPipelineState = nullptr;
	[[maybe_unused]] HRESULT hr = dxCommon->GetDevice()->CreateGraphicsPipelineState(&graphicsPipelineStatedesc, IID_PPV_ARGS(&graphicsPipelineState));
	assert(SUCCEEDED(hr));

	// 生成したPipelineStateをとっておく
	pipelineState_ = graphicsPipelineState;
}

// 生成したPipelineStateを返す
ID3D12PipelineState* PipelineState::Get() { return pipelineState_; }

// コンストラクタ
PipelineState::PipelineState() {}

// デストラクタ
PipelineState::~PipelineState() {
	if (pipelineState_) {
		pipelineState_->Release();
		pipelineState_ = nullptr;
	}
}