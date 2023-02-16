#pragma once

#include <filesystem>

#include "Engine/Renderer/Texture.h"

namespace Engine
{
	class ContentBrowserPanel
	{
	private:
		static const std::filesystem::path m_DefaultAssetPath;

	public:
		ContentBrowserPanel();

		void OnImGuiRender();

	private:
		void TraverseDirectoryTree(std::filesystem::path rootPath);

	private:
		std::filesystem::path m_CurrentDirectory;

		Ref<Texture2D> m_DirectoryIcon;
		Ref<Texture2D> m_FileIcon;
	};
}

