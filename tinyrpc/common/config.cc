#include "config.h"
#include <tinyxml/tinyxml.h>

#define READ_XML_NODE(name, parent) \
TiXmlElement* name##_node = parent->FirstChildElement(); \
if(!name##_node) { \
    printf("Start tinyrpc server error, failed to read node [%s]\n", "#name"); \
    exit(0); \
} \

#define READ_STR_FROM_XML_NODE(name, parent) \
TiXmlElement* name##_node = parent->FirstChildElement(); \
if(!name##_node || !name##_node->GetText()) { \
    printf("Start tinyrpc server error, failed to read node [%s]\n", "#name"); \
    exit(0); \
} \
std::string name##_str = std::string(name##_node->GetText()); \

namespace MyTinyRPC {

static Config* g_config = NULL;

Config* Config::GetGloabalConfig() {
    return g_config;
}

void Config::SetGlobalConfig(const char* xml_file) {
    if(!g_config) {
        g_config = new Config(xml_file);
    }
}

Config::Config(const char* xml_file) {
    TiXmlDocument* xml_document = new TiXmlDocument();
    bool ret = xml_document->LoadFile(xml_file);
    if(!ret) {
        printf("Start tinyrpc server error, failed to read config file %s, error info[%s]\n", xml_file, xml_document->ErrorDesc());
        exit(0);
    }

    READ_XML_NODE(root, xml_document);
    READ_XML_NODE(log, root_node);

    READ_STR_FROM_XML_NODE(log_level, log_node);
    READ_STR_FROM_XML_NODE(log_file_name, log_node);
    READ_STR_FROM_XML_NODE(log_file_path, log_node);
    READ_STR_FROM_XML_NODE(log_max_file_size, log_node);
    READ_STR_FROM_XML_NODE(async_log_interval, log_node);

    m_log_level = log_level_str;
    m_log_file_name = log_file_name_str;
    m_log_file_path = log_file_path_str;
    m_log_max_file_size = std::atoi(log_max_file_size_str.c_str());
    m_async_log_interval = std::atoi(async_log_interval_str.c_str());

    printf("LOG -- CONFIG LEVEL[%s], FILE_NAME[%s],FILE_PATH[%s] MAX_FILE_SIZE[%d B], SYNC_INTEVAL[%d ms]\n", 
    m_log_level.c_str(), m_log_file_name.c_str(), m_log_file_path.c_str(), m_log_max_file_size, m_async_log_interval);
}

}