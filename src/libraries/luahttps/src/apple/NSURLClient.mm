#include "NSURLClient.h"

#ifdef HTTPS_BACKEND_NSURL

#import <Foundation/Foundation.h>

#if ! __has_feature(objc_arc)
#error "ARC is off"
#endif

bool NSURLClient::valid() const
{
	return true;
}

static std::string toCppString(NSData *data)
{
	return std::string((const char*) data.bytes, (size_t) data.length);
}

static std::string toCppString(NSString *str)
{
	return std::string([str UTF8String]);
}

HTTPSClient::Reply NSURLClient::request(const HTTPSClient::Request &req)
{ @autoreleasepool {
	NSURL *url = [NSURL URLWithString:@(req.url.c_str())];
	NSMutableURLRequest *request = [NSMutableURLRequest requestWithURL:url];

	NSData *bodydata = nil;
	[request setHTTPMethod:@(req.method.c_str())];

	if (req.postdata.size() > 0 && (req.method != "GET" && req.method != "HEAD"))
	{
		bodydata = [NSData dataWithBytesNoCopy:(void*) req.postdata.data() length:req.postdata.size() freeWhenDone:NO];
		[request setHTTPBody:bodydata];
	}

	for (auto &header : req.headers)
		[request setValue:@(header.second.c_str()) forHTTPHeaderField:@(header.first.c_str())];

	__block NSHTTPURLResponse *response = nil;
	__block NSError *error = nil;
	__block NSData *body = nil;

	dispatch_semaphore_t sem = dispatch_semaphore_create(0);

	NSURLSessionTask *task = [[NSURLSession sharedSession] dataTaskWithRequest:request
		completionHandler:^(NSData *data, NSURLResponse *resp, NSError *err) {
			body = data;
			response = (NSHTTPURLResponse *)resp;
			error = err;
			dispatch_semaphore_signal(sem);
	}];

	[task resume];

	dispatch_semaphore_wait(sem, DISPATCH_TIME_FOREVER);

	HTTPSClient::Reply reply;
	reply.responseCode = 0;

	if (body)
	{
		reply.body = toCppString(body);
	}

	if (response)
	{
		reply.responseCode = [response statusCode];

		NSDictionary *headers = [response allHeaderFields];
		for (NSString *key in headers)
		{
			NSString *value = headers[key];
			reply.headers[toCppString(key)] = toCppString(value);
		}
	}

	if (reply.responseCode == 0 && body == nil && error != nil)
	{
		reply.body = toCppString(error.localizedDescription);
	}

	return reply;
}}

#endif // HTTPS_BACKEND_NSURL
