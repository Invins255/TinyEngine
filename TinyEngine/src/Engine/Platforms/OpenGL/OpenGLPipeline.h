#pragma once

#include "Engine/Renderer/Pipeline.h"

namespace Engine
{
	class OpenGLPipeline : public Pipeline
	{
	public:
		OpenGLPipeline(const PipelineSpecification& spec);
		virtual ~OpenGLPipeline();

		virtual PipelineSpecification& GetSpecification() override { return m_Specification; }
		virtual const PipelineSpecification& GetSpecification() const override { return m_Specification; }

		virtual void Bind() override;
		
		virtual void Initialize() override;

	private:
		PipelineSpecification m_Specification;
		uint32_t m_VertexArrayRendererID = 0;

	};
}