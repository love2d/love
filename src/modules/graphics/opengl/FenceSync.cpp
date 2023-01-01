/**
 * Copyright (c) 2006-2023 LOVE Development Team
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 **/


#include "FenceSync.h"

namespace love
{
namespace graphics
{
namespace opengl
{

FenceSync::~FenceSync()
{
	cleanup();
}

bool FenceSync::fence()
{
	bool wasActive = sync != 0;

	if (wasActive)
		cleanup();

	sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);

	return !wasActive;
}

bool FenceSync::cpuWait()
{
	if (sync == 0)
		return false;

	GLbitfield flags = 0;
	GLuint64 duration = 0;

	while (true)
	{
		GLenum status = glClientWaitSync(sync, flags, duration);

		if (status == GL_ALREADY_SIGNALED || status == GL_CONDITION_SATISFIED)
			break;

		if (status == GL_WAIT_FAILED)
			break;

		flags = GL_SYNC_FLUSH_COMMANDS_BIT;
		duration = 1000000000; // 1 second in nanoseconds.
	}

	cleanup();

	return true;
}

void FenceSync::cleanup()
{
	if (sync != 0)
	{
		glDeleteSync(sync);
		sync = 0;
	}
}

} // opengl
} // graphics
} // love
