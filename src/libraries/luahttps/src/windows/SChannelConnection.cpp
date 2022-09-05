#define SECURITY_WIN32
#define NOMINMAX

#include "SChannelConnection.h"

#ifdef HTTPS_BACKEND_SCHANNEL

#include <windows.h>
#include <security.h>
#include <schnlsp.h>
#include <assert.h>
#include <algorithm>
#include <memory>
#include <array>

#ifndef SCH_USE_STRONG_CRYPTO
#	define SCH_USE_STRONG_CRYPTO 0x00400000
#endif
#ifndef SP_PROT_TLS1_1_CLIENT
#	define SP_PROT_TLS1_1_CLIENT 0x00000200
#endif
#ifndef SP_PROT_TLS1_2_CLIENT
#	define SP_PROT_TLS1_2_CLIENT 0x00000800
#endif

#ifdef DEBUG_SCHANNEL
#include <iostream>
std::ostream &debug = std::cout;
#else
struct Debug
{
	template<typename T>
	Debug &operator<<(const T&) { return *this; }
} debug;
#endif

static void enqueue(std::vector<char> &buffer, char *data, size_t size)
{
	size_t oldSize = buffer.size();
	buffer.resize(oldSize + size);
	memcpy(&buffer[oldSize], data, size);
}

static void enqueue_prepend(std::vector<char> &buffer, char *data, size_t size)
{
	size_t oldSize = buffer.size();
	buffer.resize(oldSize + size);
	if (oldSize > 0)
		memmove(&buffer[size], &buffer[0], oldSize);
	memcpy(&buffer[0], data, size);
}

static size_t dequeue(std::vector<char> &buffer, char *data, size_t size)
{
	size = std::min(size, buffer.size());
	size_t remaining = buffer.size() - size;

	memcpy(data, &buffer[0], size);

	if (remaining > 0)
	{
		memmove(&buffer[0], &buffer[size], remaining);
		buffer.resize(remaining);
	}
	else
	{
		buffer.resize(0);
	}

	return size;
}

SChannelConnection::SChannelConnection()
	: context(nullptr)
{
}

SChannelConnection::~SChannelConnection()
{
	destroyContext();
}

SECURITY_STATUS InitializeSecurityContext(CredHandle *phCredential, std::unique_ptr<CtxtHandle>& phContext, const std::string& szTargetName, ULONG fContextReq, std::vector<char>& inputBuffer, std::vector<char>& outputBuffer, ULONG *pfContextAttr)
{
	std::array<SecBuffer, 1> recvBuffers;
	recvBuffers[0].BufferType = SECBUFFER_TOKEN;
	recvBuffers[0].pvBuffer = outputBuffer.data();
	recvBuffers[0].cbBuffer = outputBuffer.size();

	std::array<SecBuffer, 2> sendBuffers;
	sendBuffers[0].BufferType = SECBUFFER_TOKEN;
	sendBuffers[0].pvBuffer = inputBuffer.data();
	sendBuffers[0].cbBuffer = inputBuffer.size();
	sendBuffers[1].BufferType = SECBUFFER_EMPTY;
	sendBuffers[1].pvBuffer = nullptr;
	sendBuffers[1].cbBuffer = 0;

	SecBufferDesc recvBufferDesc, sendBufferDesc;
	recvBufferDesc.ulVersion = sendBufferDesc.ulVersion = SECBUFFER_VERSION;
	recvBufferDesc.pBuffers = &recvBuffers[0];
	recvBufferDesc.cBuffers = recvBuffers.size();

	if (!inputBuffer.empty())
	{
		sendBufferDesc.pBuffers = &sendBuffers[0];
		sendBufferDesc.cBuffers = sendBuffers.size();
	}
	else
	{
		sendBufferDesc.pBuffers = nullptr;
		sendBufferDesc.cBuffers = 0;
	}

	CtxtHandle* phOldContext = nullptr;
	CtxtHandle* phNewContext = nullptr;
	if (!phContext)
	{
		phContext = std::make_unique<CtxtHandle>();
		phNewContext = phContext.get();
	}
	else
	{
		phOldContext = phContext.get();
	}

	auto ret = InitializeSecurityContext(phCredential, phOldContext, const_cast<char*>(szTargetName.c_str()), fContextReq, 0, 0, &sendBufferDesc, 0, phNewContext, &recvBufferDesc, pfContextAttr, nullptr);

	outputBuffer.resize(recvBuffers[0].cbBuffer);

	// Clear the input buffer, so the reader can append
	// If we have unprocessed data, leave it in the buffer
	size_t unprocessed = 0;
	if (sendBuffers[1].BufferType == SECBUFFER_EXTRA)
		unprocessed = sendBuffers[1].cbBuffer;

	if (unprocessed > 0)
		memmove(inputBuffer.data(), inputBuffer.data() + inputBuffer.size() - unprocessed, unprocessed);

	inputBuffer.resize(unprocessed);

	return ret;
}

