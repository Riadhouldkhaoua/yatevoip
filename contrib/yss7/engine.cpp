/**
 * engine.cpp
 * Yet Another SS7 Stack
 * This file is part of the YATE Project http://YATE.null.ro 
 *
 * Yet Another Telephony Engine - a fully featured software PBX and IVR
 * Copyright (C) 2006 Null Team
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "yatess7.h"
#include <yateversn.h>

namespace TelEngine {

class SignallingThreadPrivate : public Thread
{
public:
    inline SignallingThreadPrivate(SignallingEngine*engine, const char* name, Priority prio, unsigned long usec)
	: Thread(name,prio), m_engine(engine), m_sleep(usec)
	{ }
    virtual ~SignallingThreadPrivate()
	{ }
    virtual void run();

private:
    SignallingEngine* m_engine;
    unsigned long m_sleep;
};

};

using namespace TelEngine;

SignallingComponent::~SignallingComponent()
{
    detach();
}

const String& SignallingComponent::toString()
{
    return m_name;
}

void SignallingComponent::insert(SignallingComponent* component)
{
    if (!component)
	return;
    if (m_engine) {
	// we have an engine - force the other component in the same
	m_engine->insert(component);
	return;
    }
    if (component->engine())
	// insert ourselves in the other's engine
	component->engine()->insert(this);
}

void SignallingComponent::detach()
{
    if (m_engine) {
	m_engine->remove(this);
	m_engine = 0;
    }
}

void SignallingComponent::timerTick(const Time& when)
{
}


SignallingEngine::SignallingEngine()
    : Mutex(true), m_thread(0), m_listChanged(true)
{
    debugName("signalling");
}

SignallingEngine::~SignallingEngine()
{
    lock();
    stop();
    m_components.clear();
    unlock();
}

SignallingComponent* SignallingEngine::find(const String& name)
{
    Lock lock(this);
    return static_cast<SignallingComponent*>(m_components[name]);
}

void SignallingEngine::insert(SignallingComponent* component)
{
    if (!component)
	return;
    if (component->engine() == this)
	return;
    Lock lock(this);
    component->detach();
    component->m_engine = this;
    m_components.append(component);
}

void SignallingEngine::remove(SignallingComponent* component)
{
    if (!component)
	return;
    if (component->engine() != this)
	return;
    Lock lock(this);
    component->m_engine = 0;
    component->detach();
    m_components.remove(component,false);
}

bool SignallingEngine::remove(const String& name)
{
    if (name.null())
	return false;
    Lock lock(this);
    SignallingComponent* component = static_cast<SignallingComponent*>(m_components[name]);
    if (!component)
	return false;
    component->m_engine = 0;
    component->detach();
    m_components.remove(component);
    return true;
}

bool SignallingEngine::start(const char* name, Thread::Priority prio, unsigned long usec)
{
    Lock lock(this);
    if (m_thread)
	return m_thread->running();
    // sanity check - 20ms is long enough
    if (usec > 20000)
	usec = 20000;
    SignallingThreadPrivate* tmp = new SignallingThreadPrivate(this,name,prio,usec);
    if (tmp->startup()) {
	m_thread = tmp;
	return true;
    }
    delete tmp;
    return false;
}

void SignallingEngine::stop()
{
    lock();
    SignallingThreadPrivate* tmp = m_thread;
    m_thread = 0;
    if (tmp)
	delete tmp;
    unlock();
}

Thread* SignallingEngine::thread() const
{
    return m_thread;
}

void SignallingEngine::timerTick(const Time& when)
{
    lock();
    m_listChanged = false;
    for (ObjList* l = &m_components; l; l = l->next()) {
	SignallingComponent* c = static_cast<SignallingComponent*>(l->get());
	if (c) {
	    c->timerTick(when);
	    // if the list was changed (can be only from this thread) we
	    //  break out and get back later - cheaper than using a ListIterator
	    if (m_listChanged)
		break;
	}
    }
    unlock();
}


void SignallingThreadPrivate::run()
{
    for (;;) {
	if (m_engine) {
	    Time t;
	    m_engine->timerTick(t);
	    if (m_sleep) {
		usleep(m_sleep,true);
		continue;
	    }
	}
	yield(true);
    }
}

/* vi: set ts=8 sw=4 sts=4 noet: */
