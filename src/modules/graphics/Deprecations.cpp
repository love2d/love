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

#include "common/config.h"
#include "Deprecations.h"
#include "Graphics.h"
#include "Font.h"
#include "common/deprecation.h"
#include "timer/Timer.h"

#include <algorithm>

namespace love
{
namespace graphics
{

Deprecations::Deprecations()
	: currentDeprecationCount(0)
	, lastUpdatedTime(0.0)
{
}

Deprecations::~Deprecations()
{
}

void Deprecations::draw(Graphics *gfx)
{
	if (!isDeprecationOutputEnabled())
		return;

	GetDeprecated deprecations;

	if (deprecations.all.empty())
		return;

	int total = (int) deprecations.all.size();

	if (total != currentDeprecationCount)
	{
		currentDeprecationCount = total;
		lastUpdatedTime = timer::Timer::getTime();
	}

	double showTime = 20.0;
	double fadeTime = 1.0;

	double delta = timer::Timer::getTime() - lastUpdatedTime;

	float alpha = 1.0f;
	if (delta > (showTime - fadeTime))
		alpha = (float) (1.0 - (delta - (showTime - fadeTime)) / fadeTime);

	if (alpha <= 0.0f)
		return;

	if (font.get() == nullptr)
	{
		auto hinting = font::TrueTypeRasterizer::HINTING_NORMAL;

		if (!isGammaCorrect() && gfx->getScreenDPIScale() <= 1.0)
			hinting = font::TrueTypeRasterizer::HINTING_LIGHT;

		font.set(gfx->newDefaultFont(9, hinting), Acquire::NORETAIN);
	}

	gfx->flushStreamDraws();

	gfx->push(Graphics::STACK_ALL);
	gfx->reset();

	int maxcount = 4;
	int remaining = std::max(0, total - maxcount);

	std::vector<Font::ColoredString> strings;
	Colorf white(1, 1, 1, 1);

	// Grab the newest deprecation notices first.
	for (int i = total - 1; i >= remaining; --i)
	{
		if (!strings.empty())
			strings.back().str += "\n";

		const DeprecationInfo *info = deprecations.all[i];
		strings.push_back({getDeprecationNotice(*info, true), white});
	}

	if (remaining > 0)
		strings.push_back({"\n(And " + std::to_string(remaining) + " more)", white});

	int padding = 5;
	int width = 600;

	for (const auto &coloredstr : strings)
		width = std::max(width, font->getWidth(coloredstr.str) + padding * 2);

	float wraplimit = std::min(gfx->getWidth(), width - padding * 2);

	std::vector<std::string> wrappedlines;
	font->getWrap(strings, wraplimit, wrappedlines);

	int linecount = std::min((int) wrappedlines.size(), maxcount);
	int height = font->getHeight() * linecount + padding * 2;

	int x = 0;
	int y = std::max(gfx->getHeight() - height, 0);

	gfx->setColor(Colorf(0, 0, 0, 0.85 * alpha));
	gfx->rectangle(Graphics::DRAW_FILL, x, y, width, height);

	gfx->setColor(Colorf(1, 0.9, 0.8, 1 * alpha));
	gfx->setScissor({x, y, width, height});

	Matrix4 textm(x + padding, y + padding, 0, 1, 1, 0, 0, 0, 0);
	gfx->printf(strings, font.get(), wraplimit, Font::ALIGN_LEFT, textm);

	gfx->pop();
}

} // graphics
} // love
