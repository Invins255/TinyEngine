#pragma once

#include "Engine/Core/Core.h"
#include "Engine/Core/Ref.h"
#include "Engine/Renderer/VertexBuffer.h"
#include "Engine/Renderer/IndexBuffer.h"
#include "Engine/Renderer/Shader.h"

namespace Engine
{
	struct PipelineSpecification
	{
		VertexBufferLayout Layout;
	};

	class Pipeline
	{
	public:
		static Ref<Pipeline> Create(const PipelineSpecification& spec);

		virtual ~Pipeline() = default;

		virtual PipelineSpecification& GetSpecification() = 0;
		virtual const PipelineSpecification& GetSpecification() const = 0;

		virtual void BindVertexLayout() const = 0;

	};
}