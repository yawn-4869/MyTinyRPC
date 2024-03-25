#ifndef MYTINYRPC_NET_CODER_TINY_PB_PROTOCOL_H
#define MYTINYRPC_NET_CODER_TINY_PB_PROTOCOL_H

#include "tinyrpc/net/coder/abstract_protocol.h"

namespace MyTinyRPC {
struct TinyPBProtocol : public AbstractProtocol {
public:
    typedef std::shared_ptr<TinyPBProtocol> s_ptr;

public:
    static char PB_START;
    static char PB_END;

public:
    int32_t m_pk_len{ 0 }; // 包长度(包括开始码和结束码)
    int32_t m_req_id_len{ 0 }; // 请求id长度
    /*req_id 继承自父类*/
    int32_t m_method_name_len{ 0 }; // 方法名长度
    std::string m_method_name; // 方法名
    int32_t m_error_code{ 0 }; // 错误码
    int32_t m_error_msg_len{ 0 }; // 错误详细信息长度
    std::string m_error_msg; // 错误详细信息
    std::string m_pb_data; // 序列化后的实际数据
    int32_t m_check_sum; // 校验和
    bool parse_success{ false };
};
}

#endif