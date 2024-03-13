#ifndef MYTINYRPC__NET_STRING_CODER_H
#define MYTINYRPC__NET_STRING_CODER_H

#include "tinyrpc/net/abstract_coder.h"

/*简单的string解码测试*/
namespace MyTinyRPC {
class StringProtocol : public AbstractProtocol {
public: 
    StringProtocol() {

    }

    void setInfo(std::string str) {
        info = str;
    }

    std::string getInfo() {
        return info;
    }

private:
    std::string info;

};

class StringCoder : public AbstractCoder {

// 将message对象转化为字节流，写入到buffer
void encode(const std::vector<AbstractProtocol*>& messages, TcpBuffer::s_ptr out_buffer) {
    for(auto message : messages) {
        StringProtocol* msg = dynamic_cast<StringProtocol*>(message);
        out_buffer->writeToBuffer(msg->getInfo().c_str(), msg->getInfo().length());
    }
}
// 将buffer中的字节流转化为message对象 
void decode(std::vector<AbstractProtocol*>& out_messages, const TcpBuffer::s_ptr buffer) {
    StringProtocol* msg = new StringProtocol();
    std::vector<char> tmp;
    buffer->readFromBuffer(tmp, buffer->readAble());
    std::string str;
    for(char& c : tmp) {
        str += c;
    }

    msg->setInfo(str);
    out_messages.push_back(msg);
}

};
}

#endif