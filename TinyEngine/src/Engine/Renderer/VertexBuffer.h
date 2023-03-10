#pragma once

#include "Engine/Core/Ref.h"

namespace Engine
{
	enum class ShaderDataType
	{
		None = 0, 
		Float, Float2, Float3, Float4,
		Int, Int2, Int3, Int4,
		Mat3, Mat4,
		Bool
	};

	/// <summary>
	/// 查询ShaderDataType对应的数据长度。
	/// </summary>
	static uint32_t ShaderDataTypeSize(ShaderDataType type)
	{
		switch (type)
		{
		case Engine::ShaderDataType::Float:		return 4;
		case Engine::ShaderDataType::Float2:	return 4 * 2;
		case Engine::ShaderDataType::Float3:	return 4 * 3;
		case Engine::ShaderDataType::Float4:	return 4 * 4;
		case Engine::ShaderDataType::Int:		return 4;
		case Engine::ShaderDataType::Int2:		return 4 * 2;
		case Engine::ShaderDataType::Int3:		return 4 * 3;
		case Engine::ShaderDataType::Int4:		return 4 * 4;
		case Engine::ShaderDataType::Mat3:		return 4 * 3 * 3;
		case Engine::ShaderDataType::Mat4:		return 4 * 4 * 4;
		case Engine::ShaderDataType::Bool:		return 1;
		default:
			ENGINE_ASSERT(false, "Unknown ShaderDataType!");
			return 0;
		}
	}

	/// <summary>
	/// VertexBuffer布局中的数据元素，记录了一种数据元素（如坐标、颜色、法线）的数据类型、大小、布局中的偏移值等。
	/// </summary>
	struct VertexBufferElement
	{
		ShaderDataType Type;
		std::string Name;
		uint32_t Size;
		uint32_t Offset;
		bool Normalized;

		VertexBufferElement() :
			Type(ShaderDataType::None), Name(""), Size(0), Offset(0), Normalized(false)
		{
		}
		VertexBufferElement(ShaderDataType type, const std::string& name, bool normalized = false) :
			Type(type), Name(name), Size(ShaderDataTypeSize(type)), Offset(0), Normalized(normalized)
		{
		}

		/// <summary>
		/// 查询组成该数据元素的数值个数。
		/// </summary>
		uint32_t GetComponentCount() const
		{
			switch (Type)
			{
			case Engine::ShaderDataType::Float:		return 1;
			case Engine::ShaderDataType::Float2:	return 2;
			case Engine::ShaderDataType::Float3:	return 3;
			case Engine::ShaderDataType::Float4:	return 4;
			case Engine::ShaderDataType::Int:		return 1;
			case Engine::ShaderDataType::Int2:		return 2;
			case Engine::ShaderDataType::Int3:		return 3;
			case Engine::ShaderDataType::Int4:		return 4;
			case Engine::ShaderDataType::Mat3:		return 3 * 3;
			case Engine::ShaderDataType::Mat4:		return 4 * 4;
			case Engine::ShaderDataType::Bool:		return 1;
			default:
				ENGINE_ASSERT(false, "Unknown ShaderDataType!");
				return 0;
			}
		}
	};

	/// <summary>
	/// VertexBuffer的数据布局，存储多个BufferElement，规定了数据的分布方式。
	/// </summary>
	class VertexBufferLayout
	{
	public:
		VertexBufferLayout() = default;
		VertexBufferLayout(const std::initializer_list<VertexBufferElement>& elements) :
			m_Elements(elements) 
		{
			CalculateOffsetsAndStride();
		}

		const std::vector<VertexBufferElement>& GetElements() const { return m_Elements; }
		uint32_t GetStride() const { return m_Stride; }

		std::vector<VertexBufferElement>::iterator begin() { return m_Elements.begin(); }
		std::vector<VertexBufferElement>::iterator end() { return m_Elements.end(); }
		std::vector<VertexBufferElement>::const_iterator begin() const { return m_Elements.begin(); }
		std::vector<VertexBufferElement>::const_iterator end() const { return m_Elements.end(); }
	private:
		/// <summary>
		/// 计算各个Element的在布局中的偏移量以及布局整体大小
		/// </summary>
		void CalculateOffsetsAndStride()
		{
			uint32_t offset = 0;
			m_Stride = 0;
			for (auto& element : m_Elements) {
				element.Offset = offset;
				offset += element.Size;
				m_Stride += element.Size;
			}
		}

	private:
		std::vector<VertexBufferElement> m_Elements;
		uint32_t m_Stride = 0;
	};

	enum class VertexBufferUsage 
	{
		None = 0, Static = 1, Dynamic = 2
	};
	
	/// <summary>
	/// VertexBuffer用于存储一个顶点数组的各类数据，通过布局对各类数据的范围进行标识。
	/// </summary>
	class VertexBuffer
	{
	public:
		virtual ~VertexBuffer() = default;

		virtual uint32_t GetRendererID() const = 0;
		virtual uint32_t GetSize() const = 0;

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		virtual void SetData(void* data, uint32_t size, uint32_t offset = 0) = 0;

		/// <summary>
		/// VertexBuffer工厂函数
		/// </summary>
		/// <param name="size">数组总长度</param>
		static Ref<VertexBuffer> Create(uint32_t size, VertexBufferUsage usage = VertexBufferUsage::Dynamic);
		/// <summary>
		/// VertexBuffer工厂函数
		/// </summary>
		/// <param name="data">顶点数据数组</param>
		/// <param name="size">数组总长度</param>
		static Ref<VertexBuffer> Create(void* data, uint32_t size, VertexBufferUsage usage = VertexBufferUsage::Static);
	};
}