#include "pch.h"
#include "RenderCommandQueue.h"
#include "RenderCommand.h"
#include "Renderer.h"

namespace Engine
{
	const uint32_t RenderCommandQueue::MaxQueueSize = 10 * 1024 * 1024; //10MB

	RenderCommandQueue::RenderCommandQueue()
	{
		m_CommandBuffer = new uint8_t[MaxQueueSize];
		m_CommandBufferPtr = m_CommandBuffer;
		memset(m_CommandBuffer, 0, MaxQueueSize);
	}

	RenderCommandQueue::~RenderCommandQueue()
	{
		if(m_CommandCount!=0)
			Execute();

		delete[] m_CommandBuffer;
	}

	void* RenderCommandQueue::Allocate(RenderCommandFn func, uint32_t size)
	{
		*(RenderCommandFn*)m_CommandBufferPtr = func;
		m_CommandBufferPtr += sizeof(RenderCommandFn);

		*(uint32_t*)m_CommandBufferPtr = size;
		m_CommandBufferPtr += sizeof(uint32_t);

		void* memory = m_CommandBufferPtr;
		m_CommandBufferPtr += size;

		m_CommandCount++;
		return memory;
	}

	void RenderCommandQueue::Execute()
	{
		uint8_t* buffer = m_CommandBuffer;

		RENDERCOMMAND_INFO("--------------------------------------------------------------");
		RENDERCOMMAND_INFO("RenderCommandQueue excute:");
		for (uint32_t i = 0; i < m_CommandCount; i++)
		{
			RenderCommandFn function = *(RenderCommandFn*)buffer;
			buffer += sizeof(RenderCommandFn);

			uint32_t size = *(uint32_t*)buffer;
			buffer += sizeof(uint32_t);
			function(buffer);
			buffer += size;
		}
		RENDERCOMMAND_INFO("--------------------------------------------------------------");

		m_CommandBufferPtr = m_CommandBuffer;
		m_CommandCount = 0;
	}
}