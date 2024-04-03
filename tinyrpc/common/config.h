#ifndef MYTINYRPC_COMMON_CONFIG_H
#define MYTINYRPC_COMMON_CONFIG_H

#include <string>
#include <tinyxml/tinyxml.h>

namespace MyTinyRPC {

class Config {
public:
    static Config* GetGloabalConfig();
    static void SetGlobalConfig(const char* xml_file);
    std::string getGlobalLogLevel() {
        return m_log_level;
    }
    
public:
    Config(const char* xml_file);
    Config();


public:
    std::string m_log_level;
    std::string m_log_file_name;
    std::string m_log_file_path;
    int m_log_max_file_size{ 0 };
    int m_async_log_interval{ 0 };

    int m_port{ 0 };
    int m_io_threads{ 0 };

    TiXmlDocument* m_xml_document;
};

}

#endif
