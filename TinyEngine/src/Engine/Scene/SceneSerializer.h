#pragma once

#include "Engine/Scene/Scene.h"

namespace Engine
{
#define SERIALIZER_DEBUG 1
#if SERIALIZER_DEBUG
#define SERIALIZER_INFO(...) ENGINE_INFO(__VA_ARGS__)
#define SERIALIZER_WARN(...) ENGINE_WARN(__VA_ARGS__)
#else
#define SERIALIZER_INFO(...)
#define SERIALIZER_WARN(...)
#endif

	class SceneSerializer
	{
	public:
		SceneSerializer(const Ref<Scene>& scene);

		void Serialize(const std::string& filepath);
		bool Deserialize(const std::string& filepath);

	private:
		Ref<Scene> m_Scene;
	};
}