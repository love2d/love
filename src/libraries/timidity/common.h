#ifndef __TIMIDITY_COMMON_H
#define __TIMIDITY_COMMON_H

#include <utility>

namespace Timidity {

typedef struct {
  char *path;
  void *next;
} PathList;

extern FILE *open_file(const char *name);
extern void add_to_pathlist(const char *s);
extern void *safe_malloc(size_t count);
extern void free_pathlist(void);

template<class T>
inline
const T MIN(const T a, const T b)
{
	return a < b ? a : b;
}

template<class T>
inline
const T MAX(const T a, const T b)
{
	return a > b ? a : b;
}

template<class T>
inline
T clamp(const T in, const T min, const T max)
{
	return in <= min ? min : in >= max ? max : in;
}

#define MAKE_ID(a,b,c,d)	((uint32_t)((a)|((b)<<8)|((c)<<16)|((d)<<24)))

template <typename T, size_t N>
char ( &_ArraySizeHelper( T (&array)[N] ))[N];

#define countof( array ) (sizeof( _ArraySizeHelper( array ) ))

template<class T>
inline
void swapvalues (T &a, T &b)
{
	T temp = std::move(a); a = std::move(b); b = std::move(temp);
}

}
#endif
