#include "App.h"

namespace //�������O���. ���̕ϐ��͂��ׂē��������P�[�W�ɂȂ�
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
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE) == TRUE)
		{
			TranslateMessage(&msg);		//�L�[���̓��b�Z�[�W�̏���
			DispatchMessage(&msg);		//�E�B���h�E�v���V�[�W���Ƀ��b�Z�[�W�𑗂�
		}
	}
}

bool App::InitD3D()
{
	//Direct3D12�f�o�C�X�̐���
	auto hr = D3D12CreateDevice(
		nullptr,					//�g�p����A�_�v�^. nullptr�̓f�t�H���g
		D3D_FEATURE_LEVEL_11_0,		//�f�o�C�X�̐���ȍ쐬�ɕK�v�ȍŏ��@�\���x��
		IID_PPV_ARGS(&m_pDevice));	//�������ꂽDirect3D12�f�o�C�X
	if (FAILED(hr))
	{
		return false;
	}

	//�R�}���h�L���[�̐���
	{
		D3D12_COMMAND_QUEUE_DESC desc = {};
		desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;	//�R�}���h���X�g�̎��
		desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;	//�R�}���h�L���[�̗D��x
		desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;	//�R�}���h�L���[�̃t���O
		desc.NodeMask = 0;	//�}���`GPU�̍ۂɎg�p����}�X�N

		hr = m_pDevice->CreateCommandQueue(&desc, IID_PPV_ARGS(&m_pQueue));
		if (FAILED(hr))
		{
			return false;
		}
	}

	//�X���b�v�`�F�C���̍쐬
	{
		//DXGI�t�@�N�g���[�̐���. DXGI�t�@�N�g���[�̓r�f�I�O���t�B�b�N�̐ݒ�̗񋓂�w��Ɏg�p������A�X���b�v�`�F�C���̍쐬�Ɏg�p����
		IDXGIFactory4* pFactory = nullptr;
		hr = CreateDXGIFactory1(IID_PPV_ARGS(&pFactory));
		if (FAILED(hr))
		{
			return false;
		}

		//�X���b�v�`�F�C���̐ݒ�
		DXGI_SWAP_CHAIN_DESC desc = {};
		desc.BufferDesc.Width = m_width;	//�o�b�N�o�b�t�@�̕�
		desc.BufferDesc.Height = m_height;	//�o�b�N�o�b�t�@�̍���
		desc.BufferDesc.RefreshRate.Numerator = 60;	//���t���b�V�����[�g�̕���
		desc.BufferDesc.RefreshRate.Denominator = 1;	//���t���b�V�����[�g�̕��q
		desc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;	//�X�L�������C���̕`�揇��
		desc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;	//�E�B���h�E�`�掞�̃X�P�[�����O�̎w��. �o�b�N�o�b�t�@�̃T�C�Y�ʂ�ɕ`�悵����A�E�B���h�E�T�C�Y�ɍ��킹��Ȃǎw��\
		desc.SampleDesc.Count = 1;	//�}���`�T���v�����O�̐�
		desc.SampleDesc.Quality = 0;	//�}���`�T���v�����O�̕i��
		desc.BufferUsage = DXGI_USAGE_BACK_BUFFER;	//�o�b�N�o�b�t�@�̎g�p�@. DXGI_USAGE_BACK_BUFFER�̓o�b�N�o�b�t�@�Ƃ��Ďg�p���邱�Ƃ�����
		desc.BufferCount = FrameCount;	//�X���b�v�`�F�C���̃o�b�t�@��
		desc.OutputWindow = m_hWnd;	//�E�B���h�E�n���h��
		desc.Windowed = TRUE;	//�E�B���h�E���[�h���t���X�N���[�����[�h��
		desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;	//�o�b�N�o�b�t�@����ւ����̌��ʎw��. DXGI_SWAP_EFFECT_FLIP_DISCARD�̓t���b�v��Ƀo�b�N�o�b�t�@�̓��e��j������
		desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;	//�X���b�v�`�F�C���̓���I�v�V����

		//�X���b�v�`�F�C���̐���
		IDXGISwapChain* pSwapChain = nullptr;
		hr = pFactory->CreateSwapChain(m_pQueue, &desc, &pSwapChain);
		if (FAILED(hr))
		{
			SafeRelease(pFactory);
			return false;
		}

		//IDXGISwapChain3: �����_�����O���ꂽ�f�[�^���o�͂ɕ\������O�Ɋi�[���邽�߂̃T�[�t�F�X(=�����_�����O���ʕۑ�����o�b�t�@�̂���)��񋟂���
		//IDXGISwapChain3�ł̓o�b�N �o�b�t�@�[�̃C���f�b�N�X�̎擾�ƐF��Ԃ̃T�|�[�g���ǉ�����Ă���

		// IDXGISwapChain3�̎擾
		hr = pSwapChain->QueryInterface(IID_PPV_ARGS(&m_pSwapChain));
		if (FAILED(hr))
		{
			SafeRelease(pFactory);
			SafeRelease(pSwapChain);
			return false;
		}

		//�o�b�N�o�b�t�@�ԍ��̎擾
		m_FrameIndex = m_pSwapChain->GetCurrentBackBufferIndex();

		//�s�v�ɂȂ����̂ŉ��
		SafeRelease(pFactory);
		SafeRelease(pSwapChain);
	}

	//�R�}���h�A���P�[�^�̐���
	//�A���P�[�^�̓������[�̈�̊��蓖�Ă��s���I�u�W�F�N�g
	//�R�}���h�A���P�[�^�̓R�}���h���X�g�̎��s�ɕK�v�ȃ������[�����蓖�Ă�
	{
		for (auto i = 0; i < FrameCount; i++)
		{
			hr = m_pDevice->CreateCommandAllocator(
				D3D12_COMMAND_LIST_TYPE_DIRECT,	//�R�}���h���X�g�̎��. D3D12_COMMAND_LIST_TYPE_DIRECT �� GPU�Ŏ��s�ł���R�}���h�o�b�t�@�[���w��
				IID_PPV_ARGS(&m_pCmdAllocator[i]));	//�������ꂽ�R�}���h�A���P�[�^
			if (FAILED(hr))
			{
				return false;
			}
		}
	}

	//�R�}���h���X�g�̍쐬
	{
		hr = m_pDevice->CreateCommandList(
			0,	//�R�}���h���X�g�̃m�[�h�}�X�N. �}���`GPU�̍ۂɎg�p����}�X�N
			D3D12_COMMAND_LIST_TYPE_DIRECT,	//�R�}���h���X�g�̎��. CreateCommandAllocator()�Ɠ���
			m_pCmdAllocator[m_FrameIndex],	//�R�}���h�A���P�[�^
			nullptr,	//�p�C�v���C���X�e�[�g�I�u�W�F�N�g. �p�C�v���C�����w��ł���. nullptr���w�肷��ƃf�t�H���g�̃p�C�v���C�����g�p�����
			IID_PPV_ARGS(&m_pCmdList));	//�������ꂽ�R�}���h���X�g
		if (FAILED(hr))
		{
			return false;
		}
	}

	//Direct3D�ł͕`��悪�����_�[�^�[�Q�b�g�ŁA���̃����_�[�^�[�Q�b�g�̎��Ԃ̓o�b�N�o�b�t�@��e�N�X�`���Ƃ��������\�[�X.
	//���\�[�X�̓������̈�����I�u�W�F�N�g�͂킩���Ă��邪�AGPU��̂ǂ̃A�h���X�̂��̂��͂킩��Ȃ�.
	//�܂��A�o�b�t�@�e�ʂ͂킩���Ă��A�ǂ̂悤�Ȏ�ނ̃o�b�t�@��(2�����e�N�X�`���H�L���[�u�}�b�v�H)�͂킩�炸�A�p�C�v���C����łǂ̂悤�Ɏg���΂悢�����킩��Ȃ�
	//�����Ń��\�[�X���ǂ̂悤�Ɉ��������w�肷��̂����\�[�X�r���[�I�u�W�F�N�g
	//�����_�[�^�[�Q�b�g�̃��\�[�X�I�u�W�F�N�g�́A�����_�[�^�[�Q�b�g�r���[�ƌĂ΂��

	//�����_�[�^�[�Q�b�g�r���[�̐���
	{
		//�����_�[�^�[�Q�b�g�r���[�ɂ� ���\�[�X�̐����ƃf�B�X�N���v�^�q�[�v�̍쐬���K�v
		//���\�[�X�̓X���b�v�`�F�C�������o�b�t�@���g�p���邽�߁A�V�K�ł̍쐬�͕s�v

		//�f�B�X�N���v�^�q�[�v�̐ݒ�. �f�B�X�N���v�^�q�[�v�̓f�B�X�N���v�^���i�[���邽�߂̃������̈�(�z��̂悤�Ȃ���)
		D3D12_DESCRIPTOR_HEAP_DESC desc = {};
		desc.NumDescriptors = FrameCount;	//�f�B�X�N���v�^�̐�
		desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;	//�f�B�X�N���v�^�̎��. D3D12_DESCRIPTOR_HEAP_TYPE_RTV�̓����_�[�^�[�Q�b�g�r���[���w��
		desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;	//�f�B�X�N���v�^�q�[�v�̃t���O
		desc.NodeMask = 0;	//�}���`GPU�̍ۂɎg�p����}�X�N

		//�f�B�X�N���v�^�q�[�v�̐���
		hr = m_pDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_pHeapRTV));
		if (FAILED(hr))
		{
			return false;
		}

		//�����_�[�^�[�Q�b�g�r���[�̍쐬
		auto handle = m_pHeapRTV->GetCPUDescriptorHandleForHeapStart();	//�f�B�X�N���v�^�q�[�v�̐擪�̃n���h�����擾
		auto incrementSize = m_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);	//�f�B�X�N���v�^�q�[�v�̃C���N�������g�T�C�Y���擾

		for (auto i = 0u; i < FrameCount; i++)
		{
			hr = m_pSwapChain->GetBuffer(i, IID_PPV_ARGS(&m_pColorBuffers[i]));	//�X���b�v�`�F�C������o�b�t�@���擾
			if (FAILED(hr))
			{
				return false;
			}

			//�����_�[�^�[�Q�b�g�r���[�̐ݒ�
			D3D12_RENDER_TARGET_VIEW_DESC viewDesc = {};
			viewDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;	//�t�H�[�}�b�g
			viewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;	//�r���[�̎��. D3D12_RTV_DIMENSION_TEXTURE2D��2�����e�N�X�`�����w��
			viewDesc.Texture2D.MipSlice = 0;	//�~�b�v�}�b�v���x���̎w��
			viewDesc.Texture2D.PlaneSlice = 0;	//�e�N�X�`���̖ʂ̎w��(���ʂ𕡐������f�[�^�ł͂Ȃ��̂�0���w��)

			//�����_�[�^�[�Q�b�g�r���[�̐���
			m_pDevice->CreateRenderTargetView(m_pColorBuffers[i], &viewDesc, handle);

			m_HandleRTV[i] = handle;
			handle.ptr += incrementSize;
		}
	}

	//�t�F���X�̍쐬
	hr = m_pDevice->CreateFence(
		m_FenceCounter[m_FrameIndex],	//�t�F���X�̏����l
		D3D12_FENCE_FLAG_NONE,	//�t�F���X�̃t���O
		IID_PPV_ARGS(&m_pFence));	//�������ꂽ�t�F���X
	if (FAILED(hr))
	{
		return false;
	}

	//�`�揈�����I���܂őҋ@����C�x���g�̍쐬
	m_FenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	if (m_FenceEvent == nullptr)
	{
		return false;
	}
}

