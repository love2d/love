/**
* Copyright (c) 2006-2010 LOVE Development Team
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

#ifndef LOVE_THREAD_SDL_THREAD_H
#define LOVE_THREAD_SDL_THREAD_H

// SDL
#include <SDL_thread.h>
#include <SDL_mutex.h>

// STL
#include <map>
#include <string>

// LOVE
#include <thread/ThreadModule.h>
#include <filesystem/File.h>
#include <common/runtime.h>

namespace love
{
namespace thread
{
namespace sdl
{
	enum ThreadVariantType
	{
		UNKNOWN = 0,
		BOOLEAN,
		NUMBER,
		STRING,
		LUSERDATA,
		FUSERDATA
	};

	class ThreadVariant : public love::Object
	{
	public:
		ThreadVariant(bool boolean);
		ThreadVariant(double number);
		ThreadVariant(const char *string);
		ThreadVariant(void *userdata);
		ThreadVariant(Type udatatype, void *userdata);
		virtual ~ThreadVariant();
		ThreadVariantType type;
		union
		{
			bool boolean;
			double number;
			const char *string;
			void *userdata;
		} data;
		Type udatatype;
		bits flags;
	};

	class ThreadData
	{
	private:
		char *code;
		char *name;
		std::map<std::string, ThreadVariant*> shared;

	public:
		ThreadData(const char *name, const char *code);
		~ThreadData();
		const char *getCode();
		const char *getName();
		ThreadVariant* getValue(const std::string & name);
		void clearValue(const std::string & name);
		void setValue(const std::string & name, ThreadVariant *v);
	};

	class Thread : public love::Object
	{
	private:
		SDL_Thread *handle;
		love::thread::ThreadModule *module;
		ThreadData *comm;
		std::string name;
		char *data;
		SDL_mutex *mutex;
		SDL_cond *cond;
		bool isThread;

	public:
		Thread(love::thread::ThreadModule *module, const std::string & name, love::Data *data);
		Thread(love::thread::ThreadModule *module, const std::string & name);
		virtual ~Thread();
		void start();
		void kill();
		void wait();
		std::string getName();
		ThreadVariant *receive(const std::string & name);
		ThreadVariant *demand(const std::string & name);
		void clear(const std::string & name);
		void send(const std::string & name, ThreadVariant *v);
		void lock();
		void unlock();
	}; // Thread

	typedef std::map<std::string, Thread*> threadlist_t;

	class ThreadModule : public love::thread::ThreadModule
	{
	private:
		threadlist_t threads;

	public:
		ThreadModule();
		virtual ~ThreadModule();
		Thread *newThread(const std::string & name, love::Data *data);
		Thread **getThreads();
		Thread *getThread(const std::string & name);
		void unregister(const std::string & name);
		const char *getName() const;
	}; // ThreadModule
} // sdl
} // thread
} // love

#endif // LOVE_THREAD_SDL_THREAD_H
