/**
 * layer3.cpp
 * Yet Another SS7 Stack
 * This file is part of the YATE Project http://YATE.null.ro 
 *
 * Yet Another Telephony Engine - a fully featured software PBX and IVR
 * Copyright (C) 2004-2006 Null Team
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "yatess7.h"


using namespace TelEngine;

SS7MTP3::SS7MTP3(SS7CodePoint::Type type)
    : SS7Layer3(type)
{
}

void SS7MTP3::attach(SS7Layer2* link)
{
    Debug(toString(),DebugStub,"Please implement SS7MTP3::attach()");
    SignallingComponent::insert(link);
}

bool SS7MTP3::receivedMSU(const SS7MSU& msu, SS7Layer2* link)
{
    Debug(toString(),DebugStub,"Please implement SS7MTP3::receivedMSU()");
    unsigned int llen = SS7Label::length(type());
    if (!llen)
	return false;
    // check MSU length against SIO + label length
    if (msu.length() <= llen) {
	XDebug(engine(),DebugMild,"Received short MSU of length %u [%p]",
	    msu.length(),this);
	return false;
    }
    SS7Label label(type(),msu);
    String tmp;
    tmp << label;
    DDebug(name(),DebugInfo,"MSU address: %s",tmp.c_str());

    return false;
}

/* vi: set ts=8 sw=4 sts=4 noet: */
