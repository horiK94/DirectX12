struct PSInput
{
	//頂点シェーダーと同じ構造体を定義. ヘッダファイル(.hlsli)を作成して共通の構造体を定義するのが良い
	float4 Position : SV_POSITION;	//頂点座標
	float4 Color : COLOR;			//頂点カラー
};

struct PSOutput
{
	//出力先のレンダーターゲットのピクセル要素に合わせる. DXGI_FORMAT_R8G8B8A8_UNORM_SRGB
	//レンダーターゲット配列の0番目に書き込むのでSV_TARGET0を指定する. 1番目ならSV_TARGET1, 2番目ならSV_TARGET2, ...
	float4 color : SV_TARGET0;		//出力色
};

float4 main() : SV_TARGET
{
	return float4(1.0f, 1.0f, 1.0f, 1.0f);
}

//ピクセルシェーダーのエントリーポイント
PSOutput main(PSInput input)
{
	PSOutput output = (PSOutput)0;

	output.color = input.Color;

	return output;
}