#pragma once

#include "../common/config.h"

#ifdef HTTPS_BACKEND_ANDROID

#include <jni.h>

#include "../common/HTTPSClient.h"

class AndroidClient: public HTTPSClient
{
public:
	AndroidClient();

	bool valid() const override;
	HTTPSClient::Reply request(const HTTPSClient::Request &req) override;

private:
	JNIEnv *(*SDL_AndroidGetJNIEnv)();
	jobject (*SDL_AndroidGetActivity)();

	jclass getHTTPSClass() const;
};

#endif
