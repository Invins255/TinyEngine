#pragma once

#include "Engine/Core/Core.h"
#include "Engine/Core/Ref.h"

namespace Engine
{
	struct Buffer
	{
		uint8_t* Data;
		uint32_t Size;

		Buffer()
			: Data(nullptr), Size(0)
		{
		}

		Buffer(uint8_t* data, uint32_t size)
			: Data(data), Size(size)
		{
		}

		static Buffer Copy(void* data, uint32_t size)
		{
			Buffer buffer;
			buffer.Allocate(size);
			memcpy(buffer.Data, data, size);
			return buffer;
		}

		void Allocate(uint32_t size)
		{
			delete[] Data;
			Data = nullptr;

			if (size == 0)
				return;

			Data = new uint8_t[size];
			Size = size;
		}

		void ZeroInitialize()
		{
			if (Data)
				memset(Data, 0, Size);
		}

		template<typename T>
		T& Read(uint32_t offset = 0)
		{
			return *(T*)(Data + offset);
		}

		void Write(uint8_t* data, uint32_t size, uint32_t offset = 0)
		{
			ENGINE_ASSERT(offset + size <= Size, "Buffer overflow!");
			memcpy(Data + offset, data, size);
		}

		operator bool() const
		{
			return Data;
		}

		uint8_t& operator[](int index)
		{
			return Data[index];
		}

		uint8_t operator[](int index) const
		{
			return Data[index];
		}

		template<typename T>
		T* As()
		{
			return (T*)Data;
		}

		uint32_t GetSize() const { return Size; }
	};
}