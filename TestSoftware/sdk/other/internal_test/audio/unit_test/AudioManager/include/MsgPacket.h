/****************************************************************************
 *
 *      File Name: MsgPacket.h
 *
 *      Description: Message Packet class definition.
 *
 *      Notes: (C) Copyright 2012,2015 Sony Corporation
 *
 *      Author: Satoru AIZAWA, Shuji Biwa
 *
 ****************************************************************************
 */
#ifndef MSG_PACKET_H_INCLUDED
#define MSG_PACKET_H_INCLUDED

#include <new>			/* placement new */
//#include <stdio.h>		/* printf */
#include "common_types.h"	/* MIN, uintN_t */
#include "common_assert.h"	/* D_ASSERT */
//#include "SpinLock.h"		/* MEMORY_BARRIER */
#include "get_cpu_id.h"		/* GET_CPU_ID */
#include "type_holder.h"	/* TypeHolder */
#include "msgq_id.h"		/* MSG_PARAM_TYPE_MATCH_CHECK */

/*
 * �e��^��`
 */

typedef uint16_t MsgType;	/* ���b�Z�[�W�^�C�vID */
typedef uint16_t MsgQueId;	/* ���b�Z�[�W�L���[ID */
typedef uint8_t  MsgCpuId;	/* ���b�Z�[�WCPU-ID */
typedef uint8_t  MsgFlags;	/* ���b�Z�[�W�t���O */

enum MsgPri {			/* ���b�Z�[�W�D��x */
	MsgPriNormal,
	MsgPriHigh,
	NumMsgPri		/* �D��x�̐� */
};

/*****************************************************************
 * ���b�Z�[�W�p�P�b�g�w�b�_�N���X
 *****************************************************************/
class MsgPacketHeader {
public:
	/* ��{�I�Ɋe�t���O�́AMessageLib������p�Ƃ���\�� */
	static const MsgFlags MsgFlagNull	= 0x00;	/* �t���O�͋� */
	static const MsgFlags MsgFlagWaitParam	= 0x80;	/* �p�����^�����ݑ҂� */
	static const MsgFlags MsgFlagTypedParam = 0x40; /* �p�����[�^�͌^���������`�� */
	MsgPacketHeader(MsgType type, MsgQueId reply, MsgFlags flags, uint16_t size = 0) :
		m_type(type),
		m_reply(reply),
		m_src_cpu(GET_CPU_ID()),
		m_flags(flags),
		m_param_size(size) {}

	MsgType  getType() const { return m_type; }
	MsgQueId getReply() const { return m_reply; }
	MsgCpuId getSrcCpu() const { return m_src_cpu; }
	MsgFlags getFlags() const { return m_flags; }
	uint16_t getParamSize() const { return m_param_size; }
	void     popParamNoDestruct() { m_param_size = 0; }

//protected:
	bool isSelfCpu() const { return GET_CPU_ID() == getSrcCpu(); }
	bool isTypedParam() const { return (m_flags & MsgFlagTypedParam) != 0; }

//protected:
	MsgType		m_type;
	MsgQueId	m_reply;
	MsgCpuId	m_src_cpu;
	MsgFlags	m_flags;
	uint16_t	m_param_size;
}; /* class MsgPacketHeader */

/*****************************************************************
 * ���b�Z�[�W�p�����^�����݂��Ȃ����Ƃ������N���X
 *****************************************************************/
class MsgNullParam {};

/*****************************************************************
 * �A�h���X�͈̓p�����^�ł��邱�Ƃ������N���X
 *****************************************************************/
class MsgRangedParam {
public:
	MsgRangedParam(const void* param, size_t param_size) :
		m_param(param),
		m_param_size(param_size) {}
	const void*	getParam() const { return m_param; }
	size_t		getParamSize() const { return m_param_size; }

private:
	const void*	m_param;
	size_t		m_param_size;
};

/*****************************************************************
 * ���b�Z�[�W�p�P�b�g�N���X
 * �{�N���X�̃C���X�^���X�̃R�s�[�ł́A�p�����^�̓R�s�[����Ȃ��̂Œ��ӂ��邱��
 *****************************************************************/
class MsgPacket : public MsgPacketHeader {
public:
	template<typename T>
	T moveParam() {
		T param = peekParam<T>();
		popParam<T>();
		return param;
	}

