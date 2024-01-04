#pragma once

#include <Windows.h>
#include <cstdint>
#include <d3d12.h>	// Direct3D12
#include <dxgi1_4.h>	// DXGI: DirectX�̃O���t�B�b�N�X�����^�C���Ɉˑ����Ȃ�(�����o�[�W������Direct2D�ɋ��ʂ̃t���[�����[�N��񋟂���)
// �h���C�o�[��n�[�h�E�F�A�̒ʐM�Ƃ������჌�x���̃^�X�N���Ǘ�. ���̂悤�ȉӏ��͕ω����ɂ₩�Ȃ��߁A�ω����p�ɂ�DirectX��API�Ƃ͕�������Ă���

// �����J�[�Ƀ��C�u�����������N����悤�Ɏw��. �����J�[�́A�����N���Ɏw�肳�ꂽ���C�u������T���A�K�v�ȃI�u�W�F�N�g�t�@�C���������N����
// �����N�Ƃ́A�����̃I�u�W�F�N�g�t�@�C�����������āA���s�t�@�C�����쐬���邱��
#pragma comment(lib, "d3d12.lib")	// Direct3D12
#pragma comment(lib, "dxgi.lib")	// DXGI

class App
{
public:
	App(uint32_t width, uint32_t height);
	~App();
	void Run();

private:
	static const uint32_t FrameCount = 2;	// �o�b�N�o�b�t�@�̐�

	HINSTANCE m_hInst;
	HWND m_hWnd;
	uint32_t m_width;
	uint32_t m_height;

	ID3D12Device* m_pDevice;	// Direct3D12�f�o�C�X
	ID3D12CommandQueue* m_pQueue;	// �R�}���h�L���[
	IDXGISwapChain3* m_pSwapChain;	// �X���b�v�`�F�C��
	ID3D12Resource* m_pColorBuffers[FrameCount];	// �J���[�o�b�t�@
	ID3D12CommandAllocator* m_pCmdAllocator[FrameCount];	// �R�}���h�A���P�[�^. �R�}���h���X�g�̃��������Ǘ�����(�炵��)
	ID3D12GraphicsCommandList* m_pCmdList;	// �R�}���h���X�g
	ID3D12DescriptorHeap* m_pHeapRTV;	// �����_�[�^�[�Q�b�g�r���[�p�f�B�X�N���v�^�q�[�v
	ID3D12Fence* m_pFence;	// �t�F���X
	HANDLE m_FenceEvent;	// �t�F���X�C�x���g
	uint64_t m_FenceCounter[FrameCount];	// �t�F���X�J�E���^
	uint32_t m_FrameIndex;	// �t���[���ԍ�
	D3D12_CPU_DESCRIPTOR_HANDLE m_HandleRTV[FrameCount];	// �����_�[�^�[�Q�b�g�r���[��CPU�f�B�X�N���v�^�n���h��

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

