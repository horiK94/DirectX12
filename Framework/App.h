#pragma once

#include <Windows.h>
#include <cstdint>
#include <d3d12.h>	// Direct3D12
#include <dxgi1_4.h>	// DXGI: DirectXのグラフィックスランタイムに依存しない(複数バージョンのDirect2Dに共通のフレームワークを提供する)
// ドライバーやハードウェアの通信といった低レベルのタスクを管理. このような箇所は変化が緩やかなため、変化が頻繁なDirectXのAPIとは分離されている
#include <wrl/client.h>		//スマートポインタを使用するためにinclude
#include <DirectXMath.h>

// リンカーにライブラリをリンクするように指示. リンカーは、リンク時に指定されたライブラリを探し、必要なオブジェクトファイルをリンクする
// リンクとは、複数のオブジェクトファイルを結合して、実行ファイルを作成すること
#pragma comment(lib, "d3d12.lib")	// Direct3D12
#pragma comment(lib, "dxgi.lib")	// DXGI

template<typename T> using ComPtr = Microsoft::WRL::ComPtr<T>;	// ComPtrは、スマートポインタ. c++標準ライブラリにもunique_ptrやshared_ptrがあるが、ComPtrはMicrsoftで作ったものなので親和性が高い

template<typename T>
struct ConstantBufferView
{
	D3D12_CONSTANT_BUFFER_VIEW_DESC Desc;		//定数バッファの構成設定
	//ディスクリプタ: GPUリソース(テクスチャ、バッファなど)の参照を提供するデータ構造
	//ハンドル: ポインタのポインタ や ファイルや画像、ウィンドウなどを操作したいときに対象を識別するために個々の要素に割り当てられる一意の番号を指す
	D3D12_CPU_DESCRIPTOR_HANDLE HandleCPU;		//CPUディスクリプタハンドル
	D3D12_GPU_DESCRIPTOR_HANDLE HandleGPU;		//GPUディスクリプタハンドル
	T* pBuffer;		//バッファ先頭へのポインタ(バッファとはデータを一時的に保持しておくための記憶領域)
};

struct Vertex		//頂点データ
{
	DirectX::XMFLOAT3 Position;		//位置情報
	DirectX::XMFLOAT4 Color;			//頂点カラー
};

//DirectX12の仕様上、定数バッファはメモリアラインメントが256byteにならないといけない
struct alignas(256) Transform		//定数バッファ用データ
{
	DirectX::XMMATRIX World;	//ワールド行列
	DirectX::XMMATRIX View;		//ビュー行列
	DirectX::XMMATRIX Proj;		//プロジェクション行列(射影行列)
};

class App
{
public:
	App(uint32_t width, uint32_t height);
	~App();
	void Run();

private:
	static const uint32_t FrameCount = 2;	// バックバッファの数

	HINSTANCE m_hInst;
	HWND m_hWnd;
	uint32_t m_width;
	uint32_t m_height;

	ComPtr<ID3D12Device> m_pDevice;	// Direct3D12デバイス
	ComPtr<ID3D12CommandQueue> m_pQueue;	// コマンドキュー
	ComPtr<IDXGISwapChain3> m_pSwapChain;	// スワップチェイン
	ComPtr<ID3D12Resource> m_pColorBuffers[FrameCount];	// カラーバッファ
	ComPtr<ID3D12CommandAllocator> m_pCmdAllocator[FrameCount];	// コマンドアロケータ. コマンドリストのメモリを管理する(らしい)
	ComPtr<ID3D12GraphicsCommandList> m_pCmdList;	// コマンドリスト
	//ヒープとは、コンピュータプログラムが利用するメモリ領域の種類の一つで、実行時に任意のタイミングで確保や解放が可能なもの
	ComPtr<ID3D12DescriptorHeap> m_pHeapRTV;	// レンダーターゲットビュー用ディスクリプタヒープ
	ComPtr<ID3D12Fence> m_pFence;	// フェンス
	ComPtr<ID3D12DescriptorHeap> m_pHeapCBV;	//定数バッファビュー / シェーダーリソースビュー / アンオーダードアクセスビュー用 ディスクリプタヒープ
	ComPtr<ID3D12Resource> m_pVB;	// 頂点バッファ
	ComPtr<ID3D12Resource> m_pCB[FrameCount];	// 定数バッファ
	ComPtr<ID3D12RootSignature> m_pRootSignature;	// ルートシグネチャ. シェーダーはどのレジスタにドのデータが入っているかをディスクリプタヒープから取得する. そのディスクリプタヒープの情報をまとめたものがルートシグネチャ
	ComPtr<ID3D12PipelineState> m_pPSO;		//パイプラインステートオブジェクト. 入力ストリーム、シェーダ、ステート等をすべてひとつのオブジェクトのまとめたもの

	HANDLE m_FenceEvent;	// フェンスイベント
	uint64_t m_FenceCounter[FrameCount];	// フェンスカウンタ
	uint32_t m_FrameIndex;	// フレーム番号
	D3D12_CPU_DESCRIPTOR_HANDLE m_HandleRTV[FrameCount];	// レンダーターゲットビューのCPUディスクリプタハンドル

	D3D12_VERTEX_BUFFER_VIEW m_VBV;	// 頂点バッファビュー
	D3D12_VIEWPORT m_Viewport;	// ビューポート
	D3D12_RECT m_Scissor;	// シザー矩形
	ConstantBufferView<Transform> m_CBV[FrameCount];	// 定数バッファビュー
	float m_RotateAngle;	// 回転角

	bool InitApp();
	void TermApp();
	bool InitWnd();
	void TermWnd();
	void MainLoop();

	bool InitD3D();
	void TermD3D();
	void Render();
	void WaitGpu();
	void Present(uint32_t interval);

	bool OnInit();

	static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
};

