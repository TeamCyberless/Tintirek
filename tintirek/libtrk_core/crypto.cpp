/*
 *	crypto.h
 *
 *	Tintirek's cryptographic actions
 */


#include "crypto.h"

#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/rsa.h>
#include <openssl/evp.h>


TrkSSL::TrkSSL(struct ssl_st* Client)
	: ssl_client(Client)
{ }

TrkSSL::~TrkSSL()
{
	if (ssl_client != nullptr)
	{
		SSL_shutdown(ssl_client);
		SSL_free(ssl_client);
	}
}

struct ssl_st* TrkSSL::GetClient() const
{
	return ssl_client;
}

TrkSSLCTX::TrkSSLCTX(ssl_ctx_st* Context)
	: ssl_context(Context)
{ }

TrkSSLCTX::~TrkSSLCTX()
{
	if (ssl_context != nullptr)
	{
		SSL_CTX_free(ssl_context);
	}
}

ssl_ctx_st* TrkSSLCTX::GetContext() const
{
	return ssl_context;
}

void TrkSSLHelper::InitSSL()
{
	OPENSSL_init_ssl(OPENSSL_INIT_LOAD_SSL_STRINGS | OPENSSL_INIT_LOAD_CRYPTO_STRINGS, NULL);
	OPENSSL_init_crypto(OPENSSL_INIT_ADD_ALL_CIPHERS | OPENSSL_INIT_ADD_ALL_DIGESTS, NULL);
}

TrkSSLCTX TrkSSLHelper::CreateServerMethod()
{
	SSL_CTX* ssl_ctx = SSL_CTX_new(SSLv23_server_method());
	return TrkSSLCTX(ssl_ctx);
}

TrkSSL TrkSSLHelper::CreateClient(TrkSSLCTX Context, int client_socket)
{
	SSL* ssl = SSL_new(Context.GetContext());
	SSL_set_fd(ssl, client_socket);
	return TrkSSL(ssl);
}

int TrkSSLHelper::AcceptClient(TrkSSL* Client)
{
	return SSL_accept(Client->GetClient());
}

int TrkSSLHelper::Write(TrkSSL* Client, TrkString Buf, int Length)
{
	return SSL_write(Client->GetClient(), Buf, Length);
}

int TrkSSLHelper::Read(TrkSSL* Client, TrkString& Buf, int Length)
{
	char* data;
	int returnable = SSL_read(Client->GetClient(), data, Length);
	Buf = TrkString(data);
	return returnable;
}

bool TrkSSLHelper::LoadCertificateFile(TrkSSLCTX SSLCTX, TrkString CertificatePath)
{
	return SSL_CTX_use_certificate_file(SSLCTX.GetContext(), CertificatePath, SSL_FILETYPE_PEM) > 0;
}

bool TrkSSLHelper::LoadPrivateKeyFile(TrkSSLCTX SSLCTX, TrkString PrivateKeyPath)
{
	return SSL_CTX_use_PrivateKey_file(SSLCTX.GetContext(), PrivateKeyPath, SSL_FILETYPE_PEM) > 0;
}
