#ifndef MYTINYRPC_COMMON_RUN_TIME_H
#define MYTINYRPC_COMMON_RUN_TIME_H

#include <string>
#include "tinyrpc/net/rpc/rpc_interface.h"

namespace MyTinyRPC {
class RunTime {
public:
	RpcInterface* getRpcInterface();
public:
	static RunTime* GetRunTime();


public:
	std::string m_msgid;
	std::string m_method_name;
	RpcInterface* m_rpc_interface {NULL};
};

}

#endif