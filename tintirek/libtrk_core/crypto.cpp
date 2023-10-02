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
#include <filesystem>
#include <regex>



/* Peer verification */
static int VerifyCallback(int preverify, X509_STORE_CTX* x509_ctx)
{
	X509* Cert = X509_STORE_CTX_get_current_cert(x509_ctx);

	if (Cert != nullptr)
	{
		if (preverify <= 0)
		{
			X509_STORE_CTX_set_error(x509_ctx, X509_V_OK);
		}

		unsigned char sha256_hash[SHA256_DIGEST_LENGTH];
		X509_digest(Cert, EVP_sha256(), sha256_hash, nullptr);

		TrkString fingerprint("");
		for (int i = 0; i < SHA256_DIGEST_LENGTH; i++)
		{
			char buffer[3];
			snprintf(buffer, sizeof(buffer), "%02X", sha256_hash[i]);
			fingerprint << buffer;

			if (i < SHA256_DIGEST_LENGTH - 1)
			{
				fingerprint << ":";
			}

		}

		std::cout << fingerprint << std::endl;
	}

	return 1;
}



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

TrkSSLCTX::TrkSSLCTX(ssl_ctx_st* Context, bool Client)
	: ssl_context(Context)
	, isclient(Client)
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

bool TrkSSLCTX::IsClient() const
{
	return isclient;
}

void TrkSSLHelper::InitSSL()
{
	SSL_library_init();
	SSL_load_error_strings();
}

void TrkSSLHelper::PrintErrors()
{
	ERR_print_errors_fp(stderr);
}

TrkSSLCTX* TrkSSLHelper::CreateServerMethod()
{
	SSL_CTX* ssl_ctx = SSL_CTX_new(TLS_server_method());
	SSL_CTX_set_min_proto_version(ssl_ctx, TLS1_VERSION);
	SSL_CTX_set_max_proto_version(ssl_ctx, TLS_MAX_VERSION);
	return new TrkSSLCTX(ssl_ctx, false);
}

TrkSSLCTX* TrkSSLHelper::CreateClientMethod()
{
	SSL_CTX* ssl_ctx = SSL_CTX_new(TLS_client_method());
	SSL_CTX_set_verify(ssl_ctx, SSL_VERIFY_PEER, VerifyCallback);
	return new TrkSSLCTX(ssl_ctx, true);
}

TrkSSL* TrkSSLHelper::CreateClient(TrkSSLCTX* Context, int client_socket)
{
	SSL* ssl = SSL_new(Context->GetContext());
	SSL_set_fd(ssl, client_socket);
	return new TrkSSL(ssl);
}

int TrkSSLHelper::AcceptClient(TrkSSL* Client)
{
	return SSL_accept(Client->GetClient());
}

int TrkSSLHelper::ConnectServer(TrkSSL* Client)
{
	return SSL_connect(Client->GetClient());
}

int TrkSSLHelper::Write(TrkSSL* Client, TrkString Buf, int Length)
{
	int status = SSL_write(Client->GetClient(), Buf, Length);
	if (status <= 0)
	{
		std::cout << "Write: " << GetError(Client) << std::endl;
		PrintErrors();
	}

	return status;
}

int TrkSSLHelper::Read(TrkSSL* Client, TrkString& Buf, int Length)
{
	char internal_strings[1024];
	int bytes_read = SSL_read(Client->GetClient(), internal_strings, Length);

	if (bytes_read > 0)
	{
		Buf = TrkString(internal_strings, internal_strings + bytes_read);
	}
	else
	{
		std::cout << GetError(Client) << std::endl;
		PrintErrors();
		Buf = TrkString("");
	}

	return bytes_read;
}

int TrkSSLHelper::GetError(TrkSSL* Client)
{
	return SSL_get_error(Client->GetClient(), -1);
}

bool TrkSSLHelper::LoadSSLFiles(TrkSSLCTX* SSLCTX, TrkString Path)
{
	TrkString certificate_path(std::filesystem::path(std::filesystem::canonical((const char*)Path) / "publickey.pem").string().c_str());
	TrkString private_key_path(std::filesystem::path(std::filesystem::canonical((const char*)Path) / "privatekey.pem").string().c_str());

	int returnable = SSL_CTX_use_certificate_file(SSLCTX->GetContext(), certificate_path, SSL_FILETYPE_PEM);

	if (returnable > 0)
	{
		returnable = SSL_CTX_use_PrivateKey_file(SSLCTX->GetContext(), private_key_path, SSL_FILETYPE_PEM) > 0;

		if (returnable > 0)
		{
			return true;
		}
	}

	PrintErrors();
	return false;
}