bool SChannelConnection::connect(const std::string &hostname, uint16_t port)
{
	debug << "Trying to connect to " << hostname << ":" << port << "\n";
	if (!socket.connect(hostname, port))
		return false;
	debug << "Connected\n";

	SCHANNEL_CRED cred;
	memset(&cred, 0, sizeof(cred));

	cred.dwVersion = SCHANNEL_CRED_VERSION;
	cred.grbitEnabledProtocols = SP_PROT_TLS1_CLIENT | SP_PROT_TLS1_1_CLIENT | SP_PROT_TLS1_2_CLIENT;
	cred.dwFlags = SCH_CRED_AUTO_CRED_VALIDATION | SCH_CRED_NO_DEFAULT_CREDS | SCH_USE_STRONG_CRYPTO | SCH_CRED_REVOCATION_CHECK_CHAIN;

	CredHandle credHandle;
	if (AcquireCredentialsHandle(nullptr, (char*) UNISP_NAME, SECPKG_CRED_OUTBOUND, nullptr, &cred, nullptr, nullptr, &credHandle, nullptr) != SEC_E_OK)
	{
		debug << "Failed to acquire handle\n";
		socket.close();
		return false;
	}
	debug << "Acquired handle\n";


	static constexpr size_t bufferSize = 8192;
	bool done = false, success = false, contextCreated = false;

	ULONG contextAttr;
	std::unique_ptr<CtxtHandle> context;
	std::vector<char> inputBuffer;
	std::vector<char> outputBuffer;

	do
	{
		outputBuffer.resize(bufferSize);

		bool recvData = false;
		bool sendData = false;
		auto ret = InitializeSecurityContext(&credHandle, context, hostname, ISC_REQ_STREAM, inputBuffer, outputBuffer, &contextAttr);
		switch (ret)
		{
		/*case SEC_I_COMPLETE_NEEDED:
		case SEC_I_COMPLETE_AND_CONTINUE:
			if (CompleteAuthToken(context.get(), &outputBuffer) != SEC_E_OK)
				done = true;
			else if (ret == SEC_I_COMPLETE_NEEDED)
				success = done = true;
			break;*/
		case SEC_I_CONTINUE_NEEDED:
			debug << "Initialize: continue needed\n";
			recvData = true;
			sendData = true;
			break;
		case SEC_E_INCOMPLETE_CREDENTIALS:
			debug << "Initialize failed: incomplete credentials\n";
			done = true;
			break;
		case SEC_E_INCOMPLETE_MESSAGE:
			debug << "Initialize: incomplete message\n";
			recvData = true;
			break;
		case SEC_E_OK:
			debug << "Initialize succeeded\n";
			success = done = true;
			sendData = true;
			break;
		default:
			debug << "Initialize done: " << outputBuffer.size() << " bytes of output and unknown status " << ret << "\n";
			done = true;
			success = false;
			break;
		}

		if (!done)
			contextCreated = true;

		if (sendData && !outputBuffer.empty())
		{
			socket.write(outputBuffer.data(), outputBuffer.size());
			debug << "Sent " << outputBuffer.size() << " bytes of data\n";
		}

		if (recvData)
		{
			size_t unprocessed = inputBuffer.size();
			inputBuffer.resize(unprocessed + bufferSize);
			size_t actual = socket.read(inputBuffer.data() + unprocessed, bufferSize);
			inputBuffer.resize(actual + unprocessed);

			debug << "Received " << actual << " bytes of data\n";
			if (unprocessed > 0)
				debug << "  had " << unprocessed << " bytes of remaining, unprocessed data\n";

			if (actual + unprocessed == 0)
			{
				debug << "No data to submit, break\n";
				break;
			}
		}
	} while (!done);

	debug << "Done!\n";

	if (success)
	{
		SecPkgContext_Flags resultFlags;
		QueryContextAttributes(context.get(), SECPKG_ATTR_FLAGS, &resultFlags);
		if (resultFlags.Flags & ISC_REQ_CONFIDENTIALITY == 0)
		{
			debug << "Resulting context is not encrypted, marking as failed\n";
			success = false;
		}
		if (resultFlags.Flags & ISC_REQ_INTEGRITY == 0)
		{
			debug << "Resulting context is not signed, marking as failed\n";
			success = false;
		}
	}

	if (success)
		this->context = context.release();
	else if (contextCreated)
		DeleteSecurityContext(context.get());

	return success;
}

