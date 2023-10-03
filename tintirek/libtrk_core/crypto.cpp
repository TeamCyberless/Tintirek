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
#include <fstream>

#if _WIN32
#include <windows.h>
#include <shlobj.h>
#endif

#include "trk_cmdline.h"



/* Peer verification for Client-Side */
static int ClientVerifyCallback(int preverify, X509_STORE_CTX* x509_ctx)
{
	X509* Cert = X509_STORE_CTX_get_current_cert(x509_ctx);
	SSL* ssl = static_cast<SSL*>(X509_STORE_CTX_get_ex_data(x509_ctx, SSL_get_ex_data_X509_STORE_CTX_idx()));
	TrkCliClientOptionResults* ClientOptions = static_cast<TrkCliClientOptionResults*>(SSL_CTX_get_app_data(SSL_get_SSL_CTX(ssl)));

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

		TrkString HomeDir = "";
#if _WIN32
		{
			WCHAR path[MAX_PATH];
			if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_PROFILE, NULL, 0, path)))
			{
				char* buffer = new char[MAX_PATH];
				int length = WideCharToMultiByte(CP_UTF8, 0, path, -1, buffer, MAX_PATH, NULL, NULL);
				if (length > 0)
				{
					HomeDir = buffer;
				}
				delete[] buffer;
			}
		}
#else
		{
			const char* homeDir = getenv("HOME");
			HomeDir = (homeDir != nullptr ? homeDir : "");
		}
#endif
		
		try
		{
			namespace fs = std::filesystem;
			fs::path TrustFile = fs::canonical(fs::path((const char*)HomeDir) / ".trktrust");

			if (fs::exists(TrustFile) && fs::is_regular_file(TrustFile))
			{
				std::ifstream TrustFileReader(TrustFile);
				if (TrustFileReader.is_open())
				{
					std::string l;
					while (std::getline(TrustFileReader, l))
					{
						const TrkString line(l.c_str());
						if (fingerprint == line)
						{
							TrustFileReader.close();

							// We can trust this certificate but check if we're trying to execute trust command
							if (ClientOptions->requested_command->command == "trust")
							{
								return 0;
							}
							return 1;
						}
					}

					TrustFileReader.close();
				}

				throw std::exception();
			}
			else
			{
				// Don't trust this certificate because we don't have any trust
				// chain file or we could not find this certificate in our chain.
				throw std::exception();
			}
		}
		catch (std::exception ex)
		{
			if (ClientOptions != nullptr)
			{
				ClientOptions->last_certificate_fingerprint = TrkString(fingerprint);
			}
			return 0;
		}
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

void TrkSSLHelper::SetupProgramOptionsToContext(TrkSSLCTX* Context, class TrkCliOptionResults* Options)
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
	SSL_CTX_set_verify(ssl_ctx, SSL_VERIFY_PEER, ClientVerifyCallback);
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
	else if (bytes_read <= 0)
	{
		Buf = TrkString("");
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

bool TrkSSLHelper::LoadSSLFiles(TrkSSLCTX* SSLCTX, TrkString Path)
{
	TrkString certificate_path(std::filesystem::path(std::filesystem::canonical((const char*)Path) / "publickey.pem").string().c_str());
	TrkString private_key_path(std::filesystem::path(std::filesystem::canonical((const char*)Path) / "privatekey.pem").string().c_str());

	if (SSL_CTX_use_certificate_file(SSLCTX->GetContext(), certificate_path, SSL_FILETYPE_PEM) <= 0)
	{
		PrintErrors();
		return false;
	}

	if (SSL_CTX_use_PrivateKey_file(SSLCTX->GetContext(), private_key_path, SSL_FILETYPE_PEM) <= 0)
	{
		PrintErrors();
		return false;
	}

	return true;
}