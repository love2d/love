#ifndef WUFF_CONFIG_H
#define WUFF_CONFIG_H

/* Defines that the internal code is being built. */
/* The wuff.h header uses this to change export and import macros. */
#define WUFF_BUILDING_CORE

#ifndef WUFF_INLINE_OVERRIDE
	#ifdef __cplusplus
		#define WUFF_INLINE inline
	#else
		#ifdef _MSC_VER
			#define WUFF_INLINE __inline
		#elif __GNUC__
			#define WUFF_INLINE __inline__
		#else
			#define WUFF_INLINE
		#endif
	#endif
#endif


#ifndef WUFF_GCC_VISIBILITY_OVERRIDE
	#if __GNUC__ >= 4
		#define WUFF_INTERN_API __attribute__((visibility("hidden")))
	#else
		#define WUFF_INTERN_API
    #endif
#endif


#ifdef WUFF_MEMALLOC_OVERRIDE
	#ifdef __cplusplus
extern "C" {
	#endif

	/* Define your own memory allocator. */
	void * wuff_alloc(size_t size);
	void wuff_free(void * mem);

	#ifdef __cplusplus
}
	#endif
#else
WUFF_INTERN_API void * wuff_alloc(size_t size);
WUFF_INTERN_API void wuff_free(void * mem);
#endif


#endif /* WUFF_CONFIG_H */
