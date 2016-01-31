/**
 * Copyright (c) 2006-2016 LOVE Development Team
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

// LOVE
#include "Module.h"
#include "Exception.h"

// std
#include <map>
#include <utility>
#include <string>

namespace
{
	typedef std::map<std::string, love::Module*> ModuleRegistry;

	// The registry must be dynamically managed, because some modules
	// (the math module) are static globals that are not guaranteed to
	// be destroyed before other static globals.
	ModuleRegistry *registry = nullptr;

	ModuleRegistry &registryInstance()
	{
		if (!registry)
			registry = new ModuleRegistry;

		return *registry;
	}

	void freeEmptyRegistry()
	{
		if (registry && registry->empty())
		{
			delete registry;
			registry = nullptr;
		}
	}

} // anonymous namespace

namespace love
{

Module *Module::instances[] = {};

Module::~Module()
{
	ModuleRegistry &registry = registryInstance();

	// We can't use the overridden Module::getName() in this destructor.
	for (auto it = registry.begin(); it != registry.end(); ++it)
	{
		if (it->second == this)
		{
			registry.erase(it);
			break;
		}
	}

	// Same deal with Module::getModuleType().
	for (int i = 0; i < (int) M_MAX_ENUM; i++)
	{
		if (instances[i] == this)
			instances[i] = nullptr;
	}

	freeEmptyRegistry();
}

void Module::registerInstance(Module *instance)
{
	if (instance == nullptr)
		throw Exception("Module instance is null");

	std::string name(instance->getName());

	ModuleRegistry &registry = registryInstance();

	auto it = registry.find(name);

	if (it != registry.end())
	{
		if (it->second == instance)
			return;
		throw Exception("Module %s already registered!", instance->getName());
	}

	registry.insert(make_pair(name, instance));

	ModuleType moduletype = instance->getModuleType();

	if (instances[moduletype] != nullptr)
	{
		printf("Warning: overwriting module instance %s with new instance %s\n",
			   instances[moduletype]->getName(), instance->getName());
	}

	instances[moduletype] = instance;
}

Module *Module::getInstance(const std::string &name)
{
	ModuleRegistry &registry = registryInstance();

	auto it = registry.find(name);

	if (registry.end() == it)
		return nullptr;

	return it->second;
}

} // love