	template<typename T>
	const T& peekParam() const {
//		D_ASSERT2(sizeof(T) == getParamSize(), AssertParamLog(AssertIdSizeError, sizeof(T), getParamSize()));
		if (isTypeCheckEnable()) {
			return reinterpret_cast<const TypeHolderBase*>(&m_param[0])->template get<T>();
		} else {
			return *reinterpret_cast<const T*>(&m_param[0]);
		}
	}

//	template<typename T>
//	const T& peekParamOther() const {
//		D_ASSERT2(sizeof(T) <= getParamSize(), AssertParamLog(AssertIdSizeError, sizeof(T), getParamSize()));
//		return peekParamAny<T>();
//	}

	template<typename T>
	void popParam() {
//		D_ASSERT2(sizeof(T) == getParamSize(), AssertParamLog(AssertIdSizeError, sizeof(T), getParamSize()));
		if (isTypeCheckEnable()) {
			TypeHolderBase* p = reinterpret_cast<TypeHolderBase*>(&m_param[0]);
//			D_ASSERT2(p->template is_type<T>(),
//				AssertParamLog(AssertIdTypeUnmatch, (uint32_t)p->id(), (uint32_t)GET_TYPE_ID(T)));
			p->~TypeHolderBase();
		} else {
			reinterpret_cast<T*>(&m_param[0])->~T();
		}
		m_param_size = 0;
	}

//	void dump() const {
//		printf("T:%04x, R:%04x, C:%02x, F:%02x, S:%04x, P:",
//			m_type, m_reply, m_src_cpu, m_flags, m_param_size);
//		for (uint16_t i = 0; i < MIN(m_param_size, 16); ++i)
//			printf("%02x ", m_param[i]);
//		printf("\n");
//	}

//protected:
	friend class MsgLib;
	friend class MsgQueBlock;
	friend class MsgLog;

	MsgPacket(MsgType type, MsgQueId reply, MsgFlags flags) :
		MsgPacketHeader(type, reply, flags) {}

	void setParam(const MsgNullParam& /* param */, bool /* type_check */) {}

	template<typename T>
	void setParam(const T& param, bool type_check) {
		if (type_check) {
			m_flags |= MsgFlagTypedParam;
			*((T *)&m_param[0]) = param;
		} else {
			*((T *)&m_param[0]) = param;
		}
		m_param_size = sizeof(T);
		MEMORY_BARRIER();
		m_flags &= ~MsgFlagWaitParam;	/* �p�����^�����ݑ҂��t���O���N���A */
	}
	
	
	
	
#if 0
	template<typename T>
	void setParam(const T& param, bool type_check) {
		if (type_check) {
			m_flags |= MsgFlagTypedParam;
			new (&m_param[0]) TypeHolder<T>(param);
		} else {
			new (&m_param[0]) T(param);
		}
		m_param_size = sizeof(T);
		MEMORY_BARRIER();
		m_flags &= ~MsgFlagWaitParam;	/* �p�����^�����ݑ҂��t���O���N���A */
	}

	void setParam(const MsgRangedParam& param, bool /* type_check */) {
		D_ASSERT((param.getParam() != NULL) && (0 < param.getParamSize()));
		memcpy(&m_param[0], param.getParam(), param.getParamSize());
		m_param_size = param.getParamSize();
		MEMORY_BARRIER();
		m_flags &= ~MsgFlagWaitParam;	/* �p�����^�����ݑ҂��t���O���N���A */
	}
#endif
	bool isTypeCheckEnable() const { return MSG_PARAM_TYPE_MATCH_CHECK && isTypedParam(); }

	/* �G���[�`�F�b�N�Ȃ��ŁA�C�ӂ̌^�Ńp�����^���Q�Ƃ��� */
	template<typename T>
	const T& peekParamAny() const {
		if (isTypeCheckEnable()) {
			return reinterpret_cast<const TypeHolderBase*>(&m_param[0])->template get_any<T>(false);
		} else {
			return *reinterpret_cast<const T*>(&m_param[0]);
		}
	}

	/* �_���v���O��p�B�T�C�Y�`�F�b�N�Ȃ��Ȃ̂ŁA�S�~��Ԃ��\������ */
	uint32_t peekParamHead() const { return peekParamAny<uint32_t>(); }

protected:
	uint8_t		m_param[0];
}; /* class MsgPacket */

#endif /* MSG_PACKET_H_INCLUDED */
