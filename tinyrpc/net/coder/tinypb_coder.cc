#include <string.h>
#include <arpa/inet.h>
#include "tinyrpc/net/coder/tinypb_coder.h"
#include "tinyrpc/net/coder/tinypb_protocol.h"
#include "tinyrpc/common/util.h"
#include "tinyrpc/common/log.h"

namespace MyTinyRPC {

void TinyPBCoder::encode(const std::vector<AbstractProtocol::s_ptr>& in_message, TcpBuffer::s_ptr out_buffer) {
    for(auto& message : in_message) {
        std::shared_ptr<TinyPBProtocol> msg = std::dynamic_pointer_cast<TinyPBProtocol>(message);
        int len = 0;
        const char* buf = encodeTinyPB(msg, len);
        if(buf != NULL && len != 0) {
            out_buffer->writeToBuffer(buf, len);
        }

        if(buf) {
            free((void*)buf);
            buf = NULL;
        }
    }
}

void TinyPBCoder::decode(std::vector<AbstractProtocol::s_ptr>& out_message, const TcpBuffer::s_ptr in_buffer) {
    std::vector<char> tmp;
    int len = in_buffer->readAble();
    in_buffer->readFromBuffer(tmp, len);
    int i = 0;
    int end_index = -1;
    // 遍历buffer, 找到PB_START, 解析整包长度, 得到结束符位置, 判断是否为PB_END
    while(i < len) {
        int start_index = i;
        bool parse_success = false;

        int32_t pkg_len = 0;
        for(i = end_index + 1; i < len; ++i) {
            if(tmp[i] == TinyPBProtocol::PB_START) {
                // 读取4个字节获得pkg_len
                // 由于是网络字节序, 需要转化为主机字节序
                if(i + 1 < len) {
                    DEBUGLOG("index i[%d], len[%d]", i, len);
                    pkg_len = getInt32FromNetByte(&tmp[i+1]);
                    DEBUGLOG("get pkg_llen = %d", pkg_len);
                    
                    int j = i + pkg_len - 1; // 结束码索引
                    if(j >= len) {
                        // 找新的开始码
                        continue;
                    }
                    if(tmp[j] == TinyPBProtocol::PB_END) {
                        // 找到结束码
                        start_index = i;
                        end_index = j;
                        parse_success = true;
                        break;
                    }
                    
                }
            }
        }

        if(i >= len) {
            // buffer的readAble遍历结束, 解析完成
            DEBUGLOG("decode end, read all buffer data");
            return;
        }

        if(parse_success) {
            // 找到请求起点和终点, 开始解析报文
            std::shared_ptr<TinyPBProtocol> message = std::make_shared<TinyPBProtocol>();
            message->m_pk_len = pkg_len;
            
            // id长度
            int msg_id_len_index = start_index + sizeof(char) + sizeof(message->m_pk_len);
            if(msg_id_len_index > end_index) {
                message->parse_success = false;
                ERRORLOG("parse error, msg_id_len_index[%d] >= end_index[%d]", msg_id_len_index, end_index);
                continue;
            }

            message->m_req_id_len = getInt32FromNetByte(&tmp[msg_id_len_index]);

            // id
            int msg_id_index = msg_id_len_index + sizeof(message->m_req_id_len);
            char msg_id[128] = {0};
            memcpy(msg_id, &tmp[msg_id_index], message->m_req_id_len);
            message->m_req_id = std::string(msg_id);
            DEBUGLOG("parse req_id=%s", message->m_req_id.c_str());

            // 方法名长度
            int method_name_len_index = msg_id_index + message->m_req_id_len;
            if(method_name_len_index >= end_index) {
                message->parse_success = false;
                ERRORLOG("parse error, method_name_len_index[%d] >= end_index[%d]", method_name_len_index, end_index);
                continue;
            }
            message->m_method_name_len = getInt32FromNetByte(&tmp[method_name_len_index]);

            // 方法名
            int method_name_index = method_name_len_index + sizeof(message->m_method_name_len);
            char method_name[512] = {0};
            memcpy(method_name, &tmp[method_name_index], message->m_method_name_len);
            message->m_method_name = std::string(method_name);
            DEBUGLOG("parse method_name=%s", message->m_method_name.c_str());

            // 错误码
            int error_code_index = method_name_index + message->m_method_name_len;
            if(error_code_index >= end_index) {
                message->parse_success = false;
                ERRORLOG("parse error, error_code_index[%d] >= end_index[%d]", error_code_index, end_index);
                continue;
            }
            message->m_error_code = getInt32FromNetByte(&tmp[error_code_index]);

            // 错误信息长度
            int error_info_len_index = error_code_index + sizeof(message->m_error_code);
            if (error_info_len_index >= end_index) {
                message->parse_success = false;
                ERRORLOG("parse error, error_info_len_index[%d] >= end_index[%d]", error_info_len_index, end_index);
                continue;
            }
            message->m_error_msg_len = getInt32FromNetByte(&tmp[error_info_len_index]);

            // 错误信息
            int error_info_index = error_info_len_index + sizeof(message->m_error_msg_len);
            char error_info[512] = {0};
            memcpy(error_info, &tmp[error_info_index], message->m_error_msg_len);
            message->m_error_msg = std::string(error_info);
            DEBUGLOG("parse error_info=%s", message->m_error_msg.c_str());

            int pb_data_len = message->m_pk_len - message->m_req_id_len - message->m_method_name_len - message->m_error_msg_len - 2 - 24;
            int pb_data_index = error_info_index + message->m_error_msg_len;
            message->m_pb_data = std::string(&tmp[pb_data_index], pb_data_len);
            DEBUGLOG("parse pb_data=%s", message->m_pb_data.c_str());

            // TODO: 校验和
            message->parse_success = true;
            
            out_message.push_back(message);
        }
    }
}

const char* TinyPBCoder::encodeTinyPB(TinyPBProtocol::s_ptr message, int& len) {
    if(message->m_req_id.empty()) {
        message->m_req_id = "12345678";
    }

    // 计算总包长度, 包的length字段有可能未被赋值
    int pkg_len = 2 + 24 + message->m_req_id.length() + message->m_method_name.length() + 
    message->m_error_msg.length() + message->m_pb_data.length();

    char* buf = reinterpret_cast<char*>(malloc(pkg_len));
    char* tmp = buf;

    *tmp = TinyPBProtocol::PB_START;
    tmp++;

    int32_t pkg_len_net = htonl(pkg_len); // 转网络字节序
    memcpy(tmp, &pkg_len_net, sizeof(pkg_len_net));
    tmp += sizeof(pkg_len_net);

    int32_t msg_id_len = message->m_req_id.length();
    int32_t msg_id_len_net = htonl(msg_id_len);
    memcpy(tmp, &msg_id_len_net, sizeof(msg_id_len_net));
    tmp += sizeof(msg_id_len_net);

    if(!message->m_req_id.empty()) {
        memcpy(tmp, &(message->m_req_id[0]), msg_id_len);
        tmp += msg_id_len;
    }

    int32_t method_name_len = message->m_method_name.length();
    int32_t method_name_len_net = htonl(method_name_len);
    memcpy(tmp, &method_name_len_net, sizeof(method_name_len_net));
    tmp += sizeof(method_name_len_net);

    if(!message->m_method_name.empty()) {
        memcpy(tmp, &(message->m_method_name[0]), method_name_len);
        tmp += method_name_len;
    }

    int32_t error_code = message->m_error_code;
    int32_t error_code_net = htonl(error_code);
    memcpy(tmp, &error_code_net, sizeof(error_code_net));
    tmp += sizeof(error_code_net);
    
    int32_t error_info_len = message->m_error_msg.length();
    int32_t error_info_len_net = htonl(error_info_len);
    memcpy(tmp, &error_info_len_net, sizeof(error_info_len_net));
    tmp += sizeof(error_info_len_net);

    if(!message->m_error_msg.empty()) {
        memcpy(tmp, &(message->m_error_msg[0]), error_info_len);
        tmp += error_info_len;
    }

    if(!message->m_pb_data.empty()) {
        memcpy(tmp, &(message->m_pb_data[0]), message->m_pb_data.length());
        tmp += message->m_pb_data.length();
    }

    int32_t check_sum_net = htonl(1);
    memcpy(tmp, &check_sum_net, sizeof(check_sum_net));
    tmp += sizeof(check_sum_net);

    message->m_pk_len = pkg_len;
    message->m_req_id_len = msg_id_len;
    message->m_method_name_len = method_name_len;
    message->m_error_msg_len = error_info_len;
    message->parse_success = true;
    len = pkg_len;

    *tmp = TinyPBProtocol::PB_END;

    DEBUGLOG("encode message[%s] success",message->m_req_id.c_str());

    return buf;
}

}