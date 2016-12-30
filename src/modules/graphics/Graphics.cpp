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
#include "Graphics.h"
#include "math/MathModule.h"
#include "Polyline.h"

// C++
#include <algorithm>

namespace love
{
namespace graphics
{

static bool gammaCorrect = false;

void setGammaCorrect(bool gammacorrect)
{
	gammaCorrect = gammacorrect;
}

bool isGammaCorrect()
{
	return gammaCorrect;
}

void gammaCorrectColor(Colorf &c)
{
	if (isGammaCorrect())
	{
		c.r = math::gammaToLinear(c.r);
		c.g = math::gammaToLinear(c.g);
		c.b = math::gammaToLinear(c.b);
	}
}

Colorf gammaCorrectColor(const Colorf &c)
{
	Colorf r = c;
	gammaCorrectColor(r);
	return r;
}

void unGammaCorrectColor(Colorf &c)
{
	if (isGammaCorrect())
	{
		c.r = math::linearToGamma(c.r);
		c.g = math::linearToGamma(c.g);
		c.b = math::linearToGamma(c.b);
	}
}

Colorf unGammaCorrectColor(const Colorf &c)
{
	Colorf r = c;
	unGammaCorrectColor(r);
	return r;
}

love::Type Graphics::type("graphics", &Module::type);

Graphics::Graphics()
	: streamBufferState()
	, projectionMatrix()
{
	transformStack.reserve(16);
	transformStack.push_back(Matrix4());
}

Graphics::~Graphics()
{
	delete streamBufferState.vb[0];
	delete streamBufferState.vb[1];
	delete streamBufferState.indexBuffer;
}

Graphics::StreamVertexData Graphics::requestStreamDraw(const StreamDrawRequest &req)
{
	using namespace vertex;

	StreamBufferState &state = streamBufferState;

	bool shouldflush = false;
	bool shouldresize = false;

	if (req.primitiveMode != state.primitiveMode
		|| req.formats[0] != state.formats[0] || req.formats[1] != state.formats[1]
		|| ((req.indexMode != TriangleIndexMode::NONE) != (state.indexCount > 0))
		|| req.texture != state.texture || req.textureHandle != state.textureHandle)
	{
		shouldflush = true;
	}

	int totalvertices = state.vertexCount + req.vertexCount;

	if (totalvertices > LOVE_UINT16_MAX && req.indexMode != TriangleIndexMode::NONE)
		shouldflush = true;

	int reqIndexCount = getIndexCount(req.indexMode, req.vertexCount);
	size_t reqIndexSize = reqIndexCount * sizeof(uint16);

	size_t newdatasizes[2] = {0, 0};
	size_t buffersizes[3] = {0, 0, 0};

	for (int i = 0; i < 2; i++)
	{
		if (req.formats[i] != CommonFormat::NONE)
		{
			size_t stride = getFormatStride(req.formats[i]);
			size_t datasize = stride * totalvertices;

			size_t cursize = state.vb[i]->getSize();

			if (datasize > cursize)
			{
				shouldflush = true;

				if (stride * req.vertexCount > cursize)
				{
					buffersizes[i] = std::max(datasize, cursize * 2);
					shouldresize = true;
				}
			}

			newdatasizes[i] = stride * req.vertexCount;
		}
	}

	{
		size_t datasize = (state.indexCount + reqIndexCount) * sizeof(uint16);
		size_t cursize = state.indexBuffer->getSize();

		if (datasize > cursize)
		{
			shouldflush = true;

			if (reqIndexSize > cursize)
			{
				buffersizes[2] = std::max(datasize, cursize * 2);
				shouldresize = true;
			}
		}
	}

	if (shouldflush)
	{
		flushStreamDraws();

		state.primitiveMode = req.primitiveMode;
		state.formats[0] = req.formats[0];
		state.formats[1] = req.formats[1];
		state.texture = req.texture;
		state.textureHandle = req.textureHandle;
	}

	if (shouldresize)
	{
		for (int i = 0; i < 2; i++)
		{
			if (state.vb[i]->getSize() < buffersizes[i])
				state.vb[i]->setSize(buffersizes[i]);
		}

		if (state.indexBuffer->getSize() < buffersizes[2])
			state.indexBuffer->setSize(buffersizes[2]);
	}

	if (req.indexMode != TriangleIndexMode::NONE)
	{
		uint16 *indices = (uint16 *) state.indexBuffer->getOffsetData();
		fillIndices(req.indexMode, state.vertexCount, req.vertexCount, indices);
		state.indexBuffer->incrementOffset(reqIndexSize);
	}

	StreamVertexData d;
	d.stream[0] = state.vb[0]->getOffsetData();
	d.stream[1] = state.vb[1]->getOffsetData();

	state.vertexCount += req.vertexCount;
	state.indexCount  += reqIndexCount;

	state.vb[0]->incrementOffset(newdatasizes[0]);
	state.vb[1]->incrementOffset(newdatasizes[1]);

	return d;
}

void Graphics::draw(Drawable *drawable, const Matrix4 &m)
{
	drawable->draw(this, m);
}

void Graphics::draw(Texture *texture, Quad *quad, const Matrix4 &m)
{
	texture->drawq(this, quad, m);
}

/**
 * Primitives
 **/

void Graphics::points(const float *coords, const Colorf *colors, size_t numpoints)
{
	StreamDrawRequest req;
	req.primitiveMode = vertex::PrimitiveMode::POINTS;
	req.formats[0] = vertex::CommonFormat::XYf;
	if (colors)
		req.formats[1] = vertex::CommonFormat::RGBAub;
	req.vertexCount = (int) numpoints;

	StreamVertexData data = requestStreamDraw(req);

	const Matrix4 &t = getTransform();
	t.transform((Vector *) data.stream[0], (const Vector *) coords, req.vertexCount);

	Color *colordata = (Color *) data.stream[1];

	if (colors)
	{
		Colorf nc = getColor();
		gammaCorrectColor(nc);

		if (isGammaCorrect())
		{
			for (int i = 0; i < req.vertexCount; i++)
			{
				Colorf ci = colors[i];
				gammaCorrectColor(ci);
				ci *= nc;
				unGammaCorrectColor(ci);
				colordata[i] = toColor(ci);
			}
		}
		else
		{
			for (int i = 0; i < req.vertexCount; i++)
				colordata[i] = toColor(nc * colors[i]);
		}
	}
	else
	{
		Color c = toColor(getColor());

		for (int i = 0; i < req.vertexCount; i++)
			colordata[i] = c;
	}
}

int Graphics::calculateEllipsePoints(float rx, float ry) const
{
	int points = (int) sqrtf(((rx + ry) / 2.0f) * 20.0f * (float) pixelScaleStack.back());
	return std::max(points, 8);
}

void Graphics::polyline(const float *coords, size_t count)
{
	float halfwidth = getLineWidth() * 0.5f;
	LineJoin linejoin = getLineJoin();
	LineStyle linestyle = getLineStyle();

	float pixelsize = 1.0f / std::max((float) pixelScaleStack.back(), 0.000001f);

	if (linejoin == LINE_JOIN_NONE)
	{
		NoneJoinPolyline line;
		line.render(coords, count, halfwidth, pixelsize, linestyle == LINE_SMOOTH);
		line.draw(this);
	}
	else if (linejoin == LINE_JOIN_BEVEL)
	{
		BevelJoinPolyline line;
		line.render(coords, count, halfwidth, pixelsize, linestyle == LINE_SMOOTH);
		line.draw(this);
	}
	else if (linejoin == LINE_JOIN_MITER)
	{
		MiterJoinPolyline line;
		line.render(coords, count, halfwidth, pixelsize, linestyle == LINE_SMOOTH);
		line.draw(this);
	}
}

void Graphics::rectangle(DrawMode mode, float x, float y, float w, float h)
{
	float coords[] = {x,y, x,y+h, x+w,y+h, x+w,y, x,y};
	polygon(mode, coords, 5 * 2);
}

void Graphics::rectangle(DrawMode mode, float x, float y, float w, float h, float rx, float ry, int points)
{
	if (rx == 0 || ry == 0)
	{
		rectangle(mode, x, y, w, h);
		return;
	}

	// Radius values that are more than half the rectangle's size aren't handled
	// correctly (for now)...
	if (w >= 0.02f)
		rx = std::min(rx, w / 2.0f - 0.01f);
	if (h >= 0.02f)
		ry = std::min(ry, h / 2.0f - 0.01f);

	points = std::max(points / 4, 1);

	const float half_pi = static_cast<float>(LOVE_M_PI / 2);
	float angle_shift = half_pi / ((float) points + 1.0f);

	int num_coords = (points + 2) * 8;
	float *coords = getScratchBuffer<float>(num_coords + 2);
	float phi = .0f;

	for (int i = 0; i <= points + 2; ++i, phi += angle_shift)
	{
		coords[2 * i + 0] = x + rx * (1 - cosf(phi));
		coords[2 * i + 1] = y + ry * (1 - sinf(phi));
	}

	phi = half_pi;

	for (int i = points + 2; i <= 2 * (points + 2); ++i, phi += angle_shift)
	{
		coords[2 * i + 0] = x + w - rx * (1 + cosf(phi));
		coords[2 * i + 1] = y + ry * (1 - sinf(phi));
	}

	phi = 2 * half_pi;

	for (int i = 2 * (points + 2); i <= 3 * (points + 2); ++i, phi += angle_shift)
	{
		coords[2 * i + 0] = x + w - rx * (1 + cosf(phi));
		coords[2 * i + 1] = y + h - ry * (1 + sinf(phi));
	}

	phi =  3 * half_pi;

	for (int i = 3 * (points + 2); i <= 4 * (points + 2); ++i, phi += angle_shift)
	{
		coords[2 * i + 0] = x + rx * (1 - cosf(phi));
		coords[2 * i + 1] = y + h - ry * (1 + sinf(phi));
	}

	coords[num_coords + 0] = coords[0];
	coords[num_coords + 1] = coords[1];

	polygon(mode, coords, num_coords + 2);
}

void Graphics::rectangle(DrawMode mode, float x, float y, float w, float h, float rx, float ry)
{
	rectangle(mode, x, y, w, h, rx, ry, calculateEllipsePoints(rx, ry));
}

void Graphics::circle(DrawMode mode, float x, float y, float radius, int points)
{
	ellipse(mode, x, y, radius, radius, points);
}

void Graphics::circle(DrawMode mode, float x, float y, float radius)
{
	ellipse(mode, x, y, radius, radius);
}

void Graphics::ellipse(DrawMode mode, float x, float y, float a, float b, int points)
{
	float two_pi = (float) (LOVE_M_PI * 2);
	if (points <= 0) points = 1;
	float angle_shift = (two_pi / points);
	float phi = .0f;

	float *coords = getScratchBuffer<float>(2 * (points + 1));
	for (int i = 0; i < points; ++i, phi += angle_shift)
	{
		coords[2*i+0] = x + a * cosf(phi);
		coords[2*i+1] = y + b * sinf(phi);
	}

	coords[2*points+0] = coords[0];
	coords[2*points+1] = coords[1];

	polygon(mode, coords, (points + 1) * 2);
}

void Graphics::ellipse(DrawMode mode, float x, float y, float a, float b)
{
	ellipse(mode, x, y, a, b, calculateEllipsePoints(a, b));
}

void Graphics::arc(DrawMode drawmode, ArcMode arcmode, float x, float y, float radius, float angle1, float angle2, int points)
{
	// Nothing to display with no points or equal angles. (Or is there with line mode?)
	if (points <= 0 || angle1 == angle2)
		return;

	// Oh, you want to draw a circle?
	if (fabs(angle1 - angle2) >= 2.0f * (float) LOVE_M_PI)
	{
		circle(drawmode, x, y, radius, points);
		return;
	}

	float angle_shift = (angle2 - angle1) / points;
	// Bail on precision issues.
	if (angle_shift == 0.0)
		return;

	// Prevent the connecting line from being drawn if a closed line arc has a
	// small angle. Avoids some visual issues when connected lines are at sharp
	// angles, due to the miter line join drawing code.
	if (drawmode == DRAW_LINE && arcmode == ARC_CLOSED && fabsf(angle1 - angle2) < LOVE_TORAD(4))
		arcmode = ARC_OPEN;

	// Quick fix for the last part of a filled open arc not being drawn (because
	// polygon(DRAW_FILL, ...) doesn't work without a closed loop of vertices.)
	if (drawmode == DRAW_FILL && arcmode == ARC_OPEN)
		arcmode = ARC_CLOSED;

	float phi = angle1;

	float *coords = nullptr;
	int num_coords = 0;

	const auto createPoints = [&](float *coordinates)
	{
		for (int i = 0; i <= points; ++i, phi += angle_shift)
		{
			coordinates[2 * i + 0] = x + radius * cosf(phi);
			coordinates[2 * i + 1] = y + radius * sinf(phi);
		}
	};

	if (arcmode == ARC_PIE)
	{
		num_coords = (points + 3) * 2;
		coords = getScratchBuffer<float>(num_coords);

		coords[0] = coords[num_coords - 2] = x;
		coords[1] = coords[num_coords - 1] = y;

		createPoints(coords + 2);
	}
	else if (arcmode == ARC_OPEN)
	{
		num_coords = (points + 1) * 2;
		coords = getScratchBuffer<float>(num_coords);

		createPoints(coords);
	}
	else // ARC_CLOSED
	{
		num_coords = (points + 2) * 2;
		coords = getScratchBuffer<float>(num_coords);

		createPoints(coords);

		// Connect the ends of the arc.
		coords[num_coords - 2] = coords[0];
		coords[num_coords - 1] = coords[1];
	}

	polygon(drawmode, coords, num_coords);
}

void Graphics::arc(DrawMode drawmode, ArcMode arcmode, float x, float y, float radius, float angle1, float angle2)
{
	float points = (float) calculateEllipsePoints(radius, radius);

	// The amount of points is based on the fraction of the circle created by the arc.
	float angle = fabsf(angle1 - angle2);
	if (angle < 2.0f * (float) LOVE_M_PI)
		points *= angle / (2.0f * (float) LOVE_M_PI);

	arc(drawmode, arcmode, x, y, radius, angle1, angle2, (int) (points + 0.5f));
}

/// @param mode    the draw mode
/// @param coords  the coordinate array
/// @param count   the number of coordinates/size of the array
void Graphics::polygon(DrawMode mode, const float *coords, size_t count)
{
	// coords is an array of a closed loop of vertices, i.e.
	// coords[count-2] = coords[0], coords[count-1] = coords[1]
	if (mode == DRAW_LINE)
	{
		polyline(coords, count);
	}
	else
	{
		StreamDrawRequest req;
		req.formats[0] = vertex::CommonFormat::XYf;
		req.formats[1] = vertex::CommonFormat::RGBAub;
		req.indexMode = vertex::TriangleIndexMode::FAN;
		req.vertexCount = (int)count/2 - 1;

		StreamVertexData data = requestStreamDraw(req);
		
		const Matrix4 &t = getTransform();
		t.transform((Vector *) data.stream[0], (const Vector *) coords, req.vertexCount);
		
		Color c = toColor(getColor());
		Color *colordata = (Color *) data.stream[1];
		for (int i = 0; i < req.vertexCount; i++)
			colordata[i] = c;
	}
}

const Matrix4 &Graphics::getTransform() const
{
	return transformStack.back();
}

const Matrix4 &Graphics::getProjection() const
{
	return projectionMatrix;
}

void Graphics::pushTransform()
{
	transformStack.push_back(transformStack.back());
}

void Graphics::pushIdentityTransform()
{
	transformStack.push_back(Matrix4());
}

void Graphics::popTransform()
{
	transformStack.pop_back();
}

void Graphics::rotate(float r)
{
	transformStack.back().rotate(r);
}

void Graphics::scale(float x, float y)
{
	transformStack.back().scale(x, y);
	pixelScaleStack.back() *= (fabs(x) + fabs(y)) / 2.0;
}

void Graphics::translate(float x, float y)
{
	transformStack.back().translate(x, y);
}

void Graphics::shear(float kx, float ky)
{
	transformStack.back().shear(kx, ky);
}

void Graphics::origin()
{
	transformStack.back().setIdentity();
	pixelScaleStack.back() = 1;
}

void Graphics::applyTransform(love::math::Transform *transform)
{
	Matrix4 &m = transformStack.back();
	m *= transform->getMatrix();

	float sx, sy;
	m.getApproximateScale(sx, sy);
	pixelScaleStack.back() = (sx + sy) / 2.0;
}

void Graphics::replaceTransform(love::math::Transform *transform)
{
	const Matrix4 &m = transform->getMatrix();
	transformStack.back() = m;

	float sx, sy;
	m.getApproximateScale(sx, sy);
	pixelScaleStack.back() = (sx + sy) / 2.0;
}

Vector Graphics::transformPoint(Vector point)
{
	Vector p;
	transformStack.back().transform(&p, &point, 1);
	return p;
}

Vector Graphics::inverseTransformPoint(Vector point)
{
	Vector p;
	// TODO: We should probably cache the inverse transform so we don't have to
	// re-calculate it every time this is called.
	transformStack.back().inverse().transform(&p, &point, 1);
	return p;
}

bool Graphics::getConstant(const char *in, DrawMode &out)
{
	return drawModes.find(in, out);
}

bool Graphics::getConstant(DrawMode in, const char *&out)
{
	return drawModes.find(in, out);
}

bool Graphics::getConstant(const char *in, ArcMode &out)
{
	return arcModes.find(in, out);
}

bool Graphics::getConstant(ArcMode in, const char *&out)
{
	return arcModes.find(in, out);
}

bool Graphics::getConstant(const char *in, BlendMode &out)
{
	return blendModes.find(in, out);
}

bool Graphics::getConstant(BlendMode in, const char *&out)
{
	return blendModes.find(in, out);
}

bool Graphics::getConstant(const char *in, BlendAlpha &out)
{
	return blendAlphaModes.find(in, out);
}

bool Graphics::getConstant(BlendAlpha in, const char *&out)
{
	return blendAlphaModes.find(in, out);
}

bool Graphics::getConstant(const char *in, LineStyle &out)
{
	return lineStyles.find(in, out);
}

bool Graphics::getConstant(LineStyle in, const char *&out)
{
	return lineStyles.find(in, out);
}

bool Graphics::getConstant(const char *in, LineJoin &out)
{
	return lineJoins.find(in, out);
}

bool Graphics::getConstant(LineJoin in, const char *&out)
{
	return lineJoins.find(in, out);
}

bool Graphics::getConstant(const char *in, StencilAction &out)
{
	return stencilActions.find(in, out);
}

bool Graphics::getConstant(StencilAction in, const char *&out)
{
	return stencilActions.find(in, out);
}

bool Graphics::getConstant(const char *in, CompareMode &out)
{
	return compareModes.find(in, out);
}

bool Graphics::getConstant(CompareMode in, const char *&out)
{
	return compareModes.find(in, out);
}

bool Graphics::getConstant(const char *in, Feature &out)
{
	return features.find(in, out);
}

bool Graphics::getConstant(Feature in, const char *&out)
{
	return features.find(in, out);
}

bool Graphics::getConstant(const char *in, SystemLimit &out)
{
	return systemLimits.find(in, out);
}

bool Graphics::getConstant(SystemLimit in, const char *&out)
{
	return systemLimits.find(in, out);
}

bool Graphics::getConstant(const char *in, StackType &out)
{
	return stackTypes.find(in, out);
}

bool Graphics::getConstant(StackType in, const char *&out)
{
	return stackTypes.find(in, out);
}

StringMap<Graphics::DrawMode, Graphics::DRAW_MAX_ENUM>::Entry Graphics::drawModeEntries[] =
{
	{ "line", DRAW_LINE },
	{ "fill", DRAW_FILL },
};

StringMap<Graphics::DrawMode, Graphics::DRAW_MAX_ENUM> Graphics::drawModes(Graphics::drawModeEntries, sizeof(Graphics::drawModeEntries));

StringMap<Graphics::ArcMode, Graphics::ARC_MAX_ENUM>::Entry Graphics::arcModeEntries[] =
{
	{ "open",   ARC_OPEN   },
	{ "closed", ARC_CLOSED },
	{ "pie",    ARC_PIE    },
};

StringMap<Graphics::ArcMode, Graphics::ARC_MAX_ENUM> Graphics::arcModes(Graphics::arcModeEntries, sizeof(Graphics::arcModeEntries));

StringMap<Graphics::BlendMode, Graphics::BLEND_MAX_ENUM>::Entry Graphics::blendModeEntries[] =
{
	{ "alpha",    BLEND_ALPHA    },
	{ "add",      BLEND_ADD      },
	{ "subtract", BLEND_SUBTRACT },
	{ "multiply", BLEND_MULTIPLY },
	{ "lighten",  BLEND_LIGHTEN  },
	{ "darken",   BLEND_DARKEN   },
	{ "screen",   BLEND_SCREEN   },
	{ "replace",  BLEND_REPLACE  },
	{ "none",     BLEND_NONE     },
};

StringMap<Graphics::BlendMode, Graphics::BLEND_MAX_ENUM> Graphics::blendModes(Graphics::blendModeEntries, sizeof(Graphics::blendModeEntries));

StringMap<Graphics::BlendAlpha, Graphics::BLENDALPHA_MAX_ENUM>::Entry Graphics::blendAlphaEntries[] =
{
	{ "alphamultiply", BLENDALPHA_MULTIPLY      },
	{ "premultiplied", BLENDALPHA_PREMULTIPLIED },
};

StringMap<Graphics::BlendAlpha, Graphics::BLENDALPHA_MAX_ENUM> Graphics::blendAlphaModes(Graphics::blendAlphaEntries, sizeof(Graphics::blendAlphaEntries));

StringMap<Graphics::LineStyle, Graphics::LINE_MAX_ENUM>::Entry Graphics::lineStyleEntries[] =
{
	{ "smooth", LINE_SMOOTH },
	{ "rough",  LINE_ROUGH  }
};

StringMap<Graphics::LineStyle, Graphics::LINE_MAX_ENUM> Graphics::lineStyles(Graphics::lineStyleEntries, sizeof(Graphics::lineStyleEntries));

StringMap<Graphics::LineJoin, Graphics::LINE_JOIN_MAX_ENUM>::Entry Graphics::lineJoinEntries[] =
{
	{ "none",  LINE_JOIN_NONE  },
	{ "miter", LINE_JOIN_MITER },
	{ "bevel", LINE_JOIN_BEVEL }
};

StringMap<Graphics::LineJoin, Graphics::LINE_JOIN_MAX_ENUM> Graphics::lineJoins(Graphics::lineJoinEntries, sizeof(Graphics::lineJoinEntries));

StringMap<Graphics::StencilAction, Graphics::STENCIL_MAX_ENUM>::Entry Graphics::stencilActionEntries[] =
{
	{ "replace",       STENCIL_REPLACE        },
	{ "increment",     STENCIL_INCREMENT      },
	{ "decrement",     STENCIL_DECREMENT      },
	{ "incrementwrap", STENCIL_INCREMENT_WRAP },
	{ "decrementwrap", STENCIL_DECREMENT_WRAP },
	{ "invert",        STENCIL_INVERT         },
};

StringMap<Graphics::StencilAction, Graphics::STENCIL_MAX_ENUM> Graphics::stencilActions(Graphics::stencilActionEntries, sizeof(Graphics::stencilActionEntries));

StringMap<Graphics::CompareMode, Graphics::COMPARE_MAX_ENUM>::Entry Graphics::compareModeEntries[] =
{
	{ "less",     COMPARE_LESS     },
	{ "lequal",   COMPARE_LEQUAL   },
	{ "equal",    COMPARE_EQUAL    },
	{ "gequal",   COMPARE_GEQUAL   },
	{ "greater",  COMPARE_GREATER  },
	{ "notequal", COMPARE_NOTEQUAL },
	{ "always",   COMPARE_ALWAYS   },
};

StringMap<Graphics::CompareMode, Graphics::COMPARE_MAX_ENUM> Graphics::compareModes(Graphics::compareModeEntries, sizeof(Graphics::compareModeEntries));

StringMap<Graphics::Feature, Graphics::FEATURE_MAX_ENUM>::Entry Graphics::featureEntries[] =
{
	{ "multicanvasformats", FEATURE_MULTI_CANVAS_FORMATS },
	{ "clampzero",          FEATURE_CLAMP_ZERO           },
	{ "lighten",            FEATURE_LIGHTEN              },
	{ "fullnpot", FEATURE_FULL_NPOT },
	{ "pixelshaderhighp", FEATURE_PIXEL_SHADER_HIGHP },
};

StringMap<Graphics::Feature, Graphics::FEATURE_MAX_ENUM> Graphics::features(Graphics::featureEntries, sizeof(Graphics::featureEntries));

StringMap<Graphics::SystemLimit, Graphics::LIMIT_MAX_ENUM>::Entry Graphics::systemLimitEntries[] =
{
	{ "pointsize",   LIMIT_POINT_SIZE   },
	{ "texturesize", LIMIT_TEXTURE_SIZE },
	{ "multicanvas", LIMIT_MULTI_CANVAS },
	{ "canvasmsaa",  LIMIT_CANVAS_MSAA  },
	{ "anisotropy",  LIMIT_ANISOTROPY   },
};

StringMap<Graphics::SystemLimit, Graphics::LIMIT_MAX_ENUM> Graphics::systemLimits(Graphics::systemLimitEntries, sizeof(Graphics::systemLimitEntries));

StringMap<Graphics::StackType, Graphics::STACK_MAX_ENUM>::Entry Graphics::stackTypeEntries[] =
{
	{ "all",       STACK_ALL       },
	{ "transform", STACK_TRANSFORM },
};

StringMap<Graphics::StackType, Graphics::STACK_MAX_ENUM> Graphics::stackTypes(Graphics::stackTypeEntries, sizeof(Graphics::stackTypeEntries));

} // graphics
} // love
