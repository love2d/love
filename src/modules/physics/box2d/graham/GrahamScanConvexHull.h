/*
 **************************************************************************
 * Class: Graham Scan Convex Hull                                         *
 * By Arash Partow - 2001                                                 *
 * URL: http://www.partow.net                                             *
 *                                                                        *
 * Copyright Notice:                                                      *
 * Free use of this library is permitted under the guidelines and         *
 * in accordance with the most current version of the Common Public       *
 * License.                                                               *
 * http://www.opensource.org/licenses/cpl.php                             *
 *                                                                        *
 **************************************************************************
*/


#ifndef INCLUDE_GRAHAMSCANCONVEXHULL_H
#define INCLUDE_GRAHAMSCANCONVEXHULL_H

#include <iostream>
#include <deque>
#include <vector>
#include <algorithm>
#include <math.h>

#include <vector>


struct point2d
{
   point2d(double _x = 0.0 , double _y = 0.0) : x(_x), y(_y){}
   double x;
   double y;
};

class ConvexHull
{
   public:

     virtual ~ConvexHull(){};
     virtual bool operator()(const std::vector<point2d>& pnt, std::vector<point2d>& final_hull) = 0;

};





struct gs_point2d
{
public:
   gs_point2d(double _x = 0.0, double _y = 0.0, double _angle = 0.0) : x(_x), y(_y), angle(_angle){}
   double x;
   double y;
   double angle;
};

const double _180DivPI  = 57.295779513082320876798154814105000;
const int    counter_clock_wise = +1;
const int    clock_wise         = -1;


class GSPoint2DCompare
{
public:

   GSPoint2DCompare(gs_point2d* _anchor):anchor(_anchor){};

   bool operator()(const gs_point2d& p1, const gs_point2d& p2)
   {
      if (p1.angle < p2.angle)      return true;
      else if (p1.angle > p2.angle) return false;
      else if (is_equal(p1,p2))     return false;
      else if (lay_distance(anchor->x, anchor->y, p1.x, p1.y) < lay_distance(anchor->x, anchor->y, p2.x, p2.y))
         return true;
      else
         return false;
   }

private:

   inline bool is_equal(const gs_point2d p1, gs_point2d p2)
   {
      return  is_equal(p1.x,p2.x) && is_equal(p1.y,p2.y);
   }

   inline bool is_equal(const double v1, const double& v2, const double epsilon = 1.0e-12)
   {
      double diff = v1 - v2;
      return  (-epsilon <= diff) && (diff <= epsilon);
   }

   inline double lay_distance(const double& x1, const double& y1, const double& x2, const double& y2)
   {
      double dx = (x1 - x2);
      double dy = (y1 - y2);
      return (dx * dx + dy * dy);
   }

   gs_point2d* anchor;

};


class GrahamScanConvexHull : public ConvexHull
{
public:

   GrahamScanConvexHull(){};
  ~GrahamScanConvexHull(){};

   virtual bool operator()(const std::vector < point2d >& pnt, std::vector< point2d >& final_hull);

private:

   void graham_scan(std::vector< point2d >& final_hull);

   inline double cartesian_angle(double x, double y);

   inline int orientation(const gs_point2d& p1,
                          const gs_point2d& p2,
                          const gs_point2d& p3);

   inline int orientation(const double x1, const double y1,
                          const double x2, const double y2,
                          const double px, const double py);

   inline bool is_equal(const double v1, const double& v2, const double epsilon = 1.0e-12);

   std::vector<gs_point2d> point;
   gs_point2d              anchor;

};



#endif
