#include "WorldTransformEx.h"


using namespace KamataEngine;
using namespace MathUtility; // Make~Matrix Matrix4x4同士の積(*)の利用

// Scale, Rotation, Translationの順に行列から　World行列を計算
void WorldTransformEx::UpdateMatrix() {
	// ワールド変換行列の生成
	matWorld_ = MakeAffineMatrix();
	// 定数バッファへ転送
	TransferMatrix();
}

//アフィン変換行列を作る
Matrix4x4 WorldTransformEx::MakeAffineMatrix() {
	// スケーリング行列
	Matrix4x4 matScale = MakeScaleMatrix(scale_);
	// X軸回転行列
	Matrix4x4 matRotX = MakeRotateXMatrix(rotation_.x);
	// Y軸回転行列
	Matrix4x4 matRotY = MakeRotateYMatrix(rotation_.y);
	// Z軸回転行列
	Matrix4x4 matRotZ = MakeRotateZMatrix(rotation_.z);
	Matrix4x4 matRot = matRotZ * matRotX * matRotY;
	// Translate Matrix
	Matrix4x4 matTrans = MakeTranslateMatrix(translation_);

	//WorldMatrix
	Matrix4x4 matWorld = matScale * matRot * matTrans;

	// ワールド変換行列の計算
	return matWorld;
}