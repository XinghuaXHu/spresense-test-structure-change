==========================================================================

       File Name: readme.txt

       Description: CppUTest Sample�r���h���ɂ���

       Notes: (C) Copyright 2015 Sony Corporation

       Author: Tetsuya Sakai


==========================================================================

�P�A�T�v
�@�{���́ACppUTest���g�p�����e�X�g���s�����߂�Sample�ł��B
�@�{�h�L�������g�͓������g�����̐�����ړI�Ƃ��Ă���A�uCppUTest�Ƃ͉����H�v
�@�ɂ��Ă͐G��܂���B


�Q�A������Ǝg�pVersion
�@�ȉ��̊��ł̂ݓ���m�F�ς݂ł��B
�@�@�v���b�g�t�H�[���Fcygwin�i�W��cygwin�j
�@�@                  Cygwin��Setup�ŁADevel - autoconf��ǉ����邱��
�@�@�R���p�C��      �FGCC 3.4.4
�@�@CppUTest        �F3.6


�R�A�f�B���N�g���\��
�@�J���Ώۂɂ���Ď኱�̈Ⴂ�͂���܂����A��{�I�ɂ͉��L�\�����Ƃ��Ă��܂��B
�@����̕t���Ă���f�B���N�g���́A�ڍׂ���q���܂��B
�@�@ Sample/test           �c �e�X�g�R�[�h
           /src            �c �X�^�u�E�_�~�[�R�[�h��
           /include        �c �e�X�g�w�b�_
           /doc            �c Sample�h�L�������g
           /build          �c �r���h�y�уv���O�������s ��
           /config         �c �R���t�B�O���[�V�����t�@�C���Q ��


�S�Aconfig�f�B���N�g��
�@�t�@�C���Ɨp�r�͈ȉ��̒ʂ�ł��B

�@�@config.mk       �c �R���t�B�O���[�V���������L�q����t�@�C���B
                       ������Makefile�ŋ��ʂɎg�p����make�ϐ��̒�`�A
                       ���[�U�}�N���̒�`�i-D�ł̒�`�j��ړI�Ƃ����t�@�C���B
                       ���̃t�@�C���ɕύX��������ƁA�S�Ẵ\�[�X�Ƀ��R���p�C����������B
                       ������Makefile�ŋ��ʂɎg�p����make�ϐ��̒�`�́A�����ɋL�q���邱�ƁB

�@�@paths.mk        �c �p�X�����L�q�����t�@�C���B
                       ������Makefile�ŋ��ʂɎg�p����p�X���̒�`��ړI�Ƃ����t�@�C���B
                       �K�v�ɉ����ĒǋL���ėǂ��B

�@�@rules.mk        �c ���[�����L�q�����t�@�C���B
                       �e�X�g���Ǝ��̃��[�����L�q���鎖��ړI�Ƃ����t�@�C���B
                       Terroir�ɂ͊g���qcpp�̃t�@�C�����R���p�C�����郋�[���������̂Œǉ����Ă���B
                       �K�v�ɉ����ĒǋL���ėǂ��B

�@�@base_config.mk  �c Terroir��config�t�@�C�������L�q�����t�@�C���B
                       Smaple����copy���Ďg���ꍇ�A�f�B���N�g���K�w���ς���Ă��܂���������B
                       ���̏ꍇ�A�{�t�@�C����ROOT_DIR�ϐ���ύX���邱�ƂőΉ��ł���B


�T�Abuild�f�B���N�g��
�@���s�t�@�C�������p��Makefile�ƁA�e�X�g���s�p��Makefile�����݂��܂��B
�@���s�t�@�C����e�X�g���ʂ��A���̏ꏊ�ɐ�������܂��B

�@�@Makefile        �c �e�X�g���s�p��Makefile�B
                       ���s�t�@�C�������݂��Ȃ��ꍇ�́A����������Ńe�X�g�����s����B

�@�@Makefile.build  �c ���s�t�@�C�������p��Makefile�B
                       �r���h�֘A�̕ύX�͂���ɑ΂��čs�����ƁB

�@�@Makefile.lib    �c ���[�U�R�[�h�A�[�J�C�u���p��Makefile�B
                       ���[�U�R�[�h�����݂��Ȃ����ł͕s�g�p�B


�U�A�g�p���@
�@Sample/build�Ɉړ����A�ȉ��̂悤�ɃR�}���h�������ĉ������B
�@�@make            �c �e�X�g�����s����B���s�t�@�C�������݂��Ȃ��ꍇ�́A�r���h��Ƀe�X�g�����s����B
�@�@make build      �c �r���h�̂ݍs���A�e�X�g�͎��s���Ȃ��B
�@�@make clean      �c �e�X�g���s���ʁixml�t�@�C���j���폜����B
�@�@make allclean   �c clean�ɉ����A���s�t�@�C����I�u�W�F�N�g�t�@�C���A�J�o���b�W���t�@�C�����폜����A


�V�A�J�o���b�W�̎���
�@GCC�̋@�\���g�����J�o���b�W�擾�ɂ��Ή����Ă��܂��B
�@�J�o���b�W�擾�̗L��/�����͑I���\�ɂȂ��Ă���A�f�t�H���g�����ƂȂ��Ă��܂��B
�@�L���ɂ���ꍇ�Aconfig.mk�̉��L������ON�ɂ��ĉ������B
�@�@�@#------------------------------------------------#
�@�@�@# Select use coverage ON/OFF.
�@�@�@#------------------------------------------------#
�@�@�@#USE_COVERAGE	= ON
�@�@�@USE_COVERAGE	= OFF



