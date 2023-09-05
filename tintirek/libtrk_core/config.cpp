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
#elif __linux__
#include <string.h>
#include <algorithm>
#endif


#include "trk_config.h"

namespace fs = std::filesystem;


/* Recursive config finder and loader */
static bool FindAndLoadConfig(const fs::path& currentPath, const char* configFileName, const char*& valid_path)
{
    fs::path configPath = currentPath / configFileName;
    if (fs::exists(configPath)) {
        valid_path = strdup(configPath.string().c_str());
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
    const char* valid_path;
    if (FindAndLoadConfig(fs::current_path(), ".trkconfig", valid_path))
    {
        std::ifstream configFile(valid_path);
        if (!configFile.is_open())
        {
            return false;
        }

        std::string line;
        while (std::getline(configFile, line))
        {
            size_t delimiterPos = line.find('=');
            if (delimiterPos != std::string::npos)
            {
                std::string key = line.substr(0, delimiterPos);
                std::string value = line.substr(delimiterPos + 1);

                TrkCliClientOptionResults* clientResults = dynamic_cast<TrkCliClientOptionResults*>(Results);
                if (clientResults)
                {
                    if (key == TRK_CONFIG_CLIENT_PASSWORD)
                    {
                        // @TODO: we will pass this as encrypted. 
                        clientResults->password = strdup(value.c_str());
                    }
                    else if (key == TRK_CONFIG_CLIENT_PATH)
                    {
                        clientResults->workspace_path = strdup(value.c_str());
                    }
                    else if (key == TRK_CONFIG_CLIENT_SERVERURL)
                    {
                        clientResults->server_url = strdup(value.c_str());
                    }
                    else if (key == TRK_CONFIG_CLIENT_TRUST)
                    {
                        std::transform(value.begin(), value.end(), value.begin(), ::tolower);
                        
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
                        clientResults->username = strdup(value.c_str());
                    }
                }
            }
        }

        configFile.close();
        delete valid_path;
    }

    TrkCliClientOptionResults* clientResults = dynamic_cast<TrkCliClientOptionResults*>(Results);
    if (clientResults)
    {
        if (clientResults->workspace_path == nullptr)
        {
            clientResults->workspace_path = std::getenv(TRK_ENV_CLIENT_PATH);
        }
        if (clientResults->server_url == nullptr)
        {
            clientResults->server_url = std::getenv(TRK_ENV_CLIENT_SERVERURL);

            if (clientResults->server_url == nullptr)
            {
                clientResults->server_url = "localhost:5566";
            }
        }
        if (clientResults->username == nullptr)
        {
            clientResults->username = std::getenv(TRK_ENV_CLIENT_USER);

            if (clientResults->username == nullptr)
            {
#ifdef _WIN32
                TCHAR infoBuf[256];
                DWORD bufCharCount = sizeof(infoBuf);

                if (GetComputerName(infoBuf, &bufCharCount))
                {
                    clientResults->username = strdup(infoBuf);
                }
                else
                {
                    clientResults->username = "CLIENTUNKNOWN";
                }
#elif __linux__
                char hostname[HOST_NAME_MAX];
                gethostname(hostname, HOST_NAME_MAX);
                clientResults->username = strdup(hostname);
#endif
            }
        }
    }

    return true;
}

bool TrkConfig::WriteConfig(const char* FilePath, const char* key, const char* value)
{
    fs::path fsPath(FilePath);

    if (fs::exists(fsPath) && fs::is_regular_file(fsPath) && fsPath.filename() == ".trkconfig")
    {
        fs::path tempFolderPath = fs::temp_directory_path() / "Tintirek";
        
        if (!fs::exists(tempFolderPath)) {
            if (!fs::create_directories(tempFolderPath)) {
                return false;
            }
        }

        std::string tempFilePath = (tempFolderPath / fsPath.filename()).string();

        if (fs::exists(tempFilePath)) {
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
            std::string line;
            while (std::getline(configFileIn, line))
            {
                size_t delimiterPos = line.find('=');
                if (delimiterPos != std::string::npos)
                {
                    std::string currentKey = line.substr(0, delimiterPos);
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
