/*
 *	crypto.h
 *
 *	Tintirek's cryptographic actions
 */

#ifndef TRK_CRYPTO_H
#define TRK_CRYPTO_H


#include "trk_types.h"


/* Implementation class for SSL */
class TrkSSL
{
public:
	TrkSSL(struct ssl_st* Client = nullptr);
	~TrkSSL();

	/* Returns SSL client reference */
	struct ssl_st* GetClient() const;

private:
	struct ssl_st* ssl_client;
};

/* Implementation class for SSL_CTX */
class TrkSSLCTX
{
public:
	TrkSSLCTX(struct ssl_ctx_st* Context = nullptr, bool Client = false);
	~TrkSSLCTX();
	
	/* Returns SSL context reference */
	struct ssl_ctx_st* GetContext() const;
	/* Returns true if this context is created from the client method, false otherwise */
	bool IsClient() const;

private:
	struct ssl_ctx_st* ssl_context;
	bool isclient;
};

/* Helper class for SSL */
class TrkSSLHelper
{
public:
	/* Initialize the SSL engine. This function must be called once before using any SSL functions */
	static void InitSSL();
	/* Print error messages to the standard error stream. Use this function primarily in Debug mode */
	static void PrintErrors();
	/* It's used for the purpose of refreshing error messages. */
	static void RefreshErrors();
	/* Adds program options to an SSL_CTX object and uses it during certificate verification */
	static void SetupProgramOptionsToContext(TrkSSLCTX* Context, class TrkCliClientOptionResults* Options);
	/* Create an SSL context using server methods */
	static TrkSSLCTX* CreateServerMethod();
	/* Create an SSL context using client methods */
	static TrkSSLCTX* CreateClientMethod();
	/* Establish an SSL connection reference for a client */
	static TrkSSL* CreateClient(TrkSSLCTX* Context, int client_socket);
	/* Accept an incoming client connection */
	static int AcceptClient(TrkSSL* Client);
	/* Connect to an SSL-enabled server */
	static int ConnectServer(TrkSSL* Client);
	/* Send data to an SSL-enabled server */
	static int Write(TrkSSL* Client, std::string Buf, int Length);
	/* Receive data from an SSL-enabled server */
	static int Read(TrkSSL* Client, std::string& Buf, int Length);
	/* Get the error code that occurred during SSL communication */
	static int GetError(TrkSSL* Client);
	/* Load private and public keys from the specified path */
	static bool LoadSSLFiles(TrkSSLCTX* SSLCTX, std::string Path);


	/* Peer verification for Client-Side */
	static int ClientVerifyCallback(int preverify, struct x509_store_ctx_st* x509_ctx);
};

/* Helper class foR Cryptography */
class TrkCryptoHelper
{
public:
	/* Calculate the SHA-256 hash of the input daha */
	static std::string SHA256(const std::string& Str, const std::string& Seperator = "");
	/* Cenerates a 16 bytes length random salt, which is a random string of characters */
	static std::string GenerateSalt();
};


#endif /* TRK_CRYPTO_H */