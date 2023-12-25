#include "App.h"

namespace //無名名前空間. 中の変数はすべて内部リンケージになる
{
	const auto ClassName = TEXT("SampleWindowClass");
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
		if(PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE) == TRUE)
		{
			TranslateMessage(&msg);		//キー入力メッセージの処理
			DispatchMessage(&msg);		//ウィンドウプロシージャにメッセージを送る
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
