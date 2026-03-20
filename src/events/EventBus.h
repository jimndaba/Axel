#pragma once
#ifndef EVENTBUS_H
#define EVENTBUS_H

#include "core/Core.h"

namespace Axel
{
	struct AX_API IEvent
	{
		virtual ~IEvent() = default;
	};


	class HandlerFunctionBase
	{

	public:

		void exec(const std::shared_ptr<IEvent>& evnt)
		{
			call(evnt);
		}

	private:
		virtual void call(const std::shared_ptr<IEvent>& evnt) = 0;

	};

	template<class T, class EventType>
	class MemberFunctionHandler : public HandlerFunctionBase
	{
	public:
		typedef void (T::* MemberFunction)(const std::shared_ptr<EventType>&);

		MemberFunctionHandler(T* instance, MemberFunction memberFunction) :m_instance(instance), m_memberFunction(memberFunction)
		{

		}

		void call(const std::shared_ptr<IEvent>& evnt) {
			// Cast event to the correct type and call member function
			(m_instance->*m_memberFunction)(std::static_pointer_cast<EventType>(evnt));
		}
	private:
		// Pointer to class instance
		T* m_instance;

		// Pointer to member function
		MemberFunction m_memberFunction;
	};

	typedef std::list<HandlerFunctionBase*> HandlerList;
	class AX_API Eventbus
	{


	public:
		Eventbus();

		template<class T, class EventType>
		static void Subscribe(T* instance, void(T::* memberFunction)(const std::shared_ptr<EventType>&))
		{
			HandlerList* handlers = subscribers[typeid(EventType)];

			//First time initialization
			if (handlers == nullptr) {
				handlers = new HandlerList();

				subscribers[typeid(EventType)] = handlers;
			}

			handlers->push_back(new MemberFunctionHandler<T, EventType>(instance, memberFunction));

		}

		template<typename EventType, typename ...Args>
		static void Publish(Args&& ...args)
		{
			std::shared_ptr<EventType> m_event = std::make_shared<EventType>(args...);


			HandlerList* handlers = subscribers[typeid(*m_event.get())];

			if (handlers == nullptr) {
				return;
			}

			for (auto& handler : *handlers) {
				if (handler != nullptr) {
					handler->exec(m_event);
				}
			}
		}

		static void Clear()
		{
			for (auto& pair : subscribers) {
				// Delete each HandlerList* created with 'new'
				for (auto handler : *pair.second) {
					delete handler;
				}
				delete pair.second;
			}
			subscribers.clear();
		}

	private:
		static std::unordered_map<std::type_index, HandlerList*> subscribers;
	};

    


}


#endif