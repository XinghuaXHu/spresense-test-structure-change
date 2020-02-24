/****************************************************************************
 *
 *      File Name: type_holder.h
 *
 *      Description: Type hold class. Without RTTI and exception.
 *
 *      Notes: (C) Copyright 2012 Sony Corporation
 *
 *      Author: Satoru AIZAWA
 *
 ****************************************************************************
 */
#ifndef TYPE_HOLDER_H_INCLUDED
#define TYPE_HOLDER_H_INCLUDED

#include <stdio.h>		/* printf, size_t */
#include "common_types.h"
#include "common_assert.h"
//#include "AssertInfo.h"

/* typeに対応するクラス識別子を返す */
#define GET_TYPE_ID(type)	(&TypeHolder<type>::type_size)

template<typename T> class TypeHolder;

/*****************************************************************
 * TypeHolder<T>クラスを共通に処理するための基底クラス
 *****************************************************************/
class TypeHolderBase {
public:
	typedef size_t (*id_t)();	/* 型識別子 */

	virtual ~TypeHolderBase() {}

	template<typename T>
	bool is_type() const { return this->id() == GET_TYPE_ID(T); }

	template<typename T>
	T& get() {
//		D_ASSERT2(this->is_type<T>(),
//			AssertParamLog(AssertIdTypeUnmatch, (uint32_t)this->id(), (uint32_t)GET_TYPE_ID(T)));
		return static_cast<TypeHolder<T>*>(this)->get();
	}

	template<typename T>
	const T& get() const {
//		D_ASSERT2(this->is_type<T>(),
//			AssertParamLog(AssertIdTypeUnmatch, (uint32_t)this->id(), (uint32_t)GET_TYPE_ID(T)));
		return static_cast<const TypeHolder<T>*>(this)->get();
	}

	template<typename T>
	T& get_any(bool size_check = true) {
		if (size_check) {
//			D_ASSERT2(sizeof(T) <= this->size(),
//				AssertParamLog(AssertIdSizeError, sizeof(T), this->size()));
		}
		return static_cast<TypeHolder<T>*>(this)->get();
	}

	template<typename T>
	const T& get_any(bool size_check = true) const {
		if (size_check) {
//			D_ASSERT2(sizeof(T) <= this->size(),
//				AssertParamLog(AssertIdSizeError, sizeof(T), this->size()));
		}
		return static_cast<const TypeHolder<T>*>(this)->get();
	}

	size_t size() const { return this->id()(); }

	void dump() const {
		size_t size = this->size();
		const unsigned char* data = &get_any<unsigned char>();
		printf("size:%u, id:%p, data: ", size, this->id());
		for (size_t i = 0; i < MIN(size, 16); ++i) {
			printf("%02x ", data[i]);
		}
		printf("\n");
	}

	virtual id_t id() const = 0;

#ifdef USE_TYPE_HOLDER_ACTION_API
	/* TypeHolderクラスのサブクラスで、必要に応じてオーバーライドして利用 */
	virtual void* action(void*) { return 0; }
	virtual void* action(void*) const { return 0; }
#endif
}; /* TypeBase */

/*****************************************************************
 * 任意の型のインスタンスと型情報を保持するクラス
 * 格納型を一意に識別するため、テンプレート引数は型のみとする
 *****************************************************************/
template<typename T>
class TypeHolder : public TypeHolderBase {
public:
	/* クラス毎に一意な関数が生成される */
	static size_t type_size() { return sizeof(T); }

	TypeHolder() : m_held() {}
	explicit TypeHolder(const T& data) : m_held(data) {}

	T& get() { return m_held; }
	const T& get() const { return m_held; }

	/* クラス毎に一意な関数のアドレスをクラス識別に利用 */
	virtual id_t id() const { return &type_size; }

private:
	T	m_held;
}; /* TypeHolder */

#endif /* TYPE_HOLDER_H_INCLUDED */
