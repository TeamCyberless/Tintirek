/*
 *	crypto.h
 *
 *	Tintirek's cryptographic actions
 */


#include "crypto.h"

#include <iostream>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/rsa.h>
#include <openssl/evp.h>
#include <openssl/sha.h>
#include <openssl/rand.h>
#include <filesystem>
#include <fstream>
#include <sstream>

#include "cmdline.h"
#include "trk_types.h"

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
	if (ssl_client != nullptr)
	{
		return ssl_client;
	}

	return nullptr;
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

void TrkSSLHelper::RefreshErrors()
{
	ERR_clear_error();
}

void TrkSSLHelper::SetupProgramOptionsToContext(TrkSSLCTX* Context, class TrkCliClientOptionResults* Options)
{
	SSL_CTX_set_app_data(Context->GetContext(), Options);
}

TrkSSLCTX* TrkSSLHelper::CreateServerMethod()
{
	SSL_CTX* ssl_ctx = SSL_CTX_new(TLS_server_method());
	
	if (!ssl_ctx)
	{
		PrintErrors();
		return nullptr;
	}

	return new TrkSSLCTX(ssl_ctx, false);
}

TrkSSLCTX* TrkSSLHelper::CreateClientMethod()
{
	SSL_CTX* ssl_ctx = SSL_CTX_new(TLS_client_method());
#ifdef LIBRARY_TRK_CLIENT
	SSL_CTX_set_verify(ssl_ctx, SSL_VERIFY_PEER, TrkSSLHelper::ClientVerifyCallback);
#endif
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

int TrkSSLHelper::Write(TrkSSL* Client, std::string Buf, int Length)
{
	int status = SSL_write(Client->GetClient(), Buf.c_str(), Length);
	if (status <= 0)
	{
		std::cout << "Write: " << GetError(Client) << std::endl;
		PrintErrors();
	}

	return status;
}

int TrkSSLHelper::Read(TrkSSL* Client, std::string& Buf, int Length)
{
	char internal_strings[1024];
	int bytes_read = SSL_read(Client->GetClient(), internal_strings, Length);

	if (bytes_read > 0)
	{
		Buf = std::string(internal_strings, internal_strings + bytes_read);
	}
	else if (bytes_read <= 0)
	{
		Buf = std::string("");
	}

	return bytes_read;
}

int TrkSSLHelper::GetError(TrkSSL* Client)
{
	int val = SSL_get_error(Client->GetClient(), -1);
	if (val == -1)
	{
		val = ERR_get_error();
	}
	return val;
}

bool TrkSSLHelper::LoadSSLFiles(TrkSSLCTX* SSLCTX, std::string Path)
{
	std::string certificate_path(std::filesystem::path(std::filesystem::canonical(Path.c_str()) / "publickey.pem").string().c_str());
	std::string private_key_path(std::filesystem::path(std::filesystem::canonical(Path.c_str()) / "privatekey.pem").string().c_str());

	if (SSL_CTX_use_certificate_file(SSLCTX->GetContext(), certificate_path.c_str(), SSL_FILETYPE_PEM) <= 0)
	{
		PrintErrors();
		return false;
	}

	if (SSL_CTX_use_PrivateKey_file(SSLCTX->GetContext(), private_key_path.c_str(), SSL_FILETYPE_PEM) <= 0)
	{
		PrintErrors();
		return false;
	}

	return true;
}

#ifndef LIBRARY_TRK_CLIENT
int ClientVerifyCallback(int preverify, struct x509_store_ctx_st* x509_ctx)
{ return 0; }
#endif

std::string TrkCryptoHelper::SHA256(const std::string& Str, const std::string& Seperator)
{
	unsigned char hash[SHA256_DIGEST_LENGTH];
	SHA256_CTX sha256;
	SHA256_Init(&sha256);
	SHA256_Update(&sha256, Str.c_str(), Str.size());
	SHA256_Final(hash, &sha256);

	std::stringstream hashString;
	char hex[3];
	for (int i = 0; i < SHA256_DIGEST_LENGTH; i++)
	{
		sprintf(hex, "%02x", hash[i]);
		hashString << hex;

		if (Seperator != "" && i < SHA256_DIGEST_LENGTH - 1)
		{
			hashString << Seperator;
		}
	}

	return hashString.str();
}

std::string TrkCryptoHelper::GenerateSalt()
{
	std::string salt;
	unsigned char* saltBuffer = new unsigned char[12];
	BIO *b64 = nullptr, *out = nullptr;
	BUF_MEM* bptr = nullptr;

	if ((b64 = BIO_new(BIO_f_base64())) == nullptr)
	{
		delete[] saltBuffer;
		return "";
	}

	if ((out = BIO_new(BIO_s_mem())) == nullptr)
	{
		BIO_free(b64);
		delete[] saltBuffer;
		return "";
	}

	out = BIO_push(b64, out);
	BIO_set_flags(out, BIO_FLAGS_BASE64_NO_NL);
	
	if (!RAND_bytes(saltBuffer, 12))
	{
		delete[] saltBuffer;
		BIO_free_all(out);
		return "";
	}

	BIO_write(out, saltBuffer, 12);
	BIO_flush(out);

	out = BIO_pop(b64);

	BIO_write(out, "\0", 1);
	BIO_get_mem_ptr(out, &bptr);

	salt = std::string(bptr->data, bptr->data + bptr->length);

	BIO_set_close(out, BIO_CLOSE);
	BIO_free_all(out);
	delete[] saltBuffer;

	return salt;
}
