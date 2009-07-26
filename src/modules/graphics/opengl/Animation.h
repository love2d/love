/**
* Copyright (c) 2006-2009 LOVE Development Team
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

#ifndef LOVE_OPENGL_ANIMATION_H
#define LOVE_OPENGL_ANIMATION_H

// LOVE
#include "../Drawable.h"
#include "Image.h"

// STD
#include <vector>

namespace love
{
namespace graphics
{
namespace opengl
{

	// Represents a single frame.
	struct AnimationFrame
	{
		float x, y;    // Top left corner of the frame.
		float w, h;    // Size of the frame.
		int preDelay;  // Delay to previous frame.
		int postDelay; // Delay to next frame.
	};

	class Animation : public Drawable
	{
	private:

		// The source of the animation.
		Image * image;

		// Delays between frames.
		// delays[0] is the delay between frames[0] and frames[1].
		std::vector<float> delays;

		// Holds all the frames.
		std::vector<AnimationFrame> frames;

		// Animation mode.
		int mode;

		// The current frame.
		int current;
	
		// True if playing, false otherwise.
		bool playing;

		// "Left over"-time.
		float timeBuffer;

		// Used for bounce mode.
		int direction;

		// Overall speed. (1 = normal).
		float speed;

	public:

		/**
		* Creates an Animation with no frames.
		* @param image The image to use as the source.
		**/
		Animation(Image * image);

		/**
		* Creates an Animation with frames from top-left to bottom right.
		* @param image The image to use as the source.
		* @param fw The width of each frame.
		* @param fh The height of each frame.
		* @param delay The delay after each frame.
		* @param num The number of frames. (0 = all)
		**/
		Animation(Image * image, float fw, float fh, float delay, int num = 0);

		virtual ~Animation();

		/**
		* Adds a single frame.
		* @param x The top-left corner of the frame.
		* @param y The top-right corner of the frame.
		* @param w The width of the frame.
		* @param h The height of the frame.
		* @param delay The delay after the frame.
		**/
		void addFrame(float x, float y, float w, float h, float delay);

		/**
		* Sets the current animation mode, and reset.
		**/
		void setMode(int mode);

		/**
		* Causes the Animation to start playing.
		**/
		void play();

		/**
		* Causes the Animation to stop.
		**/
		void stop();

		/**
		* Resets the Animation.
		**/
		void reset();

		/**
		* Resets timebuffers, and sets the current frame directly.
		**/
		void seek(int frame);

		/**
		* Gets the current frame.
		**/
		int getCurrentFrame() const;

		/**
		* Gets amount of frames.
		**/
		int getSize() const;

		/**
		* Sets the delay after a frame.
		**/
		void setDelay(int frame, float delay);

		/**
		* Sets the overall animation speed.
		**/
		void setSpeed(float speed);

		/**
		* Gets the overall animation speed.
		**/
		float getSpeed() const;

		void update(float dt);

		void draw(float x, float y, float angle, float sx, float sy, float ox, float oy) const;
	
		float getWidth() const;

		float getHeight() const;
	};

} // opengl
} // graphics
} // love

#endif // LOVE_OPENGL_ANIMATION_H