/// <summary>
/// �`�揈��
/// </summary>
void App::Render()
{
	//�R�}���h�̋L�^�J�n
	m_pCmdAllocator[m_FrameIndex]->Reset();
	m_pCmdList->Reset(m_pCmdAllocator[m_FrameIndex], nullptr);

	//���\�[�X�o���A�̐ݒ�. ���\�[�X�o���A�Ƃ�GPU�̕\���������ɕ`��R�}���h�����s����A�\�����o�O��Ƃ������悤�Ȋ��荞�݂ɂ��o�O��h�����߂ɁA�g�p���̊��荞�݂�r������d�g��
	D3D12_RESOURCE_BARRIER barrier = {};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;	//���\�[�X�o���A�̎��. D3D12_RESOURCE_BARRIER_TYPE_TRANSITION�̓��\�[�X�̏�Ԃ�ύX����
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;	//���\�[�X�o���A�̃t���O
	barrier.Transition.pResource = m_pColorBuffers[m_FrameIndex];	//���\�[�X�o���A�̑ΏۂƂȂ郊�\�[�X
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;	//���\�[�X�o���A�̕ύX�O�̃��\�[�X�̏��. D3D12_RESOURCE_STATE_PRESENT �̓��\�[�X���X���b�v�`�F�C���ɕ\������Ă�����
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;	//���\�[�X�o���A�̕ύX��̃��\�[�X�̏��. D3D12_RESOURCE_STATE_RENDER_TARGET�̓��\�[�X�������_�[�^�[�Q�b�g�Ƃ��Ďg�p����Ă�����
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;	//���\�[�X�o���A�̑ΏۂƂȂ�T�u���\�[�X

	//���\�[�X�o���A�̐ݒ�
	m_pCmdList->ResourceBarrier(1, &barrier);

	//�����_�[�^�[�Q�b�g�̐ݒ�
	m_pCmdList->OMSetRenderTargets(1, &m_HandleRTV[m_FrameIndex], FALSE, nullptr);

	//�N���A�J���[�̐ݒ�
	float clearColor[] = { 0.25f, 0.25f, 0.25f, 1 };

	//�����_�[�^�[�Q�b�g�r���[���N���A
	m_pCmdList->ClearRenderTargetView(m_HandleRTV[m_FrameIndex], clearColor, 0, nullptr);

	//�`�揈��
	{
		//TODO: 
	}

	//���\�[�X�o���A�̐ݒ�
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;	//���\�[�X�o���A�̎��. D3D12_RESOURCE_BARRIER_TYPE_TRANSITION�̓��\�[�X�̏�Ԃ�ύX����
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;	//���\�[�X�o���A�̃t���O
	barrier.Transition.pResource = m_pColorBuffers[m_FrameIndex];	//���\�[�X�o���A�̑ΏۂƂȂ郊�\�[�X
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;	//���\�[�X�o���A�̕ύX�O�̃��\�[�X�̏��. D3D12_RESOURCE_STATE_RENDER_TARGET�̓��\�[�X�������_�[�^�[�Q�b�g�Ƃ��Ďg�p����Ă�����
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;	//���\�[�X�o���A�̕ύX��̃��\�[�X�̏��. D3D12_RESOURCE_STATE_PRESENT�̓��\�[�X���X���b�v�`�F�C���ɕ\������Ă�����
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;	//���\�[�X�o���A�̑ΏۂƂȂ�T�u���\�[�X

	//���\�[�X�o���A�̐ݒ�
	m_pCmdList->ResourceBarrier(1, &barrier);

	//�R�}���h�̋L�^�I��
	m_pCmdList->Close();

	//�R�}���h�̎��s
	ID3D12CommandList* ppCmdLists[] = { m_pCmdList };		//�R�}���h���X�g�̔z��(�R�}���h�L���[�ł͂Ȃ�)
	m_pQueue->ExecuteCommandLists(1, ppCmdLists);	//�R�}���h���X�g�̎��s

	//��ʂɕ\��
	Present(1);
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