size_t SChannelConnection::read(char *buffer, size_t size)
{
	if (decRecvBuffer.size() > 0)
	{
		size = dequeue(decRecvBuffer, buffer, size);
		debug << "Read " << size << " bytes of previously decoded data\n";
		return size;
	}
	else if (encRecvBuffer.size() > 0)
	{
		size = dequeue(encRecvBuffer, buffer, size);
		debug << "Read " << size << " bytes of extra data\n";
	}
	else
	{
		size = socket.read(buffer, size);
		debug << "Received " << size << " bytes of data\n";
	}

	return decrypt(buffer, size);
}

size_t SChannelConnection::decrypt(char *buffer, size_t size, bool recurse)
{
	if (size == 0)
		return 0;

	SecBuffer secBuffers[4];
	secBuffers[0].cbBuffer = size;
	secBuffers[0].BufferType = SECBUFFER_DATA;
	secBuffers[0].pvBuffer = buffer;

	for (size_t i = 1; i < 4; ++i)
	{
		secBuffers[i].BufferType = SECBUFFER_EMPTY;
		secBuffers[i].pvBuffer = nullptr;
		secBuffers[i].cbBuffer = 0;
	}

	SecBufferDesc secBufferDesc;
	secBufferDesc.ulVersion = SECBUFFER_VERSION;
	secBufferDesc.cBuffers = 4;
	secBufferDesc.pBuffers = &secBuffers[0];

	auto ret = DecryptMessage(static_cast<CtxtHandle*>(context), &secBufferDesc, 0, nullptr); // FIXME
	debug << "DecryptMessage returns: " << ret << "\n";
	switch (ret)
	{
	case SEC_E_OK:
	{
		void *actualDataStart = buffer;
		for (size_t i = 0; i < 4; ++i)
		{
			auto &buffer = secBuffers[i];
			if (buffer.BufferType == SECBUFFER_DATA)
			{
				actualDataStart = buffer.pvBuffer;
				size = buffer.cbBuffer;
			}
			else if (buffer.BufferType == SECBUFFER_EXTRA)
			{
				debug << "\tExtra data in buffer " << i << " (" << buffer.cbBuffer << " bytes)\n";
				enqueue(encRecvBuffer, static_cast<char*>(buffer.pvBuffer), buffer.cbBuffer);
			}
			else if (buffer.BufferType != SECBUFFER_EMPTY)
				debug << "\tBuffer of type " << buffer.BufferType << "\n";
		}

		if (actualDataStart)
			memmove(buffer, actualDataStart, size);

		break;
	}
	case SEC_E_INCOMPLETE_MESSAGE:
	{
		// Move all our current data to encRecvBuffer
		enqueue(encRecvBuffer, buffer, size);

		// Now try to read some more data from the socket
		size_t bufferSize = encRecvBuffer.size() + 8192;
		char *recvBuffer = new char[bufferSize];
		size_t recvd = socket.read(recvBuffer+encRecvBuffer.size(), 8192);
		debug << recvd << " bytes of extra data read from socket\n";

		if (recvd == 0 && !recurse)
		{
			debug << "Recursion prevented, bailing\n";
			return 0;
		}

		// Fill our buffer with the queued data and the newly received data
		size_t totalSize = encRecvBuffer.size() + recvd;
		dequeue(encRecvBuffer, recvBuffer, encRecvBuffer.size());
		debug << "Trying to decrypt with " << totalSize << " bytes of data\n";

		// Now try to decrypt that
		size_t decrypted = decrypt(recvBuffer, totalSize, false);
		debug << "\tObtained " << decrypted << " bytes of decrypted data\n";

		// Copy the first size bytes to the output buffer
		size = std::min(size, decrypted);
		memcpy(buffer, recvBuffer, size);

		// And write the remainder to our queued decrypted data...
		// Note: we prepend, since our recursive call may already have written
		// something and we can be sure decrypt wasn't called if the buffer was
		// non-empty in read
		enqueue_prepend(decRecvBuffer, recvBuffer+size, decrypted-size);
		debug << "\tStoring " << decrypted-size << " bytes of extra decrypted data\n";
		return size;
	}
	// TODO: More?
	default:
		size = 0;
		break;
	}

	debug << "\tDecrypted " << size << " bytes of data\n";

	return size;
}

