#pragma once

#include "Event.h"
#include "components/Fwd.h"

struct EventList
{
	// window events
	static Event<void, int, int> eResize;

};