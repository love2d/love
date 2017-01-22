/**
 * Copyright (c) 2006-2017 LOVE Development Team
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


#include "BufferSync.h"

namespace love
{
namespace graphics
{
namespace opengl
{

BufferSync::~BufferSync()
{
	cleanup();
}

void BufferSync::lock(size_t start, size_t length)
{
	Range range = {start, length};
	GLsync sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
	locks.emplace_back(range, sync);
}

void BufferSync::wait(size_t start, size_t length)
{
	Range range = {start, length};
	int lockcount = (int) locks.size();

	for (int i = 0; i < lockcount; i++)
	{
		if (range.overlaps(locks[i].range))
		{
			syncWait(locks[i].sync);
			glDeleteSync(locks[i].sync);

			locks[i] = locks[lockcount - 1];
			locks.pop_back();

			--lockcount;
			--i;
		}
	}
}

void BufferSync::cleanup()
{
	for (const auto &lock : locks)
		glDeleteSync(lock.sync);

	locks.clear();
}

void BufferSync::syncWait(GLsync sync)
{
	GLbitfield flags = 0;
	GLuint64 duration = 0;

	while (true)
	{
		GLenum status = glClientWaitSync(sync, flags, duration);

		if (status == GL_ALREADY_SIGNALED || status == GL_CONDITION_SATISFIED)
			return;

		if (status == GL_WAIT_FAILED)
			return;

		flags = GL_SYNC_FLUSH_COMMANDS_BIT;
		duration = 1000000000; // 1 second in nanoseconds.
	}
}

} // opengl
} // graphics
} // love
