#pragma once
#include <IvConstantBuffer.h>
namespace cali
{
	template <typename T>
	class constant_buffer_wrapper
	{
	private:
		IvConstantBuffer* m_cbuffer;

		constant_buffer_wrapper& operator=(const constant_buffer_wrapper&) = delete;
	public:
		constant_buffer_wrapper() : m_cbuffer(nullptr) {}
		constant_buffer_wrapper(IvConstantBuffer* cbuffer) : m_cbuffer(cbuffer) {}
		~constant_buffer_wrapper() { delete m_cbuffer; }

		constant_buffer_wrapper(constant_buffer_wrapper&& that)
		{
			m_cbuffer = that.m_cbuffer;
			that.m_cbuffer = nullptr;
		}

		constant_buffer_wrapper& operator=(constant_buffer_wrapper&& rhv)
		{
			m_cbuffer = rhv.m_cbuffer;
			rhv.m_cbuffer = nullptr;
			return *this;
		}

		bool valid() { return m_cbuffer != nullptr; }
		T* operator->() { return reinterpret_cast<T*>(m_cbuffer->Get()); }

		IvConstantBuffer* ivcbuffer() { return m_cbuffer; }
	};
}
