struct VSInput		//頂点データとして渡されるデータ
{
	//POSITION: セマンティクス. どう解釈するか定義するもので入力データは自由につけられる
	float3 Position : POSITION;		//位置情報
	float4 Color : COLOR;			//頂点カラー
};

cbuffer Transform : register(b0)		//定数バッファ.　定数バッファ用のレジスタの0番目を使用することを意味する
{
	//行列は4x4のfloat型(=16個)で定義する
	// : backoffset(cX) 定数バッファの先頭からのオフセットを指定する
	// c0が先頭データ. c1が先頭からfloat4つ分の16byte先
	float4x4 World : packoffset(c0);		//ワールド行列
	float4x4 View : packoffset(c4);		//ビュー行列
	float4x4 Proj : packoffset(c8);	//プロジェクション行列
};

struct VSOutput
{
	//出力データに関してはシステム値セマンティクスと呼ばれるシステムで定義された特殊なセマンティクスがある
	//どのシェーダーステージでシステム値セマンティクスを使用するかは決まっており、SV_で始まる
	float4 Position : SV_POSITION;	//頂点座標. ラスタライザーに位置座標を設定する必要があるのでSV_POSITIONを使用する
	float4 Color : COLOR;			//頂点カラー. システムで定義されたモノにする必要はないのでCOLORとした
};

float4 main( float4 pos : POSITION ) : SV_POSITION
{
	return pos;
}

//頂点シェーダーのエントリーポイント
VSOutput main(VSInput input)
{
	VSOutput output = (VSOutput)0;

	float4 localPos = float4(input.Position, 1.0f);
	float4 worldPos = mul(localPos, World);		//TODO: 逆
	float4 viewPos = mul(worldPos, View);		//TODO: 逆
	float4 projPos = mul(viewPos, Proj);		//TODO: 逆

	output.Position = projPos;
	output.Color = input.Color;

	return output;
}