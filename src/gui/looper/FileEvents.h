//
// FileEvents.h - This file is part of the looper library
//
// Copyright (c) 2015 Matthew Brush <mbrush@codebrainz.ca>
// All rights reserved.
//

#ifndef LOOP_FILEEVENTS_H
#define LOOP_FILEEVENTS_H

#include <poll.h>

namespace looper 
{

enum class FileEvents
{
	NONE     = 0,
	INPUT    = POLLIN,
	OUTPUT   = POLLOUT,
	PRIORITY = POLLPRI,
	ERROR    = POLLERR,
	HANGUP   = POLLHUP,
};

} // namespace looper

static inline int operator&(looper::FileEvents ev1, looper::FileEvents ev2)
{
	return static_cast<int>(ev1) & static_cast<int>(ev2);
}

static inline int operator&(int ev1, looper::FileEvents ev2)
{
	return ev1 & static_cast<int>(ev2);
}

static inline looper::FileEvents operator|(looper::FileEvents ev1, looper::FileEvents ev2)
{
	return static_cast<looper::FileEvents>(static_cast<int>(ev1) | static_cast<int>(ev2));
}

#endif // LOOP_FILEEVENTS_H

