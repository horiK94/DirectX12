#include "App.h"

namespace //無名名前空間. 中の変数はすべて内部リンケージになる
{
	const auto ClassName = TEXT("SampleWindowClass");

	template<typename T>
	void SafeRelease(T*& p)
	{
		if (p != nullptr)
		{
			p->Release();
			p = nullptr;
		}
	}
}

App::App(uint32_t width, uint32_t height)
	: m_hInst(nullptr)
	, m_hWnd(nullptr)
	, m_width(width)
	, m_height(height)
{}

App::~App()
{}

/// <summary>
/// 実行
/// </summary>
void App::Run()
{
	if (InitApp())
	{
		MainLoop();
	}

	TermApp();
}

bool App::InitApp()
{
	if (!InitWnd())		// ウィンドウの初期化
	{
		return false;
	}

	//正常終了
	return true;
}

bool App::InitWnd()
{
	//インスタンスハンドルの取得. ハンドルは識別子を表す
	//複数起動した際、メモリ上には同じプログラムが複数存在するので、それを区別するために用いる
	auto hInst = GetModuleHandle(nullptr);

	//ウィンドウの設定
	WNDCLASSEX wc = {};
	wc.cbSize = sizeof(WNDCLASSEX);		//構造体のサイズ
	wc.style = CS_HREDRAW | CS_VREDRAW;	//ウィンドウスタイル
	wc.lpfnWndProc = WndProc;				//ウィンドウプロシージャ
	wc.hIcon = LoadIcon(hInst, IDI_APPLICATION);	//アイコン
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);	//カーソル
	wc.hbrBackground = GetSysColorBrush(COLOR_BACKGROUND);	//背景色
	wc.lpszMenuName = nullptr;				//メニュー
	wc.lpszClassName = ClassName;			//クラス名
	wc.hIconSm = LoadIcon(hInst, IDI_APPLICATION);	//アイコン(小)

	//ウィンドウの登録
	if (!RegisterClassEx(&wc))
	{
		return false;
	}

	//インスタンスハンドルの設定
	m_hInst = hInst;

	//ウィンドウのサイズ設定
	RECT rc = {};
	rc.right = static_cast<LONG>(m_width);
	rc.bottom = static_cast<LONG>(m_height);

	//ウィンドウのサイズを調整
	auto style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU;
	AdjustWindowRect(&rc, style, FALSE);

	//ウィンドウの作成
	m_hWnd = CreateWindowEx(
		0,						//拡張ウィンドウスタイル
		ClassName,				//クラス名
		TEXT("Sample"),			//ウィンドウ名
		style,					//ウィンドウスタイル
		CW_USEDEFAULT,			//表示X座標
		CW_USEDEFAULT,			//表示Y座標
		rc.right - rc.left,		//ウィンドウ幅
		rc.bottom - rc.top,		//ウィンドウ高さ
		nullptr,				//親ウィンドウハンドル
		nullptr,				//メニューハンドル
		hInst,					//インスタンスハンドル
		nullptr);				//その他の作成データ

	if (m_hWnd == nullptr)
	{
		return false;
	}

	//ウィンドウの表示
	ShowWindow(m_hWnd, SW_SHOWNORMAL);

	//ウィンドウの更新
	UpdateWindow(m_hWnd);

	//ウィンドウにフォーカスを設定
	SetFocus(m_hWnd);

	//正常終了
	return true;
}

void App::TermApp()
{
	//ウィンドウの終了処理
	TermWnd();
}

void App::TermWnd()
{
	if (m_hInst != nullptr)
	{
		//ウィンドウの登録を解除
		UnregisterClass(ClassName, m_hInst);
	}

	m_hInst = nullptr;
	m_hWnd = nullptr;
}

void App::MainLoop()
{
	MSG msg = {};

	while (msg.message != WM_QUIT)
	{
		//メッセージの取得
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE) == TRUE)
		{
			TranslateMessage(&msg);		//キー入力メッセージの処理
			DispatchMessage(&msg);		//ウィンドウプロシージャにメッセージを送る
		}
	}
}

