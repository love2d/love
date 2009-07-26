#include "GrahamScanConvexHull.h"

#include <cmath>

bool GrahamScanConvexHull::operator()(const std::vector < point2d >& pnt, std::vector< point2d >& final_hull)
{
   final_hull.clear();

   if (pnt.size() <= 3)
   {
      std::copy(pnt.begin(), pnt.end(), std::back_inserter(final_hull));
      return true;
   }

   unsigned int j = 0;
   gs_point2d   tmp_pnt;

   for(unsigned int i = 0; i < pnt.size(); i++)
   {
      point.push_back(gs_point2d(pnt[i].x,pnt[i].y,0.0));

      if (point[i].y < point[j].y)
        j = i;
      else if (point[i].y == point[j].y)
        if (point[i].x < point[j].x)
          j = i;
   }

   tmp_pnt  = point[0];
   point[0] = point[j];
   point[j] = tmp_pnt;

   anchor = point[0];

   for (unsigned int i = 1; i < point.size(); i++)
   {
      point[i].angle = cartesian_angle(point[i].x - anchor.x, point[i].y - anchor.y);
   }

   sort(++point.begin(),point.end(),GSPoint2DCompare(&anchor));

   graham_scan(final_hull);

   return true;
}


void GrahamScanConvexHull::graham_scan(std::vector< point2d >& final_hull)
{
   const std::size_t HEAD     = 0;
   const std::size_t PRE_HEAD = 1;

   std::deque<gs_point2d> pnt_queue;

   pnt_queue.push_front(point[0]);
   pnt_queue.push_front(point[1]);

   unsigned int i = 2;

   while(i < point.size())
   {
      if (pnt_queue.size() > 1)
      {
         if (orientation(pnt_queue[PRE_HEAD],pnt_queue[HEAD],point[i]) == counter_clock_wise)
            pnt_queue.push_front(point[i++]);
         else
          pnt_queue.pop_front();
      }
      else
         pnt_queue.push_front(point[i++]);
   }

   for(std::deque<gs_point2d>::iterator it = pnt_queue.begin(); it != pnt_queue.end(); it++)
   {
      final_hull.push_back(point2d((*it).x, (*it).y));
   }
}


inline double GrahamScanConvexHull::cartesian_angle(double x, double y)
{
  if      ((x >  0.0) && (y >  0.0)) return (atan( y / x) * _180DivPI);
  else if ((x <  0.0) && (y >  0.0)) return (atan(-x / y) * _180DivPI) +  90.0;
  else if ((x <  0.0) && (y <  0.0)) return (atan( y / x) * _180DivPI) + 180.0;
  else if ((x >  0.0) && (y <  0.0)) return (atan(-x / y) * _180DivPI) + 270.0;
  else if ((x == 0.0) && (y >  0.0)) return  90.0;
  else if ((x <  0.0) && (y == 0.0)) return 180.0;
  else if ((x == 0.0) && (y <  0.0)) return 270.0;
  else
    return 0.0;
}


inline int GrahamScanConvexHull::orientation(const gs_point2d& p1, const gs_point2d& p2, const gs_point2d& p3)
{
   return orientation(p1.x,p1.y,p2.x,p2.y,p3.x,p3.y);
}


inline int GrahamScanConvexHull::orientation(const double x1, const double y1,
                                             const double x2, const double y2,
                                             const double px, const double py)
{
   double orin = (x2 - x1) * (py - y1) - (px - x1) * (y2 - y1);

   if (is_equal(orin,0.0))
     return 0;              /* Orientaion is neutral aka collinear  */
   else if (orin < 0.0)
     return -1;             /* Orientaion is to the right-hand side */
   else
     return +1;             /* Orientaion is to the left-hand side  */

}


inline bool GrahamScanConvexHull::is_equal(const double v1, const double& v2, const double epsilon)
{
   double diff = v1 - v2;
   return  (-epsilon <= diff) && (diff <= epsilon);
}


