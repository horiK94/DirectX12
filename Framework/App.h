#pragma once

#include <Windows.h>
#include <cstdint>
#include <d3d12.h>	// Direct3D12
#include <dxgi1_4.h>	// DXGI: DirectXのグラフィックスランタイムに依存しない(複数バージョンのDirect2Dに共通のフレームワークを提供する)
// ドライバーやハードウェアの通信といった低レベルのタスクを管理. このような箇所は変化が緩やかなため、変化が頻繁なDirectXのAPIとは分離されている

// リンカーにライブラリをリンクするように指示. リンカーは、リンク時に指定されたライブラリを探し、必要なオブジェクトファイルをリンクする
// リンクとは、複数のオブジェクトファイルを結合して、実行ファイルを作成すること
#pragma comment(lib, "d3d12.lib")	// Direct3D12
#pragma comment(lib, "dxgi.lib")	// DXGI

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

	ID3D12Device* m_pDevice;	// Direct3D12デバイス
	ID3D12CommandQueue* m_pQueue;	// コマンドキュー
	IDXGISwapChain3* m_pSwapChain;	// スワップチェイン
	ID3D12Resource* m_pColorBuffers[FrameCount];	// カラーバッファ
	ID3D12CommandAllocator* m_pCmdAllocator[FrameCount];	// コマンドアロケータ. コマンドリストのメモリを管理する(らしい)
	ID3D12GraphicsCommandList* m_pCmdList;	// コマンドリスト
	ID3D12DescriptorHeap* m_pHeapRTV;	// レンダーターゲットビュー用ディスクリプタヒープ
	ID3D12Fence* m_pFence;	// フェンス
	HANDLE m_FenceEvent;	// フェンスイベント
	uint64_t m_FenceCounter[FrameCount];	// フェンスカウンタ
	uint32_t m_FrameIndex;	// フレーム番号
	D3D12_CPU_DESCRIPTOR_HANDLE m_HandleRTV[FrameCount];	// レンダーターゲットビューのCPUディスクリプタハンドル

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

	static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
};

