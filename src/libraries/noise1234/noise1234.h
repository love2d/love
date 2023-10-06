// Noise1234
// Author: Stefan Gustavson (stegu@itn.liu.se)
//
// This library is public domain software, released by the author
// into the public domain in February 2011. You may do anything
// you like with it. You may even remove all attributions,
// but of course I'd appreciate it if you kept my name somewhere.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.

// Modified by the LOVE Development Team to use double precision.

/** \file
		\brief Declares the Noise1234 class for producing Perlin noise.
		\author Stefan Gustavson (stegu@itn.liu.se)
*/

/*
 * This is a clean, fast, modern and free Perlin noise class in C++.
 * Being a stand-alone class with no external dependencies, it is
 * highly reusable without source code modifications.
 */

class Noise1234 {

  public:
    Noise1234() {}
    ~Noise1234() {}

/** 1D, 2D, 3D and 4D float Perlin noise, SL "noise()"
 */
    static double noise( double x );
    static double noise( double x, double y );
    static double noise( double x, double y, double z );
    static double noise( double x, double y, double z, double w );

/** 1D, 2D, 3D and 4D float Perlin periodic noise, SL "pnoise()"
 */
    static double pnoise( double x, int px );
    static double pnoise( double x, double y, int px, int py );
    static double pnoise( double x, double y, double z, int px, int py, int pz );
    static double pnoise( double x, double y, double z, double w,
                              int px, int py, int pz, int pw );

  private:
    static unsigned char perm[];
    static double  grad( int hash, double x );
    static double  grad( int hash, double x, double y );
    static double  grad( int hash, double x, double y , double z );
    static double  grad( int hash, double x, double y, double z, double t );

};
