#pragma once

#include "Engine/Core/Core.h"
#include "Engine/Renderer/Buffer.h"
#include "Engine/Renderer/Shader.h"

namespace Engine
{
	struct PipelineSpecification
	{
		Ref<Shader> Shader;
		BufferLayout Layout;
	};

	class Pipeline
	{
	public:
		static Ref<Pipeline> Create(const PipelineSpecification& spec);

		virtual ~Pipeline() = default;

		virtual PipelineSpecification& GetSpecification() = 0;
		virtual const PipelineSpecification& GetSpecification() const = 0;

		virtual void Bind() = 0;

		virtual void Initialize() = 0;
	};
}