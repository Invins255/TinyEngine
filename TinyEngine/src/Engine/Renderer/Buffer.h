#pragma once

namespace Engine
{
	//-------------------------------------------------------------------------
	//Buffer Layout
	//-------------------------------------------------------------------------
	enum class ShaderDataType
	{
		None = 0, 
		Float, Float2, Float3, Float4,
		Int, Int2, Int3, Int4,
		Mat3, Mat4,
		Bool
	};

	/// <summary>
	/// ��ѯShaderDataType��Ӧ�����ݳ��ȡ�
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
	/// Buffer�����е�����Ԫ�أ���¼��һ������Ԫ�أ������ꡢ��ɫ�����ߣ����������͡���С�������е�ƫ��ֵ�ȡ�
	/// </summary>
	struct BufferElement
	{
		ShaderDataType Type;
		std::string Name;
		uint32_t Size;
		uint32_t Offset;
		bool Normalized;

		BufferElement() :
			Type(ShaderDataType::None), Name(""), Size(0), Offset(0), Normalized(false)
		{
		}
		BufferElement(ShaderDataType type, const std::string& name, bool normalized = false) :
			Type(type), Name(name), Size(ShaderDataTypeSize(type)), Offset(0), Normalized(normalized)
		{
		}

		/// <summary>
		/// ��ѯ��ɸ�����Ԫ�ص���ֵ������
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
	/// Buffer�����ݲ��֣��洢���BufferElement���涨�����ݵķֲ���ʽ��
	/// </summary>
	class BufferLayout
	{
	public:
		BufferLayout() = default;
		BufferLayout(const std::initializer_list<BufferElement>& elements) :
			m_Elements(elements) 
		{
			CalculateOffsetsAndStride();
		}

		const std::vector<BufferElement>& GetElements() const { return m_Elements; }
		uint32_t GetStride() const { return m_Stride; }

		std::vector<BufferElement>::iterator begin() { return m_Elements.begin(); }
		std::vector<BufferElement>::iterator end() { return m_Elements.end(); }
		std::vector<BufferElement>::const_iterator begin() const { return m_Elements.begin(); }
		std::vector<BufferElement>::const_iterator end() const { return m_Elements.end(); }
	private:
		/// <summary>
		/// �������Element���ڲ����е�ƫ�����Լ����������С
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
		std::vector<BufferElement> m_Elements;
		uint32_t m_Stride = 0;
	};

	//-------------------------------------------------------------------------
	//VertexBuffer
	//-------------------------------------------------------------------------	
	/// <summary>
	/// VertexBuffer���ڴ洢һ����������ĸ������ݣ�ͨ�����ֶԸ������ݵķ�Χ���б�ʶ��
	/// </summary>
	class VertexBuffer
	{
	public:
		virtual ~VertexBuffer() = default;

		virtual uint32_t GetRendererID() const = 0;
		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		virtual const BufferLayout& GetLayout() const = 0;
		virtual void SetLayout(const BufferLayout& layout) = 0;

		virtual void SetData(const void* data, uint32_t size) = 0;

		/// <summary>
		/// VertexBuffer��������
		/// </summary>
		/// <param name="size">�����ܳ���</param>
		static Ref<VertexBuffer> Create(uint32_t size);
		/// <summary>
		/// VertexBuffer��������
		/// </summary>
		/// <param name="vertices">������������</param>
		/// <param name="size">�����ܳ���</param>
		static Ref<VertexBuffer> Create(float* vertices, uint32_t size);
	};

	//-------------------------------------------------------------------------
	//IndexBuffer
	//-------------------------------------------------------------------------
	/// <summary>
	/// IndexBuffer���ڴ洢һ�������������ͼԪ�Ķ����±ꡣ
	/// </summary>
	class IndexBuffer
	{
	public:
		virtual ~IndexBuffer() = default;

		virtual uint32_t GetRendererID() const = 0;
		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		virtual uint32_t GetCount() const = 0;

		/// <summary>
		/// IndexBuffer��������
		/// </summary>
		/// <param name="vertices">�����±�����</param>
		/// <param name="size">�����ܳ���</param>
		static Ref<IndexBuffer> Create(uint32_t* indices, uint32_t count);
	};
}