/*
 *	config.cpp
 *
 *  Tintirek's configuration reader system
 */

#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>
#include <cstdlib>

#ifdef _WIN32
#include <Windows.h>
#elif __unix__
#include <unistd.h>
#include <limits.h>
#include <stdlib.h>
#endif


#include "trk_config.h"

namespace fs = std::filesystem;


/* Recursive config finder and loader */
static bool FindAndLoadConfig(const fs::path& currentPath, const char* configFileName, TrkString& valid_path)
{
    fs::path configPath = currentPath / configFileName;
    if (fs::exists(configPath)) {
        valid_path << configPath.string();
        return true;
    }
    else {
        fs::path parentPath = currentPath.parent_path();
        if (!parentPath.empty() && parentPath != currentPath) {
            return FindAndLoadConfig(parentPath, configFileName, valid_path);
        }
    }
    return false;
}

bool TrkConfig::LoadConfig(TrkCliOptionResults* Results)
{
    TrkString valid_path;
    if (FindAndLoadConfig(fs::current_path(), ".trkconfig", valid_path))
    {
        std::ifstream configFile(valid_path);
        if (!configFile.is_open())
        {
            return false;
        }

        std::string l;
        while (std::getline(configFile, l))
        {
            TrkString line(l.c_str());
            size_t delimiterPos = line.find("=");
            if (delimiterPos != std::string::npos)
            {
                TrkString key = line.substr(0, delimiterPos);
                TrkString value = line.substr(delimiterPos + 1);

                TrkCliClientOptionResults* clientResults = dynamic_cast<TrkCliClientOptionResults*>(Results);
                if (clientResults)
                {
                    if (key == TRK_CONFIG_CLIENT_PASSWORD)
                    {
                        // @TODO: we will pass this as encrypted. 
                        clientResults->password = value;
                    }
                    else if (key == TRK_CONFIG_CLIENT_SERVERURL)
                    {
                        clientResults->server_url = value;
                    }
                    else if (key == TRK_CONFIG_CLIENT_TRUST)
                    {
                        value.ToLower();
                        
                        if (value == "true" || value == "yes" || value == "1")
                        {
                            clientResults->trust = true;
                        }
                        else if (value == "false" || value == "no" || value == "0")
                        {
                            clientResults->trust = false;
                        }
                    }
                    else if (key == TRK_CONFIG_CLIENT_USER)
                    {
                        clientResults->username = value;
                    }
                }
            }
        }

        configFile.close();
    }

    TrkCliClientOptionResults* clientResults = dynamic_cast<TrkCliClientOptionResults*>(Results);
    if (clientResults)
    {
        if (clientResults->server_url == "")
        {
            if (std::getenv(TRK_ENV_CLIENT_SERVERURL) != nullptr)
            {
                clientResults->server_url = std::getenv(TRK_ENV_CLIENT_SERVERURL);
            }
            else
            {
                clientResults->server_url = "localhost:5566";
            }
        }
        if (clientResults->username == "")
        {
            if (std::getenv(TRK_ENV_CLIENT_USER) != nullptr)
            {
                clientResults->username = std::getenv(TRK_ENV_CLIENT_USER);
            }
            else
            {
#ifdef _WIN32
                TCHAR infoBuf[256];
                DWORD bufCharCount = sizeof(infoBuf);

                if (GetComputerName(infoBuf, &bufCharCount))
                {
                    clientResults->username = TrkString(infoBuf);
                }
                else
                {
                    clientResults->username = "CLIENTUNKNOWN";
                }
#elif __unix__
                char hostname[HOST_NAME_MAX];
                gethostname(hostname, HOST_NAME_MAX);
                clientResults->username = TrkString(hostname);
#endif
            }
        }
    }

    return true;
}

bool TrkConfig::WriteConfig(const TrkString FilePath, const TrkString key, const TrkString value)
{
    fs::path fsPath((const char*)FilePath);

    if (fs::exists(fsPath) && fs::is_regular_file(fsPath) && fsPath.filename() == ".trkconfig")
    {
        fs::path tempFolderPath = fs::temp_directory_path() / "Tintirek";
        
        if (!fs::exists(tempFolderPath)) {
            if (!fs::create_directories(tempFolderPath)) {
                return false;
            }
        }

        TrkString tempFilePath = (tempFolderPath / fsPath.filename()).string().c_str();

        if (fs::exists(std::string(tempFilePath))) {
            std::ofstream tempFile(tempFilePath, std::ios::trunc);
            if (!tempFile.is_open()) {
                return false;
            }
            tempFile.close();
        }

        std::ifstream configFileIn(FilePath);
        std::ofstream configFileOut(tempFilePath);

        if (configFileIn.is_open() && configFileOut.is_open())
        {
            bool found = false;
            std::string l;
            while (std::getline(configFileIn, l))
            {
                TrkString line(l.c_str());
                size_t delimiterPos = line.find("=");
                if (delimiterPos != TrkString::npos)
                {
                    TrkString currentKey = line.substr(0, delimiterPos);
                    if (currentKey == key)
                    {
                        found = true;
                        continue;
                    }
                }
                configFileOut << line << "\n";
            }

            configFileIn.close();
            configFileOut.close();

            fs::remove(fsPath);
            fs::rename("temp_config.tmp", fsPath);

            std::ofstream configFileAppend(FilePath, std::ios::app);
            if (configFileAppend.is_open()) {
                configFileAppend << key << "=" << value << "\n";
                configFileAppend.close();
                return true;
            }
        }
    }

    return false;
}
