#ifndef __TIMIDITY_COMMON_H
#define __TIMIDITY_COMMON_H

typedef struct {
  char *path;
  void *next;
} PathList;

extern FILE *open_file(const char *name);
extern void add_to_pathlist(const char *s);
extern void *safe_malloc(size_t count);
extern void free_pathlist(void);

#endif
