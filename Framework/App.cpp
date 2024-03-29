#include "App.h"
#include <cassert>

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

	if (!InitD3D())		// Direct3Dの初期化
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
	//Direct3Dの終了処理
	TermD3D();

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
		else
		{
			Render();	//描画処理
		}
	}
}

bool App::InitD3D()
{
	//Direct3D12のデバッグレイヤーを有効にする. APIのエラーや警告を表示するが、パフォーマンスは低下する
#if defined(_DEBUG) || defined(DEBUG)
	{
		ComPtr<ID3D12Debug> pDebug = nullptr;
		auto hr = D3D12GetDebugInterface(IID_PPV_ARGS(pDebug.GetAddressOf()));

		//デバッグレイヤーの有効化
		if (SUCCEEDED(hr))
		{
			pDebug->EnableDebugLayer();
		}
	}
#endif

//Direct3D12デバイスの生成
auto hr = D3D12CreateDevice(
	nullptr,					//使用するアダプタ. nullptrはデフォルト
	D3D_FEATURE_LEVEL_11_0,		//デバイスの正常な作成に必要な最小機能レベル
	IID_PPV_ARGS(m_pDevice.GetAddressOf()));	//生成されたDirect3D12デバイス
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

	hr = m_pDevice->CreateCommandQueue(&desc, IID_PPV_ARGS(m_pQueue.GetAddressOf()));
	if (FAILED(hr))
	{
		return false;
	}
}

//スワップチェインの作成
{
	//DXGIファクトリーの生成. DXGIファクトリーはビデオグラフィックの設定の列挙や指定に使用したり、スワップチェインの作成に使用する
	ComPtr<IDXGIFactory4> pFactory = nullptr;
	hr = CreateDXGIFactory1(IID_PPV_ARGS(pFactory.GetAddressOf()));
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
	desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;	//バックバッファのフォーマット
	desc.SampleDesc.Count = 1;	//マルチサンプリングの数
	desc.SampleDesc.Quality = 0;	//マルチサンプリングの品質
	desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;	//バックバッファの使用法. DXGI_USAGE_RENDER_TARGET_OUTPUTはバックバッファをレンダーターゲットとして使用する
	desc.BufferCount = FrameCount;	//スワップチェインのバッファ数
	desc.OutputWindow = m_hWnd;	//ウィンドウハンドル
	desc.Windowed = TRUE;	//ウィンドウモードかフルスクリーンモードか
	desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;	//バックバッファ入れ替え時の効果指定. DXGI_SWAP_EFFECT_FLIP_DISCARDはフリップ後にバックバッファの内容を破棄する
	desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;	//スワップチェインの動作オプション

	//スワップチェインの生成
	ComPtr<IDXGISwapChain> pSwapChain = nullptr;
	hr = pFactory->CreateSwapChain(m_pQueue.Get(), &desc, pSwapChain.GetAddressOf());
	if (FAILED(hr))
	{
		pFactory.Reset();
		return false;
	}

	//IDXGISwapChain3: レンダリングされたデータを出力に表示する前に格納するためのサーフェス(=レンダリング結果保存するバッファのこと)を提供する
	//IDXGISwapChain3ではバック バッファーのインデックスの取得と色空間のサポートが追加されている

	// IDXGISwapChain3の取得
	hr = pSwapChain->QueryInterface(IID_PPV_ARGS(m_pSwapChain.GetAddressOf()));
	if (FAILED(hr))
	{
		pFactory.Reset();
		pSwapChain.Reset();
		return false;
	}

	//バックバッファ番号の取得
	m_FrameIndex = m_pSwapChain->GetCurrentBackBufferIndex();

	//不要になったので解放
	pFactory.Reset();
	pSwapChain.Reset();
}

