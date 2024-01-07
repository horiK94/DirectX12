struct VSInput		//���_�f�[�^�Ƃ��ēn�����f�[�^
{
	//POSITION: �Z�}���e�B�N�X. �ǂ����߂��邩��`������̂œ��̓f�[�^�͎��R�ɂ�����
	float3 Position : POSITION;		//�ʒu���
	float4 Color : COLOR;			//���_�J���[
};

cbuffer Transform : register(b0)		//�萔�o�b�t�@.�@�萔�o�b�t�@�p�̃��W�X�^��0�Ԗڂ��g�p���邱�Ƃ��Ӗ�����
{
	//�s���4x4��float�^(=16��)�Œ�`����
	// : backoffset(cX) �萔�o�b�t�@�̐擪����̃I�t�Z�b�g���w�肷��
	// c0���擪�f�[�^. c1���擪����float4����16byte��
	float4x4 World : packoffset(c0);		//���[���h�s��
	float4x4 View : packoffset(c4);		//�r���[�s��
	float4x4 Proj : packoffset(c8);	//�v���W�F�N�V�����s��
};

struct VSOutput
{
	//�o�̓f�[�^�Ɋւ��Ă̓V�X�e���l�Z�}���e�B�N�X�ƌĂ΂��V�X�e���Œ�`���ꂽ����ȃZ�}���e�B�N�X������
	//�ǂ̃V�F�[�_�[�X�e�[�W�ŃV�X�e���l�Z�}���e�B�N�X���g�p���邩�͌��܂��Ă���ASV_�Ŏn�܂�
	float4 Position : SV_POSITION;	//���_���W. ���X�^���C�U�[�Ɉʒu���W��ݒ肷��K�v������̂�SV_POSITION���g�p����
	float4 Color : COLOR;			//���_�J���[. �V�X�e���Œ�`���ꂽ���m�ɂ���K�v�͂Ȃ��̂�COLOR�Ƃ���
};

float4 main( float4 pos : POSITION ) : SV_POSITION
{
	return pos;
}

//���_�V�F�[�_�[�̃G���g���[�|�C���g
VSOutput main(VSInput input)
{
	VSOutput output = (VSOutput)0;

	float4 localPos = float4(input.Position, 1.0f);
	float4 worldPos = mul(localPos, World);		//TODO: �t
	float4 viewPos = mul(worldPos, View);		//TODO: �t
	float4 projPos = mul(viewPos, Proj);		//TODO: �t

	output.Position = projPos;
	output.Color = input.Color;

	return output;
}