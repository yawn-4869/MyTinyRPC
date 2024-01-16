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

    m_log_level = log_level_str;
}

}