//コマンドアロケータの生成
//アロケータはメモリー領域の割り当てを行うオブジェクト
//コマンドアロケータはコマンドリストの実行に必要なメモリーを割り当てる
{
	for (auto i = 0; i < FrameCount; i++)
	{
		hr = m_pDevice->CreateCommandAllocator(
			D3D12_COMMAND_LIST_TYPE_DIRECT,	//コマンドリストの種類. D3D12_COMMAND_LIST_TYPE_DIRECT は GPUで実行できるコマンドバッファーを指定
			IID_PPV_ARGS(m_pCmdAllocator[i].GetAddressOf()));	//生成されたコマンドアロケータ
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
		m_pCmdAllocator[m_FrameIndex].Get(),	//コマンドアロケータ
		nullptr,	//パイプラインステートオブジェクト. パイプラインを指定できる. nullptrを指定するとデフォルトのパイプラインが使用される
		IID_PPV_ARGS(m_pCmdList.GetAddressOf()));	//生成されたコマンドリスト
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
	hr = m_pDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(m_pHeapRTV.GetAddressOf()));
	if (FAILED(hr))
	{
		return false;
	}

	//レンダーターゲットビューの作成
	auto handle = m_pHeapRTV->GetCPUDescriptorHandleForHeapStart();	//ディスクリプタヒープの先頭のハンドルを取得
	auto incrementSize = m_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);	//ディスクリプタヒープのインクリメントサイズを取得

	for (auto i = 0u; i < FrameCount; i++)
	{
		hr = m_pSwapChain->GetBuffer(i, IID_PPV_ARGS(m_pColorBuffers[i].GetAddressOf()));	//スワップチェインからバッファを取得
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
		m_pDevice->CreateRenderTargetView(m_pColorBuffers[i].Get(), &viewDesc, handle);

		m_HandleRTV[i] = handle;
		handle.ptr += incrementSize;
	}
}

//フェンスの作成
{
	//フェンスカウンタのリセット
	for (auto i = 0; i < FrameCount; i++)
	{
		m_FenceCounter[i] = 0;
	}

	hr = m_pDevice->CreateFence(
		m_FenceCounter[m_FrameIndex],	//フェンスの初期値
		D3D12_FENCE_FLAG_NONE,	//フェンスのフラグ
		IID_PPV_ARGS(m_pFence.GetAddressOf()));	//生成されたフェンス
	if (FAILED(hr))
	{
		return false;
	}

	m_FenceCounter[m_FrameIndex]++;

	//描画処理が終わるまで待機するイベントの作成
	m_FenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	if (m_FenceEvent == nullptr)
	{
		return false;
	}
}

//コマンドリストを閉じる
m_pCmdList->Close();

return true;
}

void App::TermD3D()
{
	//GPU処理の完了を待機
	WaitGpu();

	//イベント破棄
	if (m_FenceEvent != nullptr)
	{
		CloseHandle(m_FenceEvent);
		m_FenceEvent = nullptr;
	}

	//フェンス破棄
	m_pFence.Reset();

	//レンダーターゲットビュー破棄
	m_pHeapRTV.Reset();
	for (auto i = 0; i < FrameCount; i++)
	{
		m_pColorBuffers[i].Reset();
	}

	//コマンドリストの破棄
	m_pCmdList.Reset();

	//コマンドアロケーターの破棄
	for (auto i = 0; i < FrameCount; i++)
	{
		m_pCmdAllocator[i].Reset();
	}

	//スワップチェインの破棄
	m_pSwapChain.Reset();

	//コマンドキューの破棄
	m_pQueue.Reset();

	//デバイスの破棄
	m_pDevice.Reset();
}

