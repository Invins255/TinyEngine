#include <TinyEngine.h>
#include "Engine/Core/EntryPoint.h"
#include "EditorLayer.h"

namespace Engine
{
	class EngineEditor : public Engine::Application
	{
	public:
		EngineEditor():
			Application("Engine Editor")
		{
		}

		~EngineEditor()
		{
		}

		virtual void OnInit() override
		{
			PushLayer(new EditorLayer());
		}
	};

	Application* Engine::CreateApplication()
	{
		return new EngineEditor();
	}
}

