#ifndef MYTINYRPC_COMMON_RUN_TIME_H
#define MYTINYRPC_COMMON_RUN_TIME_H

#include <string>

namespace MyTinyRPC {
class RunTime {
 public:
  static RunTime* GetRunTime();


 public:
  std::string m_msgid;
  std::string m_method_name;
};

}

#endif