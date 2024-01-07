struct PSInput
{
	//���_�V�F�[�_�[�Ɠ����\���̂��`. �w�b�_�t�@�C��(.hlsli)���쐬���ċ��ʂ̍\���̂��`����̂��ǂ�
	float4 Position : SV_POSITION;	//���_���W
	float4 Color : COLOR;			//���_�J���[
};

struct PSOutput
{
	//�o�͐�̃����_�[�^�[�Q�b�g�̃s�N�Z���v�f�ɍ��킹��. DXGI_FORMAT_R8G8B8A8_UNORM_SRGB
	//�����_�[�^�[�Q�b�g�z���0�Ԗڂɏ������ނ̂�SV_TARGET0���w�肷��. 1�ԖڂȂ�SV_TARGET1, 2�ԖڂȂ�SV_TARGET2, ...
	float4 color : SV_TARGET0;		//�o�͐F
};

float4 main() : SV_TARGET
{
	return float4(1.0f, 1.0f, 1.0f, 1.0f);
}

//�s�N�Z���V�F�[�_�[�̃G���g���[�|�C���g
PSOutput main(PSInput input)
{
	PSOutput output = (PSOutput)0;

	output.color = input.Color;

	return output;
}