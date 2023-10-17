/*
 *	client.cpp
 *
 *	Client functions of Tintirek's Client Library (C++)
 */


#include "trk_client.h"
#include "trk_string.h"
#include "cmdline.h"
#include "crypto.h"
#include "trkstring.h"

#include <openssl/x509.h>
#include <openssl/ssl.h>
#include <filesystem>
#include <fstream>
#include <string>

#if _WIN32
#include <windows.h>
#include <shlobj.h>
#endif


TrkString GetCurrentUserDir()
{
	TrkString HomeDir;
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

	return HomeDir;
}

int TrkSSLHelper::ClientVerifyCallback(int preverify, struct x509_store_ctx_st* x509_ctx)
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

		TrkString HomeDir = GetCurrentUserDir();

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