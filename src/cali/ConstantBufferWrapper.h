#pragma once
#include <IvConstantBuffer.h>
namespace Cali
{
	template <typename T>
	class ConstantBufferWrapper
	{
	private:
		IvConstantBuffer* m_cbuffer;

		ConstantBufferWrapper& operator=(const ConstantBufferWrapper&) = delete;
	public:
		ConstantBufferWrapper() : m_cbuffer(nullptr) {}
		ConstantBufferWrapper(IvConstantBuffer* cbuffer) : m_cbuffer(cbuffer) {}
		~ConstantBufferWrapper() { delete m_cbuffer; }

		ConstantBufferWrapper(ConstantBufferWrapper&& that)
		{
			m_cbuffer = that.m_cbuffer;
			that.m_cbuffer = nullptr;
		}

		ConstantBufferWrapper& operator=(ConstantBufferWrapper&& rhv)
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
