#ifndef __MHDHELPER_H__
#define __MHDHELPER_H__


#include <string>
#include <microhttpd.h>


class MHDHelper {
 private:
  struct MHD_Daemon* m_mhddDaemon;
  
 protected:
 public:
  MHDHelper();
  ~MHDHelper();
  
  struct MHD_Daemon* daemonHandle();
  void setDaemonHandle(struct MHD_Daemon* mhddDaemon);
  
  std::string parameter(MHD_Connection* conConnection, std::string strKey);
  struct MHD_Response* createResponse(std::string strContent);
  struct MHD_Response* createResponse(void* vdData, int nSize);
};


#endif /* __MHDHELPER_H__ */
