#pragma once

#include "../common/config.h"

#ifdef HTTPS_BACKEND_OPENSSL

#include <openssl/ssl.h>

#include "../common/Connection.h"
#include "../common/PlaintextConnection.h"

class OpenSSLConnection : public Connection
{
public:
	OpenSSLConnection();
	virtual bool connect(const std::string &hostname, uint16_t port) override;
	virtual size_t read(char *buffer, size_t size) override;
	virtual size_t write(const char *buffer, size_t size) override;
	virtual void close() override;
	virtual ~OpenSSLConnection();

	static bool valid();

private:
	PlaintextConnection socket;
	SSL_CTX *context;
	SSL *conn;

	struct SSLFuncs
	{
		SSLFuncs();
		bool valid;

		int (*library_init)();
		int (*init_ssl)(uint64_t opts, const void *settings);

		SSL_CTX *(*CTX_new)(const SSL_METHOD *method);
		long (*CTX_ctrl)(SSL_CTX *ctx, int cmd, long larg, void *parg);
		long (*CTX_set_options)(SSL_CTX *ctx, long options);
		void (*CTX_set_verify)(SSL_CTX *ctx, int mode, void *verify_callback);
		int (*CTX_set_default_verify_paths)(SSL_CTX *ctx);
		void (*CTX_free)(SSL_CTX *ctx);

		SSL *(*SSL_new)(SSL_CTX *ctx);
		void (*SSL_free)(SSL *ctx);
		int (*set_fd)(SSL *ssl, int fd);
		int (*connect)(SSL *ssl);
		int (*read)(SSL *ssl, void *buf, int num);
		int (*write)(SSL *ssl, const void *buf, int num);
		int (*shutdown)(SSL *ssl);
		long (*get_verify_result)(const SSL *ssl);
		X509 *(*get_peer_certificate)(const SSL *ssl);

		const SSL_METHOD *(*SSLv23_method)();

		int (*check_host)(X509 *cert, const char *name, size_t namelen, unsigned int flags, char **peername);
		void (*X509_free)(X509* cert);
	};
	static SSLFuncs ssl;
};

#endif // HTTPS_BACKEND_OPENSSL
