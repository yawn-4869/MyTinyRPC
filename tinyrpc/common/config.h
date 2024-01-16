#ifndef MYTINYRPC_COMMON_CONFIG_H
#define MYTINYRPC_COMMON_CONFIG_H

#include <string>

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


private:
    std::string m_log_level;
};

}

#endif