/// <summary>
/// 描画処理
/// </summary>
void App::Render()
{
	//コマンドの記録開始
	m_pCmdAllocator[m_FrameIndex]->Reset();
	m_pCmdList->Reset(m_pCmdAllocator[m_FrameIndex].Get(), nullptr);

	//リソースバリアの設定. リソースバリアとはGPUの表示処理中に描画コマンドが発行され、表示がバグるといったような割り込みによるバグを防ぐために、使用中の割り込みを排除する仕組み
	D3D12_RESOURCE_BARRIER barrier = {};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;	//リソースバリアの種類. D3D12_RESOURCE_BARRIER_TYPE_TRANSITIONはリソースの状態を変更する
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;	//リソースバリアのフラグ
	barrier.Transition.pResource = m_pColorBuffers[m_FrameIndex].Get();	//リソースバリアの対象となるリソース
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
	barrier.Transition.pResource = m_pColorBuffers[m_FrameIndex].Get();	//リソースバリアの対象となるリソース
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;	//リソースバリアの変更前のリソースの状態. D3D12_RESOURCE_STATE_RENDER_TARGETはリソースがレンダーターゲットとして使用されている状態
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;	//リソースバリアの変更後のリソースの状態. D3D12_RESOURCE_STATE_PRESENTはリソースがスワップチェインに表示されている状態
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;	//リソースバリアの対象となるサブリソース

	//リソースバリアの設定
	m_pCmdList->ResourceBarrier(1, &barrier);

	//コマンドの記録終了
	m_pCmdList->Close();

	//コマンドの実行
	ComPtr<ID3D12CommandList> ppCmdLists[] = { m_pCmdList.Get() };		//コマンドリストの配列(コマンドキューではない)
	m_pQueue->ExecuteCommandLists(1, ppCmdLists->GetAddressOf());	//コマンドリストの実行

	//画面に表示
	Present(1);
}

void App::WaitGpu()
{
	assert(m_pQueue != nullptr);
	assert(m_pFence != nullptr);
	assert(m_FenceEvent != nullptr);

	//GPUの実行中に解放してしまうとアプリケーションのクラッシュや、グラフィックスドライバーが落ちる場合があるので、実行完了まで待機する
	//シグナル処理
	m_pQueue->Signal(
		m_pFence.Get(),		//フェンスのポインタ 
		m_FenceCounter[m_FrameIndex]); //フェンスに設定する値

	//フェンスの値が更新されるまで待機するイベントを設定
	m_pFence->SetEventOnCompletion(
		m_FenceCounter[m_FrameIndex],	//フェンスの値
		m_FenceEvent);	//イベントのハンドル

	WaitForSingleObjectEx(		//更新の待機
		m_FenceEvent,	//イベントのハンドル
		INFINITE,		//タイムアウト時間
		FALSE);			//完了待機終了条件の設定

	//次のフレームのフェンスカウンタを増やす
	m_FenceCounter[m_FrameIndex]++;
}

void App::Present(uint32_t interval)
{
	//表示処理も明示的に行う必要がある

	//画面に表示
	m_pSwapChain->Present(
		interval,		//垂直同期とフレーム表示の同期方法. 0は垂直同期なし. 1は垂直同期後に表示
		0);		//表示オプション

	//シグナル処理
	//コマンドキューの実行が完了(=GPU上のコマンド処理終了)したときにフェンスの値を更新する
	const auto currentValue = m_FenceCounter[m_FrameIndex];
	m_pQueue->Signal(
		m_pFence.Get(),		//フェンスのポインタ 
		currentValue); //フェンスに設定する値

	//スワップ処理(m_pSwapChain-> Present())を呼んだので、バックバッファのインデックスを更新
	m_FrameIndex = m_pSwapChain->GetCurrentBackBufferIndex();
	if (m_pFence->GetCompletedValue() < m_FenceCounter[m_FrameIndex])
	{
		//フェンスの値が更新されるまで待機するイベントを設定
		m_pFence->SetEventOnCompletion(
			m_FenceCounter[m_FrameIndex],	//フェンスの値
			m_FenceEvent);	//イベントのハンドル

		WaitForSingleObjectEx(		//更新の待機
			m_FenceEvent,	//イベントのハンドル
			INFINITE,		//タイムアウト時間
			FALSE);			//完了待機終了条件の設定
	}

	//次のフレームのフェンスカウンタを増やす
	m_FenceCounter[m_FrameIndex] = currentValue + 1;
}

