#include <MHDHelper.h>


MHDHelper::MHDHelper() {
}

MHDHelper::~MHDHelper() {
  MHD_stop_daemon(m_mhddDaemon);
}

struct MHD_Daemon* MHDHelper::daemonHandle() {
  return m_mhddDaemon;
}

void MHDHelper::setDaemonHandle(struct MHD_Daemon* mhddDaemon) {
  m_mhddDaemon = mhddDaemon;
}

std::string MHDHelper::parameter(MHD_Connection* conConnection, std::string strKey) {
  return MHD_lookup_connection_value(conConnection, MHD_GET_ARGUMENT_KIND, strKey.c_str());
}

struct MHD_Response* MHDHelper::createResponse(std::string strContent) {
  return this->createResponse((void*)strContent.c_str(), strContent.length());
}

struct MHD_Response* MHDHelper::createResponse(void* vdData, int nSize) {
  return MHD_create_response_from_buffer(nSize, vdData, MHD_RESPMEM_PERSISTENT);
}