size_t SChannelConnection::write(const char *buffer, size_t size)
{
	static constexpr size_t bufferSize = 8192;
	assert(size <= bufferSize);

	SecPkgContext_StreamSizes Sizes;
	QueryContextAttributes(
            static_cast<CtxtHandle*>(context),
            SECPKG_ATTR_STREAM_SIZES,
            &Sizes);
	debug << "stream sizes:\n\theader: " << Sizes.cbHeader << "\n\tfooter: " << Sizes.cbTrailer << "\n";

	char *sendBuffer = new char[bufferSize + Sizes.cbHeader + Sizes.cbTrailer];
	memcpy(sendBuffer+Sizes.cbHeader, buffer, size);

	SecBuffer secBuffers[4];
	secBuffers[0].cbBuffer = Sizes.cbHeader;
	secBuffers[0].BufferType = SECBUFFER_STREAM_HEADER;
	secBuffers[0].pvBuffer = sendBuffer;

	secBuffers[1].cbBuffer = size;
	secBuffers[1].BufferType = SECBUFFER_DATA;
	secBuffers[1].pvBuffer = sendBuffer+Sizes.cbHeader;

	secBuffers[2].cbBuffer = Sizes.cbTrailer;
	secBuffers[2].pvBuffer = sendBuffer+Sizes.cbHeader+size;
	secBuffers[2].BufferType = SECBUFFER_STREAM_TRAILER;

	secBuffers[3].cbBuffer = 0;
	secBuffers[3].BufferType = SECBUFFER_EMPTY;
	secBuffers[3].pvBuffer = nullptr;

	SecBufferDesc secBufferDesc;
	secBufferDesc.ulVersion = SECBUFFER_VERSION;
	secBufferDesc.cBuffers = 4;
	secBufferDesc.pBuffers = secBuffers;

	auto ret = EncryptMessage(static_cast<CtxtHandle*>(context), 0, &secBufferDesc, 0); // FIXME
	debug << "Send:\n\tHeader size: " << secBuffers[0].cbBuffer << "\n\t\ttype: " << secBuffers[0].BufferType << "\n\tData size: " << secBuffers[1].cbBuffer << "\n\t\ttype: " << secBuffers[1].BufferType << "\n\tFooter size: " << secBuffers[2].cbBuffer << "\n\t\ttype: " << secBuffers[2].BufferType << "\n";

	size_t sendSize = 0;
	for (size_t i = 0; i < 4; ++i)
		if (secBuffers[i].cbBuffer != bufferSize)
			sendSize += secBuffers[i].cbBuffer;

	debug << "\tReal length? " << sendSize << "\n";
	switch (ret)
	{
	case SEC_E_OK:
		socket.write(sendBuffer, sendSize);
		break;
	// TODO: More?
	default:
		size = 0;
		break;
	}

	delete[] sendBuffer;
	return size;
}

void SChannelConnection::destroyContext()
{
	if (context)
	{
		DeleteSecurityContext(context);
		delete context;
		context = nullptr;
	}
}

void SChannelConnection::close()
{
	destroyContext();
	socket.close();
}

bool SChannelConnection::valid()
{
	return true;
}

#endif // HTTPS_BACKEND_SCHANNEL
