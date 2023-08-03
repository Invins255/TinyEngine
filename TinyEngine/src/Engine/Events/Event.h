#pragma once

#include "Engine/Core/Core.h"

namespace Engine 
{
	enum class EventType
	{
		None = 0,
		//Window
		WindowClose, WindowResize, WindowFocus, WindowLostFocus, WindowMoved,
		//Application
		AppTick, AppUpdate, AppRender,
		//Keyboard
		KeyPressed, KeyReleased, KeyTyped,
		//Mouse
		MouseButtonPressed, MouseButtonReleased, MouseMoved, MouseScrolled
	};

	enum EventCategory
	{
		None = 0,
		EventCategoryApplication	= BIT(0),
		EventCategoryInput			= BIT(1),
		EventCategoryKeyboard		= BIT(2),
		EventCategoryMouse			= BIT(3),
		EventCategoryMouseButton	= BIT(4)
	};

	//-------------------------------------------------------------------------
	//Event
	//-------------------------------------------------------------------------
	
	/// <summary>
	/// Event基类	
	/// </summary>
	class Event 
	{
		friend class EventDispatcher;
	public:
		virtual EventType GetEventType() const = 0;
		virtual const char* GetName() const = 0;
		virtual int GetCategoryFlags() const = 0;
		virtual std::string ToString() const { return GetName(); }

		bool IsInCategory(EventCategory category) 
		{
			return GetCategoryFlags() & category;
		}
		
	public:
		bool Handled = false;
	};

	//Event子类宏定义函数
#define EVENT_CLASS_TYPE(type)	static EventType GetStaticType() { return EventType::##type; }\
								virtual EventType GetEventType() const override { return GetStaticType(); }\
								virtual const char* GetName() const override { return #type; }

#define EVENT_CLASS_CATEGORY(category) virtual int GetCategoryFlags() const override { return category; }
	
	//-------------------------------------------------------------------------
	//EventDispatcher
	//-------------------------------------------------------------------------

	/// <summary>
	/// 接收一个Event，进行类型检查并对符合类型的Event进行执行。
	/// </summary>
	class EventDispatcher
	{
		template<typename T>
		using EventFn = std::function<bool(T&)>;

	public:
		EventDispatcher(Event& event) :
			m_Event(event) {}
		
		/// <summary>
		/// 对EventDispatcher享有的Event基于类型T进行检查，符合类型则执行回调函数func
		/// </summary>
		/// <typeparam name="T">需要检查的事件类型</typeparam>
		/// <param name="func">事件回调函数</param>
		/// <returns>是否完成执行</returns>
		template<typename T>
		bool Dispatch(EventFn<T> func)
		{
			if (m_Event.GetEventType() == T::GetStaticType())
			{
				m_Event.Handled = func(*(T*)&m_Event);
				return true;
			}
			return false;
		}

	private:
		Event& m_Event;
	};

	inline std::ostream& operator<<(std::ostream& os, const Event& e)
	{
		return os << e.ToString();
	}
	

}