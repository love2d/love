/**
 * Copyright (c) 2006-2014 LOVE Development Team
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

#include "Graphics.h"

namespace love
{
namespace graphics
{

Graphics::~Graphics()
{
}

bool Graphics::getConstant(const char *in, DrawMode &out)
{
	return drawModes.find(in, out);
}

bool Graphics::getConstant(DrawMode in, const char  *&out)
{
	return drawModes.find(in, out);
}

bool Graphics::getConstant(const char *in, AlignMode &out)
{
	return alignModes.find(in, out);
}

bool Graphics::getConstant(AlignMode in, const char  *&out)
{
	return alignModes.find(in, out);
}

bool Graphics::getConstant(const char *in, BlendMode &out)
{
	return blendModes.find(in, out);
}

bool Graphics::getConstant(BlendMode in, const char  *&out)
{
	return blendModes.find(in, out);
}

bool Graphics::getConstant(const char *in, LineStyle &out)
{
	return lineStyles.find(in, out);
}

bool Graphics::getConstant(LineStyle in, const char  *&out)
{
	return lineStyles.find(in, out);
}

bool Graphics::getConstant(const char *in, LineJoin &out)
{
	return lineJoins.find(in, out);
}

bool Graphics::getConstant(LineJoin in, const char  *&out)
{
	return lineJoins.find(in, out);
}

bool Graphics::getConstant(const char *in, Support &out)
{
	return support.find(in, out);
}

bool Graphics::getConstant(Support in, const char  *&out)
{
	return support.find(in, out);
}

bool Graphics::getConstant(const char *in, RendererInfo &out)
{
	return rendererInfo.find(in, out);
}

bool Graphics::getConstant(RendererInfo in, const char *&out)
{
	return rendererInfo.find(in, out);
}

bool Graphics::getConstant(const char *in, SystemLimit &out)
{
	return systemLimits.find(in, out);
}

bool Graphics::getConstant(SystemLimit in, const char *&out)
{
	return systemLimits.find(in, out);
}

StringMap<Graphics::DrawMode, Graphics::DRAW_MAX_ENUM>::Entry Graphics::drawModeEntries[] =
{
	{ "line", Graphics::DRAW_LINE },
	{ "fill", Graphics::DRAW_FILL },
};

StringMap<Graphics::DrawMode, Graphics::DRAW_MAX_ENUM> Graphics::drawModes(Graphics::drawModeEntries, sizeof(Graphics::drawModeEntries));

StringMap<Graphics::AlignMode, Graphics::ALIGN_MAX_ENUM>::Entry Graphics::alignModeEntries[] =
{
	{ "left", Graphics::ALIGN_LEFT },
	{ "right", Graphics::ALIGN_RIGHT },
	{ "center", Graphics::ALIGN_CENTER },
	{ "justify", Graphics::ALIGN_JUSTIFY },
};

StringMap<Graphics::AlignMode, Graphics::ALIGN_MAX_ENUM> Graphics::alignModes(Graphics::alignModeEntries, sizeof(Graphics::alignModeEntries));

StringMap<Graphics::BlendMode, Graphics::BLEND_MAX_ENUM>::Entry Graphics::blendModeEntries[] =
{
	{ "alpha", Graphics::BLEND_ALPHA },
	{ "add", Graphics::BLEND_ADD },
	{ "subtract", Graphics::BLEND_SUBTRACT },
	{ "multiply", Graphics::BLEND_MULTIPLY },
	{ "premultiplied", Graphics::BLEND_PREMULTIPLIED },
	{ "screen", Graphics::BLEND_SCREEN },
	{ "replace", Graphics::BLEND_REPLACE },
};

StringMap<Graphics::BlendMode, Graphics::BLEND_MAX_ENUM> Graphics::blendModes(Graphics::blendModeEntries, sizeof(Graphics::blendModeEntries));

StringMap<Graphics::LineStyle, Graphics::LINE_MAX_ENUM>::Entry Graphics::lineStyleEntries[] =
{
	{ "smooth", Graphics::LINE_SMOOTH },
	{ "rough", Graphics::LINE_ROUGH }
};

StringMap<Graphics::LineStyle, Graphics::LINE_MAX_ENUM> Graphics::lineStyles(Graphics::lineStyleEntries, sizeof(Graphics::lineStyleEntries));

StringMap<Graphics::LineJoin, Graphics::LINE_JOIN_MAX_ENUM>::Entry Graphics::lineJoinEntries[] =
{
	{ "none",  Graphics::LINE_JOIN_NONE  },
	{ "miter", Graphics::LINE_JOIN_MITER },
	{ "bevel", Graphics::LINE_JOIN_BEVEL }
};

StringMap<Graphics::LineJoin, Graphics::LINE_JOIN_MAX_ENUM> Graphics::lineJoins(Graphics::lineJoinEntries, sizeof(Graphics::lineJoinEntries));

StringMap<Graphics::Support, Graphics::SUPPORT_MAX_ENUM>::Entry Graphics::supportEntries[] =
{
	{ "hdrcanvas", Graphics::SUPPORT_HDR_CANVAS },
	{ "multicanvas", Graphics::SUPPORT_MULTI_CANVAS },
	{ "dxt", Graphics::SUPPORT_DXT },
	{ "bc5", Graphics::SUPPORT_BC5 },
	{ "instancing", Graphics::SUPPORT_INSTANCING },
	{ "srgb", Graphics::SUPPORT_SRGB },
};

StringMap<Graphics::Support, Graphics::SUPPORT_MAX_ENUM> Graphics::support(Graphics::supportEntries, sizeof(Graphics::supportEntries));

StringMap<Graphics::RendererInfo, Graphics::RENDERER_INFO_MAX_ENUM>::Entry Graphics::rendererInfoEntries[] =
{
	{ "name", Graphics::RENDERER_INFO_NAME },
	{ "version", Graphics::RENDERER_INFO_VERSION },
	{ "vendor", Graphics::RENDERER_INFO_VENDOR },
	{ "device", Graphics::RENDERER_INFO_DEVICE },
};

StringMap<Graphics::RendererInfo, Graphics::RENDERER_INFO_MAX_ENUM> Graphics::rendererInfo(Graphics::rendererInfoEntries, sizeof(Graphics::rendererInfoEntries));

StringMap<Graphics::SystemLimit, Graphics::LIMIT_MAX_ENUM>::Entry Graphics::systemLimitEntries[] =
{
	{"pointsize", Graphics::LIMIT_POINT_SIZE},
	{"texturesize", Graphics::LIMIT_TEXTURE_SIZE},
	{"multicanvas", Graphics::LIMIT_MULTI_CANVAS},
	{"canvasmsaa", Graphics::LIMIT_CANVAS_MSAA},
};

StringMap<Graphics::SystemLimit, Graphics::LIMIT_MAX_ENUM> Graphics::systemLimits(Graphics::systemLimitEntries, sizeof(Graphics::systemLimitEntries));

} // graphics
} // love
