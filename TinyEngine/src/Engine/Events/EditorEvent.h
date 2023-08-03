#pragma once

#include "Engine/Events/Event.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Scene/Entity.h"

#include <sstream>

namespace Engine
{
	class SelectionChangedEvent : public Event
	{
	public:
		SelectionChangedEvent(UUID selectionID, bool selected)
			: m_SelectionID(selectionID), m_Selected(selected)
		{
		}

		UUID GetSelectionID() const { return m_SelectionID; }
		bool IsSelected() const { return m_Selected; }

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "EntitySelectionChangedEvent: Context(" << (int32_t)m_Context << "), Selection(" << m_SelectionID << "), " << m_Selected;
			return ss.str();
		}

		EVENT_CLASS_CATEGORY(EventCategoryScene)
			EVENT_CLASS_TYPE(SelectionChanged)
	private:
		UUID m_SelectionID;
		bool m_Selected;
	};
}