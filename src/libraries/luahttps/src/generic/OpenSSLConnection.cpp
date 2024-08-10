#include "OpenSSLConnection.h"

#ifdef HTTPS_BACKEND_OPENSSL

#include "../common/LibraryLoader.h"

// Not present in openssl 1.1 headers
#define SSL_CTRL_OPTIONS 32

static bool TryOpenLibraries(const char *sslName, LibraryLoader::handle *& sslHandle, const char *cryptoName, LibraryLoader::handle *&cryptoHandle)
{
	sslHandle = LibraryLoader::OpenLibrary(sslName);
	cryptoHandle = LibraryLoader::OpenLibrary(cryptoName);

	if (sslHandle && cryptoHandle)
		return true;

	if (sslHandle)
		LibraryLoader::CloseLibrary(sslHandle);
	if (cryptoHandle)
		LibraryLoader::CloseLibrary(cryptoHandle);
	return false;
}

OpenSSLConnection::SSLFuncs::SSLFuncs()
{
	using namespace LibraryLoader;

	handle *sslhandle = nullptr;
	handle *cryptohandle = nullptr;

	valid = TryOpenLibraries("libssl.so.3", sslhandle, "libcrypto.so.3", cryptohandle)
		|| TryOpenLibraries("libssl.so.1.1", sslhandle, "libcrypto.so.1.1", cryptohandle)
		|| TryOpenLibraries("libssl.so.1.0.0", sslhandle, "libcrypto.so.1.0.0", cryptohandle)
		// Try the version-less name last, it may not be compatible or tested
		|| TryOpenLibraries("libssl.so", sslhandle, "libcrypto.so", cryptohandle);
	if (!valid)
		return;

	valid = true;
	valid = valid && (
			LoadSymbol(init_ssl, sslhandle, "OPENSSL_init_ssl") ||
			LoadSymbol(library_init, sslhandle, "SSL_library_init"));

	valid = valid && LoadSymbol(CTX_new, sslhandle, "SSL_CTX_new");
	valid = valid && LoadSymbol(CTX_ctrl, sslhandle, "SSL_CTX_ctrl");
	if (valid)
		LoadSymbol(CTX_set_options, sslhandle, "SSL_CTX_set_options");
	valid = valid && LoadSymbol(CTX_set_verify, sslhandle, "SSL_CTX_set_verify");
	valid = valid && LoadSymbol(CTX_set_default_verify_paths, sslhandle, "SSL_CTX_set_default_verify_paths");
	valid = valid && LoadSymbol(CTX_free, sslhandle, "SSL_CTX_free");

	valid = valid && LoadSymbol(SSL_new, sslhandle, "SSL_new");
	valid = valid && LoadSymbol(SSL_free, sslhandle, "SSL_free");
	valid = valid && LoadSymbol(set_fd, sslhandle, "SSL_set_fd");
	valid = valid && LoadSymbol(connect, sslhandle, "SSL_connect");
	valid = valid && LoadSymbol(read, sslhandle, "SSL_read");
	valid = valid && LoadSymbol(write, sslhandle, "SSL_write");
	valid = valid && LoadSymbol(shutdown, sslhandle, "SSL_shutdown");
	valid = valid && LoadSymbol(get_verify_result, sslhandle, "SSL_get_verify_result");
	valid = valid && (LoadSymbol(get_peer_certificate, sslhandle, "SSL_get1_peer_certificate") ||
			LoadSymbol(get_peer_certificate, sslhandle, "SSL_get_peer_certificate"));

	valid = valid && (
			LoadSymbol(SSLv23_method, sslhandle, "TLS_client_method") ||
			LoadSymbol(SSLv23_method, sslhandle, "TLS_method") ||
			LoadSymbol(SSLv23_method, sslhandle, "SSLv23_method"));

	valid = valid && LoadSymbol(check_host, cryptohandle, "X509_check_host");
	valid = valid && LoadSymbol(X509_free, cryptohandle, "X509_free");

	if (library_init)
		library_init();
	else if(init_ssl)
		init_ssl(0, nullptr);
	// else not valid
}

bool OpenSSLConnection::valid()
{
	return ssl.valid;
}

OpenSSLConnection::OpenSSLConnection()
	: conn(nullptr)
{
	context = ssl.CTX_new(ssl.SSLv23_method());
	if (!context)
		return;

	if (ssl.CTX_set_options)
		ssl.CTX_set_options(context, SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3);
	else
		ssl.CTX_ctrl(context, SSL_CTRL_OPTIONS, SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3, nullptr);
	ssl.CTX_set_verify(context, SSL_VERIFY_PEER, nullptr);
	ssl.CTX_set_default_verify_paths(context);
}

OpenSSLConnection::~OpenSSLConnection()
{
	if (conn)
		ssl.SSL_free(conn);

	if (context)
		ssl.CTX_free(context);
}

bool OpenSSLConnection::connect(const std::string &hostname, uint16_t port)
{
	if (!context)
		return false;

	if (!socket.connect(hostname, port))
		return false;

	conn = ssl.SSL_new(context);
	if (!conn)
	{
		socket.close();
		return false;
	}

	ssl.set_fd(conn, socket.getFd());
	if (ssl.connect(conn) != 1 || ssl.get_verify_result(conn) != X509_V_OK)
	{
		socket.close();
		return false;
	}

	X509 *cert = ssl.get_peer_certificate(conn);
	if (ssl.check_host(cert, hostname.c_str(), hostname.size(), 0, nullptr) != 1)
	{
		close();
		return false;
	}
	ssl.X509_free(cert);

	return true;
}

size_t OpenSSLConnection::read(char *buffer, size_t size)
{
	return ssl.read(conn, buffer, (int) size);
}

size_t OpenSSLConnection::write(const char *buffer, size_t size)
{
	return ssl.write(conn, buffer, (int) size);
}

void OpenSSLConnection::close()
{
	ssl.shutdown(conn);
	socket.close();
}

OpenSSLConnection::SSLFuncs OpenSSLConnection::ssl;

#endif // HTTPS_BACKEND_OPENSSL
