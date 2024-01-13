#pragma once

#include <Windows.h>
#include <cstdint>
#include <d3d12.h>	// Direct3D12
#include <dxgi1_4.h>	// DXGI: DirectX�̃O���t�B�b�N�X�����^�C���Ɉˑ����Ȃ�(�����o�[�W������Direct2D�ɋ��ʂ̃t���[�����[�N��񋟂���)
// �h���C�o�[��n�[�h�E�F�A�̒ʐM�Ƃ������჌�x���̃^�X�N���Ǘ�. ���̂悤�ȉӏ��͕ω����ɂ₩�Ȃ��߁A�ω����p�ɂ�DirectX��API�Ƃ͕�������Ă���
#include <wrl/client.h>		//�X�}�[�g�|�C���^���g�p���邽�߂�include
#include <DirectXMath.h>

// �����J�[�Ƀ��C�u�����������N����悤�Ɏw��. �����J�[�́A�����N���Ɏw�肳�ꂽ���C�u������T���A�K�v�ȃI�u�W�F�N�g�t�@�C���������N����
// �����N�Ƃ́A�����̃I�u�W�F�N�g�t�@�C�����������āA���s�t�@�C�����쐬���邱��
#pragma comment(lib, "d3d12.lib")	// Direct3D12
#pragma comment(lib, "dxgi.lib")	// DXGI

template<typename T> using ComPtr = Microsoft::WRL::ComPtr<T>;	// ComPtr�́A�X�}�[�g�|�C���^. c++�W�����C�u�����ɂ�unique_ptr��shared_ptr�����邪�AComPtr��Micrsoft�ō�������̂Ȃ̂Őe�a��������

template<typename T>
struct ConstantBufferView
{
	D3D12_CONSTANT_BUFFER_VIEW_DESC Desc;		//�萔�o�b�t�@�̍\���ݒ�
	//�f�B�X�N���v�^: GPU���\�[�X(�e�N�X�`���A�o�b�t�@�Ȃ�)�̎Q�Ƃ�񋟂���f�[�^�\��
	//�n���h��: �|�C���^�̃|�C���^ �� �t�@�C����摜�A�E�B���h�E�Ȃǂ𑀍삵�����Ƃ��ɑΏۂ����ʂ��邽�߂ɌX�̗v�f�Ɋ��蓖�Ă����ӂ̔ԍ����w��
	D3D12_CPU_DESCRIPTOR_HANDLE HandleCPU;		//CPU�f�B�X�N���v�^�n���h��
	D3D12_GPU_DESCRIPTOR_HANDLE HandleGPU;		//GPU�f�B�X�N���v�^�n���h��
	T* pBuffer;		//�o�b�t�@�擪�ւ̃|�C���^(�o�b�t�@�Ƃ̓f�[�^���ꎞ�I�ɕێ����Ă������߂̋L���̈�)
};

struct Vertex		//���_�f�[�^
{
	DirectX::XMFLOAT3 Position;		//�ʒu���
	DirectX::XMFLOAT4 Color;			//���_�J���[
};

//DirectX12�̎d�l��A�萔�o�b�t�@�̓������A���C�������g��256byte�ɂȂ�Ȃ��Ƃ����Ȃ�
struct alignas(256) Transform		//�萔�o�b�t�@�p�f�[�^
{
	DirectX::XMMATRIX World;	//���[���h�s��
	DirectX::XMMATRIX View;		//�r���[�s��
	DirectX::XMMATRIX Proj;		//�v���W�F�N�V�����s��(�ˉe�s��)
};

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

	ComPtr<ID3D12Device> m_pDevice;	// Direct3D12�f�o�C�X
	ComPtr<ID3D12CommandQueue> m_pQueue;	// �R�}���h�L���[
	ComPtr<IDXGISwapChain3> m_pSwapChain;	// �X���b�v�`�F�C��
	ComPtr<ID3D12Resource> m_pColorBuffers[FrameCount];	// �J���[�o�b�t�@
	ComPtr<ID3D12CommandAllocator> m_pCmdAllocator[FrameCount];	// �R�}���h�A���P�[�^. �R�}���h���X�g�̃��������Ǘ�����(�炵��)
	ComPtr<ID3D12GraphicsCommandList> m_pCmdList;	// �R�}���h���X�g
	//�q�[�v�Ƃ́A�R���s���[�^�v���O���������p���郁�����̈�̎�ނ̈�ŁA���s���ɔC�ӂ̃^�C�~���O�Ŋm�ۂ������\�Ȃ���
	ComPtr<ID3D12DescriptorHeap> m_pHeapRTV;	// �����_�[�^�[�Q�b�g�r���[�p�f�B�X�N���v�^�q�[�v
	ComPtr<ID3D12Fence> m_pFence;	// �t�F���X
	ComPtr<ID3D12DescriptorHeap> m_pHeapCBV;	//�萔�o�b�t�@�r���[ / �V�F�[�_�[���\�[�X�r���[ / �A���I�[�_�[�h�A�N�Z�X�r���[�p �f�B�X�N���v�^�q�[�v
	ComPtr<ID3D12Resource> m_pVB;	// ���_�o�b�t�@
	ComPtr<ID3D12Resource> m_pCB[FrameCount];	// �萔�o�b�t�@
	ComPtr<ID3D12RootSignature> m_pRootSignature;	// ���[�g�V�O�l�`��. �V�F�[�_�[�͂ǂ̃��W�X�^�Ƀh�̃f�[�^�������Ă��邩���f�B�X�N���v�^�q�[�v����擾����. ���̃f�B�X�N���v�^�q�[�v�̏����܂Ƃ߂����̂����[�g�V�O�l�`��
	ComPtr<ID3D12PipelineState> m_pPSO;		//�p�C�v���C���X�e�[�g�I�u�W�F�N�g. ���̓X�g���[���A�V�F�[�_�A�X�e�[�g�������ׂĂЂƂ̃I�u�W�F�N�g�̂܂Ƃ߂�����

	HANDLE m_FenceEvent;	// �t�F���X�C�x���g
	uint64_t m_FenceCounter[FrameCount];	// �t�F���X�J�E���^
	uint32_t m_FrameIndex;	// �t���[���ԍ�
	D3D12_CPU_DESCRIPTOR_HANDLE m_HandleRTV[FrameCount];	// �����_�[�^�[�Q�b�g�r���[��CPU�f�B�X�N���v�^�n���h��

	D3D12_VERTEX_BUFFER_VIEW m_VBV;	// ���_�o�b�t�@�r���[
	D3D12_VIEWPORT m_Viewport;	// �r���[�|�[�g
	D3D12_RECT m_Scissor;	// �V�U�[��`
	ConstantBufferView<Transform> m_CBV[FrameCount];	// �萔�o�b�t�@�r���[
	float m_RotateAngle;	// ��]�p

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

