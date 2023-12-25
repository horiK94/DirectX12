#include "App.h"

namespace //�������O���. ���̕ϐ��͂��ׂē��������P�[�W�ɂȂ�
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
/// ���s
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
	if (!InitWnd())		// �E�B���h�E�̏�����
	{
		return false;
	}

	//����I��
	return true;
}

bool App::InitWnd()
{
	//�C���X�^���X�n���h���̎擾. �n���h���͎��ʎq��\��
	//�����N�������ہA��������ɂ͓����v���O�������������݂���̂ŁA�������ʂ��邽�߂ɗp����
	auto hInst = GetModuleHandle(nullptr);

	//�E�B���h�E�̐ݒ�
	WNDCLASSEX wc = {};
	wc.cbSize = sizeof(WNDCLASSEX);		//�\���̂̃T�C�Y
	wc.style = CS_HREDRAW | CS_VREDRAW;	//�E�B���h�E�X�^�C��
	wc.lpfnWndProc = WndProc;				//�E�B���h�E�v���V�[�W��
	wc.hIcon = LoadIcon(hInst, IDI_APPLICATION);	//�A�C�R��
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);	//�J�[�\��
	wc.hbrBackground = GetSysColorBrush(COLOR_BACKGROUND);	//�w�i�F
	wc.lpszMenuName = nullptr;				//���j���[
	wc.lpszClassName = ClassName;			//�N���X��
	wc.hIconSm = LoadIcon(hInst, IDI_APPLICATION);	//�A�C�R��(��)

	//�E�B���h�E�̓o�^
	if (!RegisterClassEx(&wc))
	{
		return false;
	}

	//�C���X�^���X�n���h���̐ݒ�
	m_hInst = hInst;

	//�E�B���h�E�̃T�C�Y�ݒ�
	RECT rc = {};
	rc.right = static_cast<LONG>(m_width);
	rc.bottom = static_cast<LONG>(m_height);

	//�E�B���h�E�̃T�C�Y�𒲐�
	auto style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU;
	AdjustWindowRect(&rc, style, FALSE);

	//�E�B���h�E�̍쐬
	m_hWnd = CreateWindowEx(
		0,						//�g���E�B���h�E�X�^�C��
		ClassName,				//�N���X��
		TEXT("Sample"),			//�E�B���h�E��
		style,					//�E�B���h�E�X�^�C��
		CW_USEDEFAULT,			//�\��X���W
		CW_USEDEFAULT,			//�\��Y���W
		rc.right - rc.left,		//�E�B���h�E��
		rc.bottom - rc.top,		//�E�B���h�E����
		nullptr,				//�e�E�B���h�E�n���h��
		nullptr,				//���j���[�n���h��
		hInst,					//�C���X�^���X�n���h��
		nullptr);				//���̑��̍쐬�f�[�^

	if (m_hWnd == nullptr)
	{
		return false;
	}

	//�E�B���h�E�̕\��
	ShowWindow(m_hWnd, SW_SHOWNORMAL);

	//�E�B���h�E�̍X�V
	UpdateWindow(m_hWnd);

	//�E�B���h�E�Ƀt�H�[�J�X��ݒ�
	SetFocus(m_hWnd);

	//����I��
	return true;
}

void App::TermApp()
{
	//�E�B���h�E�̏I������
	TermWnd();
}

void App::TermWnd()
{
	if (m_hInst != nullptr)
	{
		//�E�B���h�E�̓o�^������
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
		//���b�Z�[�W�̎擾
		if(PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE) == TRUE)
		{
			TranslateMessage(&msg);		//�L�[���̓��b�Z�[�W�̏���
			DispatchMessage(&msg);		//�E�B���h�E�v���V�[�W���Ƀ��b�Z�[�W�𑗂�
		}
	}
}

LRESULT App::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_DESTROY:	//�E�B���h�E�j��
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