bool App::InitD3D()
{
	//Direct3D12デバイスの生成
	auto hr = D3D12CreateDevice(
		nullptr,					//使用するアダプタ. nullptrはデフォルト
		D3D_FEATURE_LEVEL_11_0,		//デバイスの正常な作成に必要な最小機能レベル
		IID_PPV_ARGS(&m_pDevice));	//生成されたDirect3D12デバイス
	if (FAILED(hr))
	{
		return false;
	}

	//コマンドキューの生成
	{
		D3D12_COMMAND_QUEUE_DESC desc = {};
		desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;	//コマンドリストの種類
		desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;	//コマンドキューの優先度
		desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;	//コマンドキューのフラグ
		desc.NodeMask = 0;	//マルチGPUの際に使用するマスク

		hr = m_pDevice->CreateCommandQueue(&desc, IID_PPV_ARGS(&m_pQueue));
		if (FAILED(hr))
		{
			return false;
		}
	}

	//スワップチェインの作成
	{
		//DXGIファクトリーの生成. DXGIファクトリーはビデオグラフィックの設定の列挙や指定に使用したり、スワップチェインの作成に使用する
		IDXGIFactory4* pFactory = nullptr;
		hr = CreateDXGIFactory1(IID_PPV_ARGS(&pFactory));
		if (FAILED(hr))
		{
			return false;
		}

		//スワップチェインの設定
		DXGI_SWAP_CHAIN_DESC desc = {};
		desc.BufferDesc.Width = m_width;	//バックバッファの幅
		desc.BufferDesc.Height = m_height;	//バックバッファの高さ
		desc.BufferDesc.RefreshRate.Numerator = 60;	//リフレッシュレートの分母
		desc.BufferDesc.RefreshRate.Denominator = 1;	//リフレッシュレートの分子
		desc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;	//スキャンラインの描画順序
		desc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;	//ウィンドウ描画時のスケーリングの指定. バックバッファのサイズ通りに描画したり、ウィンドウサイズに合わせるなど指定可能
		desc.SampleDesc.Count = 1;	//マルチサンプリングの数
		desc.SampleDesc.Quality = 0;	//マルチサンプリングの品質
		desc.BufferUsage = DXGI_USAGE_BACK_BUFFER;	//バックバッファの使用法. DXGI_USAGE_BACK_BUFFERはバックバッファとして使用することを示す
		desc.BufferCount = FrameCount;	//スワップチェインのバッファ数
		desc.OutputWindow = m_hWnd;	//ウィンドウハンドル
		desc.Windowed = TRUE;	//ウィンドウモードかフルスクリーンモードか
		desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;	//バックバッファ入れ替え時の効果指定. DXGI_SWAP_EFFECT_FLIP_DISCARDはフリップ後にバックバッファの内容を破棄する
		desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;	//スワップチェインの動作オプション

		//スワップチェインの生成
		IDXGISwapChain* pSwapChain = nullptr;
		hr = pFactory->CreateSwapChain(m_pQueue, &desc, &pSwapChain);
		if (FAILED(hr))
		{
			SafeRelease(pFactory);
			return false;
		}

		//IDXGISwapChain3: レンダリングされたデータを出力に表示する前に格納するためのサーフェス(=レンダリング結果保存するバッファのこと)を提供する
		//IDXGISwapChain3ではバック バッファーのインデックスの取得と色空間のサポートが追加されている

		// IDXGISwapChain3の取得
		hr = pSwapChain->QueryInterface(IID_PPV_ARGS(&m_pSwapChain));
		if (FAILED(hr))
		{
			SafeRelease(pFactory);
			SafeRelease(pSwapChain);
			return false;
		}

		//バックバッファ番号の取得
		m_FrameIndex = m_pSwapChain->GetCurrentBackBufferIndex();

		//不要になったので解放
		SafeRelease(pFactory);
		SafeRelease(pSwapChain);
	}

	//コマンドアロケータの生成
	//アロケータはメモリー領域の割り当てを行うオブジェクト
	//コマンドアロケータはコマンドリストの実行に必要なメモリーを割り当てる
	{
		for (auto i = 0; i < FrameCount; i++)
		{
			hr = m_pDevice->CreateCommandAllocator(
				D3D12_COMMAND_LIST_TYPE_DIRECT,	//コマンドリストの種類. D3D12_COMMAND_LIST_TYPE_DIRECT は GPUで実行できるコマンドバッファーを指定
				IID_PPV_ARGS(&m_pCmdAllocator[i]));	//生成されたコマンドアロケータ
			if (FAILED(hr))
			{
				return false;
			}
		}
	}

	//コマンドリストの作成
	{
		hr = m_pDevice->CreateCommandList(
			0,	//コマンドリストのノードマスク. マルチGPUの際に使用するマスク
			D3D12_COMMAND_LIST_TYPE_DIRECT,	//コマンドリストの種類. CreateCommandAllocator()と同じ
			m_pCmdAllocator[m_FrameIndex],	//コマンドアロケータ
			nullptr,	//パイプラインステートオブジェクト. パイプラインを指定できる. nullptrを指定するとデフォルトのパイプラインが使用される
			IID_PPV_ARGS(&m_pCmdList));	//生成されたコマンドリスト
		if (FAILED(hr))
		{
			return false;
		}
	}

	//Direct3Dでは描画先がレンダーターゲットで、そのレンダーターゲットの実態はバックバッファやテクスチャといったリソース.
	//リソースはメモリ領域を持つオブジェクトはわかっているが、GPU上のどのアドレスのものかはわからない.
	//また、バッファ容量はわかっても、どのような種類のバッファか(2次元テクスチャ？キューブマップ？)はわからず、パイプライン上でどのように使えばよいかがわからない
	//そこでリソースをどのように扱うかを指定するのがリソースビューオブジェクト
	//レンダーターゲットのリソースオブジェクトは、レンダーターゲットビューと呼ばれる

	//レンダーターゲットビューの生成
	{
		//レンダーターゲットビューには リソースの生成とディスクリプタヒープの作成が必要
		//リソースはスワップチェインがもつバッファを使用するため、新規での作成は不要

		//ディスクリプタヒープの設定. ディスクリプタヒープはディスクリプタを格納するためのメモリ領域(配列のようなもの)
		D3D12_DESCRIPTOR_HEAP_DESC desc = {};
		desc.NumDescriptors = FrameCount;	//ディスクリプタの数
		desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;	//ディスクリプタの種類. D3D12_DESCRIPTOR_HEAP_TYPE_RTVはレンダーターゲットビューを指定
		desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;	//ディスクリプタヒープのフラグ
		desc.NodeMask = 0;	//マルチGPUの際に使用するマスク

		//ディスクリプタヒープの生成
		hr = m_pDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_pHeapRTV));
		if (FAILED(hr))
		{
			return false;
		}

		//レンダーターゲットビューの作成
		auto handle = m_pHeapRTV->GetCPUDescriptorHandleForHeapStart();	//ディスクリプタヒープの先頭のハンドルを取得
		auto incrementSize = m_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);	//ディスクリプタヒープのインクリメントサイズを取得

		for (auto i = 0u; i < FrameCount; i++)
		{
			hr = m_pSwapChain->GetBuffer(i, IID_PPV_ARGS(&m_pColorBuffers[i]));	//スワップチェインからバッファを取得
			if (FAILED(hr))
			{
				return false;
			}

			//レンダーターゲットビューの設定
			D3D12_RENDER_TARGET_VIEW_DESC viewDesc = {};
			viewDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;	//フォーマット
			viewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;	//ビューの種類. D3D12_RTV_DIMENSION_TEXTURE2Dは2次元テクスチャを指定
			viewDesc.Texture2D.MipSlice = 0;	//ミップマップレベルの指定
			viewDesc.Texture2D.PlaneSlice = 0;	//テクスチャの面の指定(平面を複数枚持つデータではないので0を指定)

			//レンダーターゲットビューの生成
			m_pDevice->CreateRenderTargetView(m_pColorBuffers[i], &viewDesc, handle);

			m_HandleRTV[i] = handle;
			handle.ptr += incrementSize;
		}
	}

	//フェンスの作成
	hr = m_pDevice->CreateFence(
		m_FenceCounter[m_FrameIndex],	//フェンスの初期値
		D3D12_FENCE_FLAG_NONE,	//フェンスのフラグ
		IID_PPV_ARGS(&m_pFence));	//生成されたフェンス
	if (FAILED(hr))
	{
		return false;
	}

	//描画処理が終わるまで待機するイベントの作成
	m_FenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	if (m_FenceEvent == nullptr)
	{
		return false;
	}
}

