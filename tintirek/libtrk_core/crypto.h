/*
 *	crypto.h
 *
 *	Tintirek's cryptographic actions
 */

#ifndef TRK_CRYPTO_H
#define TRK_CRYPTO_H


#include "trk_types.h"


/* Implementation class for SSL (Client) */
class TrkSSL
{
public:
	TrkSSL(struct ssl_st* Client = nullptr);
	~TrkSSL();

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
	
	struct ssl_ctx_st* GetContext() const;
	bool IsClient() const;

private:
	struct ssl_ctx_st* ssl_context;
	bool isclient;
};

/* Helper class for Cryptography (uses OpenSSL) */
class TrkSSLHelper
{
public:
	static void InitSSL();
	static void PrintErrors();
	static TrkSSLCTX* CreateServerMethod();
	static TrkSSLCTX* CreateClientMethod();
	static TrkSSL* CreateClient(TrkSSLCTX* Context, int client_socket);
	static int AcceptClient(TrkSSL* Client);
	static int ConnectServer(TrkSSL* Client);
	static int Write(TrkSSL* Client, TrkString Buf, int Length);
	static int Read(TrkSSL* Client, TrkString& Buf, int Length);
	static int GetError(TrkSSL* Client);
	static bool LoadSSLFiles(TrkSSLCTX* SSLCTX, TrkString Path);
};


#endif /* TRK_CRYPTO_H */