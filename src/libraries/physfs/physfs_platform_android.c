/*
 * Android support routines for PhysicsFS.
 *
 * Please see the file LICENSE.txt in the source's root directory.
 *
 *  This file written by Ryan C. Gordon.
 */

#define __PHYSICSFS_INTERNAL__
#include "physfs_platforms.h"

#ifdef PHYSFS_PLATFORM_ANDROID

#include <jni.h>
#include <android/log.h>
#include "physfs_internal.h"

static char *prefpath = NULL;


int __PHYSFS_platformInit(void)
{
    return 1;  /* always succeed. */
} /* __PHYSFS_platformInit */


void __PHYSFS_platformDeinit(void)
{
    if (prefpath)
    {
        allocator.Free(prefpath);
        prefpath = NULL;
    } /* if */
} /* __PHYSFS_platformDeinit */


void __PHYSFS_platformDetectAvailableCDs(PHYSFS_StringCallback cb, void *data)
{
    /* no-op. */
} /* __PHYSFS_platformDetectAvailableCDs */


char *__PHYSFS_platformCalcBaseDir(const char *argv0)
{
    /* as a cheat, we expect argv0 to be a PHYSFS_AndroidInit* on Android. */
    PHYSFS_AndroidInit *ainit = (PHYSFS_AndroidInit *) argv0;
    char *retval = NULL;
    JNIEnv *jenv = NULL;
    jobject jcontext;

    if (ainit == NULL)
        return __PHYSFS_strdup("/");  /* oh well. */

    jenv = (JNIEnv *) ainit->jnienv;
    jcontext = (jobject) ainit->context;

    if ((*jenv)->PushLocalFrame(jenv, 16) >= 0)
    {
        jobject jfileobj = 0;
        jmethodID jmeth = 0;
        jthrowable jexception = 0;
        jstring jstr = 0;

        jmeth = (*jenv)->GetMethodID(jenv, (*jenv)->GetObjectClass(jenv, jcontext), "getPackageResourcePath", "()Ljava/lang/String;");
        jstr = (jstring)(*jenv)->CallObjectMethod(jenv, jcontext, jmeth);
        jexception = (*jenv)->ExceptionOccurred(jenv);  /* this can't throw an exception, right? Just in case. */
        if (jexception != NULL)
            (*jenv)->ExceptionClear(jenv);
        else
        {
            const char *path = (*jenv)->GetStringUTFChars(jenv, jstr, NULL);
            retval = __PHYSFS_strdup(path);
            (*jenv)->ReleaseStringUTFChars(jenv, jstr, path);
        } /* else */

        /* We only can rely on the Activity being valid during this function call,
           so go ahead and grab the prefpath too. */
        jmeth = (*jenv)->GetMethodID(jenv, (*jenv)->GetObjectClass(jenv, jcontext), "getFilesDir", "()Ljava/io/File;");
        jfileobj = (*jenv)->CallObjectMethod(jenv, jcontext, jmeth);
        if (jfileobj)
        {
            jmeth = (*jenv)->GetMethodID(jenv, (*jenv)->GetObjectClass(jenv, jfileobj), "getCanonicalPath", "()Ljava/lang/String;");
            jstr = (jstring)(*jenv)->CallObjectMethod(jenv, jfileobj, jmeth);
            jexception = (*jenv)->ExceptionOccurred(jenv);
            if (jexception != NULL)
                (*jenv)->ExceptionClear(jenv);
            else
            {
                const char *path = (*jenv)->GetStringUTFChars(jenv, jstr, NULL);
                const size_t len = strlen(path) + 2;
                prefpath = allocator.Malloc(len);
                if (prefpath)
                    snprintf(prefpath, len, "%s/", path);
                (*jenv)->ReleaseStringUTFChars(jenv, jstr, path);
            } /* else */
        } /* if */

        (*jenv)->PopLocalFrame(jenv, NULL);
    } /* if */

    /* we can't return NULL because then PhysicsFS will treat argv0 as a string, but it's a non-NULL jobject! */
    if (retval == NULL)
        retval = __PHYSFS_strdup("/");   /* we pray this works. */

    return retval;
} /* __PHYSFS_platformCalcBaseDir */


char *__PHYSFS_platformCalcPrefDir(const char *org, const char *app)
{
    return __PHYSFS_strdup(prefpath ? prefpath : "/");
} /* __PHYSFS_platformCalcPrefDir */

#endif /* PHYSFS_PLATFORM_ANDROID */

/* end of physfs_platform_android.c ... */