/// <summary>
/// 描画処理
/// </summary>
void App::Render()
{
	//コマンドの記録開始
	m_pCmdAllocator[m_FrameIndex]->Reset();
	m_pCmdList->Reset(m_pCmdAllocator[m_FrameIndex], nullptr);

	//リソースバリアの設定. リソースバリアとはGPUの表示処理中に描画コマンドが発行され、表示がバグるといったような割り込みによるバグを防ぐために、使用中の割り込みを排除する仕組み
	D3D12_RESOURCE_BARRIER barrier = {};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;	//リソースバリアの種類. D3D12_RESOURCE_BARRIER_TYPE_TRANSITIONはリソースの状態を変更する
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;	//リソースバリアのフラグ
	barrier.Transition.pResource = m_pColorBuffers[m_FrameIndex];	//リソースバリアの対象となるリソース
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;	//リソースバリアの変更前のリソースの状態. D3D12_RESOURCE_STATE_PRESENT はリソースがスワップチェインに表示されている状態
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;	//リソースバリアの変更後のリソースの状態. D3D12_RESOURCE_STATE_RENDER_TARGETはリソースがレンダーターゲットとして使用されている状態
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;	//リソースバリアの対象となるサブリソース

	//リソースバリアの設定
	m_pCmdList->ResourceBarrier(1, &barrier);

	//レンダーターゲットの設定
	m_pCmdList->OMSetRenderTargets(1, &m_HandleRTV[m_FrameIndex], FALSE, nullptr);

	//クリアカラーの設定
	float clearColor[] = { 0.25f, 0.25f, 0.25f, 1 };

	//レンダーターゲットビューをクリア
	m_pCmdList->ClearRenderTargetView(m_HandleRTV[m_FrameIndex], clearColor, 0, nullptr);

	//描画処理
	{
		//TODO: 
	}

	//リソースバリアの設定
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;	//リソースバリアの種類. D3D12_RESOURCE_BARRIER_TYPE_TRANSITIONはリソースの状態を変更する
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;	//リソースバリアのフラグ
	barrier.Transition.pResource = m_pColorBuffers[m_FrameIndex];	//リソースバリアの対象となるリソース
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;	//リソースバリアの変更前のリソースの状態. D3D12_RESOURCE_STATE_RENDER_TARGETはリソースがレンダーターゲットとして使用されている状態
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;	//リソースバリアの変更後のリソースの状態. D3D12_RESOURCE_STATE_PRESENTはリソースがスワップチェインに表示されている状態
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;	//リソースバリアの対象となるサブリソース

	//リソースバリアの設定
	m_pCmdList->ResourceBarrier(1, &barrier);

	//コマンドの記録終了
	m_pCmdList->Close();

	//コマンドの実行
	ID3D12CommandList* ppCmdLists[] = { m_pCmdList };		//コマンドリストの配列(コマンドキューではない)
	m_pQueue->ExecuteCommandLists(1, ppCmdLists);	//コマンドリストの実行

	//画面に表示
	Present(1);
}

LRESULT App::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_DESTROY:	//ウィンドウ破棄
	{
		PostQuitMessage(0);
		break;
	}
	default:
	{
	}
	break;
	}

	return DefWindowProc(hWnd, msg, wParam, lParam);
}