bool App::OnInit()
{
	{
		//頂点バッファの生成
		//頂点データ
		Vertex vertices[] = {
		{DirectX::XMFLOAT3(-1.0f, -1.0f, 0.0f), DirectX::XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f)},
		{DirectX::XMFLOAT3(1.0f, -1.0f, 0.0f), DirectX::XMFLOAT4(0.0f, 1.0f,0.0f, 1.0f)},
		{DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f), DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f)}
		};

		//GPUに頂点を送る時、バッファを作成して纏めて送る. この時作成するバッファが頂点バッファ
		//頂点バッファを作るには、ID3D12Resourcesオブジェクトとして生成し、GPUの仮想アドレス、頂点全体のデータサイズ、1頂点当たりのサイズをD3D12_VERTEX_BUFFER_VIEW構造体に設定する
		//ID3D12ResourcesオブジェクトはCreateCommittedResource()で作成

		//ヒーププロパティ
		D3D12_HEAP_PROPERTIES prop = {};
		prop.Type = D3D12_HEAP_TYPE_UPLOAD;	//ヒープの種類. D3D12_HEAP_TYPE_UPLOADはCPUに書き込み1度、GPU読み込みが1度のデータが適している
		prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;	//CPUページプロパティ. メモリへの書き込みタイミングを指定するものっぽい
		prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;	//メモリプールの指定. プールの方法を指定するもの
		prop.CreationNodeMask = 1;	//生成されるリソースの場所のノードを指定. マルチGPUの際に使用する. 単一GPUの場合は1を指定
		prop.VisibleNodeMask = 1;	//リソースが可視できる場所のノードを指定. マルチGPUの際に使用する. 単一GPUの場合は1を指定

		//リソースの指定
		D3D12_RESOURCE_DESC desc = {};
		desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;	//リソースの種類. D3D12_RESOURCE_DIMENSION_BUFFERはバッファを指定
		desc.Alignment = 0;	//リソースのアライメント. D3D12_RESOURCE_DIMENSION_BUFFER 指定時が D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT(=64KB) か0を指定する
		desc.Width = sizeof(vertices);	//リソースの幅. バッファのサイズを指定. テクスチャの場合は横幅を指定する
		desc.Height = 1;	//リソースの高さ. バッファの場合は1を指定. テクスチャの場合は縦幅を指定する
		desc.DepthOrArraySize = 1;	//リソースの深さ. バッファの場合は1を指定. 3次元テクスチャの場合は奥行、テクスチャ配列の場合は配列数を指定する
		desc.MipLevels = 1;	//ミップマップレベル. バッファの場合は1を指定. テクスチャの場合はミップマップレベルを指定する
		desc.Format = DXGI_FORMAT_UNKNOWN;	//フォーマット. バッファの場合はDXGI_FORMAT_UNKNOWNを指定. テクスチャの場合はピクセルフォーマットを指定
		desc.SampleDesc.Count = 1;	//マルチサンプリングの数. バッファの場合は1を指定. テクスチャの場合はマルチサンプリングの数を指定する
		desc.SampleDesc.Quality = 0;	//マルチサンプリングの品質. バッファの場合は0を指定. テクスチャの場合はマルチサンプリングの品質を指定する
		desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;	//テクスチャのレイアウト. バッファの場合はD3D12_TEXTURE_LAYOUT_ROW_MAJORを指定
		desc.Flags = D3D12_RESOURCE_FLAG_NONE;	//リソースのオプションフラグ. 必要な場合はビット論理和で指定する

		//リソースの生成
		auto hr = m_pDevice->CreateCommittedResource(
			&prop,
			D3D12_HEAP_FLAG_NONE,		//ヒープのオプションフラグ. D3D12_HEAP_FLAG_NONEは特に指定なし
			&desc,
			D3D12_RESOURCE_STATE_GENERIC_READ,	//リソースの初期状態.　ヒープの種類でD3D12_HEAP_TYPE_UPLOADを指定したので、D3D12_RESOURCE_STATE_GENERIC_READを指定する必要がある
			nullptr,		//レンダーターゲットと深度ステンシルテクスチャのためのオプション. リソースの種類でD3D12_RESOURCE_DIMENSION_BUFFERを指定したので、nullptrを指定する必要がある
			IID_PPV_ARGS(m_pVB.GetAddressOf()));
		if (FAILED(hr))
		{
			return false;
		}

		//頂点データの内容をリソースに書き込む(=マッピング)
		void* ptr = nullptr;
		hr = m_pVB->Map(0,		//サブリソース(=ミップマップのデータのまとまり)のインデックス.頂点バッファは1つしかサブリソースがないので0を指定 
			nullptr,		//マッピング開始オフセット.終了オフセットの指定. 全領域はnullptrを指定
			&ptr);			//リソースデータへのポインタを受け取るメモリブロックへのポインタ(=リソースデータのポインタのポインタ)
		if (FAILED(hr))
		{
			return false;
		}

		//memcpy(dest, src, count): src が指すオブジェクトから dest が指すオブジェクトに、count バイトをコピー
		memcpy(ptr, vertices, sizeof(vertices));

		//マッピング解除
		m_pVB->Unmap(
			0,		//サブリソース(=ミップマップのデータのまとまり)のインデックス.頂点バッファは1つしかサブリソースがないので0を指定 
			nullptr		//マッピングした開始オフセット.終了オフセットの指定. 全領域はnullptrを指定
		);

		//頂点バッファビューの作成. 描画コマンド作成時に使用
		m_VBV.BufferLocation = m_pVB->GetGPUVirtualAddress();	//頂点バッファのGPU仮想アドレス
		m_VBV.SizeInBytes = static_cast<UINT>(sizeof(vertices));	//頂点バッファの全体のサイズ
		m_VBV.StrideInBytes = static_cast<UINT>(sizeof(Vertex));	//1頂点当たりのサイズ
	}

	//定数バッファ: シェーダー内で計算に使う定数値. vertex shaderのWorld, View, Projが該当する
	//流れは頂点バッファと同様で 定義を用意 → リソース生成 → 定数バッファビューの作成
	//定数バッファではCPUでデータを変える可能性が多く、GPUの処理中に変えてしまう可能性があるので、レンダーターゲットと同じようにダブルバッファにしておく
	{
		//定数バッファ用ディスクリプタヒープの作成

		D3D12_DESCRIPTOR_HEAP_DESC desc = {};
		desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;	//ディスクリプタヒープの種類. D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAVは定数バッファ、シェーダリソース、アンオーダードアクセスビューを指定
		desc.NumDescriptors = 1 * FrameCount;	//ディスクリプタの数
		desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;	//ディスクリプタヒープのフラグ. D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLEはシェーダから見えるディスクリプタヒープを指定
		desc.NodeMask = 0;	//マルチGPUの際に使用するマスク

		auto hr = m_pDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(m_pHeapCBV.GetAddressOf()));
		if (FAILED(hr))
		{
			return false;
		}
	}

	//定数バッファの作成
	{
		//ヒーププロパティ
		D3D12_HEAP_PROPERTIES prop = {};
		prop.Type = D3D12_HEAP_TYPE_UPLOAD;	//ヒープの種類. D3D12_HEAP_TYPE_UPLOADはCPUに書き込み1度、GPU読み込みが1度のデータが適している
		prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;	//CPUページプロパティ. メモリへの書き込みタイミングを指定するものっぽい
		prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;	//メモリプールの指定. プールの方法を指定するもの
		prop.CreationNodeMask = 1;	//生成されるリソースの場所のノードを指定. マルチGPUの際に使用する. 単一GPUの場合は1を指定
		prop.VisibleNodeMask = 1;	//リソースが可視できる場所のノードを指定. マルチGPUの際に使用する. 単一GPUの場合は1を指定

		//リソースの指定
		D3D12_RESOURCE_DESC desc = {};
		desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;	//リソースの種類. D3D12_RESOURCE_DIMENSION_BUFFERはバッファを指定
		desc.Alignment = 0;	//リソースのアライメント. D3D12_RESOURCE_DIMENSION_BUFFER 指定時が D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT(=64KB) か0を指定する
		desc.Width = sizeof(Transform);	//リソースの幅. バッファのサイズを指定. テクスチャの場合は横幅を指定する
		desc.Height = 1;	//リソースの高さ. バッファの場合は1を指定. テクスチャの場合は縦幅を指定する
		desc.DepthOrArraySize = 1;	//リソースの深さ. バッファの場合は1を指定. 3次元テクスチャの場合は奥行、テクスチャ配列の場合は配列数を指定する
		desc.MipLevels = 1;	//ミップマップレベル. バッファの場合は1を指定. テクスチャの場合はミップマップレベルを指定する
		desc.Format = DXGI_FORMAT_UNKNOWN;	//フォーマット. バッファの場合はDXGI_FORMAT_UNKNOWNを指定. テクスチャの場合はピクセルフォーマットを指定
		desc.SampleDesc.Count = 1;	//マルチサンプリングの数. バッファの場合は1を指定. テクスチャの場合はマルチサンプリングの数を指定する
		desc.SampleDesc.Quality = 0;	//マルチサンプリングの品質. バッファの場合は0を指定. テクスチャの場合はマルチサンプリングの品質を指定する
		desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;	//テクスチャのレイアウト. バッファの場合はD3D12_TEXTURE_LAYOUT_ROW_MAJORを指定
		desc.Flags = D3D12_RESOURCE_FLAG_NONE;	//リソースのオプションフラグ. 必要な場合はビット論理和で指定する

		auto incrementSize = m_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);	//ディスクリプタヒープのインクリメントサイズを取得

		for (auto i = 0; i < FrameCount; i++)
		{
			//各フレームでリソースの生成
			auto hr = m_pDevice->CreateCommittedResource(
				&prop,
				D3D12_HEAP_FLAG_NONE,		//ヒープのオプションフラグ. D3D12_HEAP_FLAG_NONEは特に指定なし
				&desc,
				D3D12_RESOURCE_STATE_GENERIC_READ,	//リソースの初期状態.　ヒープの種類でD3D12_HEAP_TYPE_UPLOADを指定したので、D3D12_RESOURCE_STATE_GENERIC_READを指定する必要がある
				nullptr,		//レンダーターゲットと深度ステンシルテクスチャのためのオプション. リソースの種類でD3D12_RESOURCE_DIMENSION_BUFFERを指定したので、nullptrを指定する必要がある
				IID_PPV_ARGS(m_pCB[i].GetAddressOf()));		//生成したリソースをm_pCBに入れていく
			if (FAILED(hr))
			{
				return false;
			}

			//頂点バッファを作るのに、ID3D12Resourcesオブジェクトとして生成し、GPUの仮想アドレス、頂点全体のデータサイズ、1頂点当たりのサイズをD3D12_VERTEX_BUFFER_VIEW構造体に設定した
			//定数バッファーも同じでGPUに送るために定数バッファビューを構造体D3D12_CONSTANT_BUFFER_VIEWに設定する必要がある
			//必要な情報はGPU仮想アドレス
			//typedef struct D3D12_CONSTANT_BUFFER_VIEW_DESC {
			//	D3D12_GPU_VIRTUAL_ADDRESS BufferLocation;		// GPU仮想アドレス
			//	UINT                      SizeInBytes;		// 定数バッファのサイズ
			//} D3D12_CONSTANT_BUFFER_VIEW_DESC;
			//CreateConstantBufferView(D3D12_CONSTANT_BUFFER_VIEW_DESC, D3D12_CPU_DESCRIPTOR_HANDLE CPUディスクリプタハンドル)
			//ディスクリプタ: GPUリソース(テクスチャ、バッファなど)の参照を提供するデータ構造
			//ハンドル: ポインタのポインタ や ファイルや画像、ウィンドウなどを操作したいときに対象を識別するために個々の要素に割り当てられる一意の番号を指す

			auto address = m_pCB[i]->GetGPUVirtualAddress();	//定数バッファのGPU仮想アドレス
			auto handleCPU = m_pHeapCBV->GetCPUDescriptorHandleForHeapStart();	//CPUディスクリプタヒープの先頭のハンドルを取得
			auto handleGPU = m_pHeapCBV->GetGPUDescriptorHandleForHeapStart();	//GPUディスクリプタヒープの先頭のハンドルを取得
		}
	}
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
