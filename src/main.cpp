#include <cstdlib>
#include <iostream>
#include <string>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <list>
#include <turbojpeg.h>
#include <microhttpd.h>
#include <byteswap.h>
#include <mutex>
#include <vector>

#include <XHelper.h>


int* g_nArgc;
char*** g_carrArgv;

std::mutex mtxPicture;
std::vector<WindowCapture> vecWindowCaptures;

const int JPEG_QUALITY = 75;
const int COLOR_COMPONENTS = 3;

struct MHD_Daemon* m_mhddDaemon;

XHelper* xhlp;


static int httpRequestCallback(void* cls, struct MHD_Connection* connection, const char* url, const char* method, const char* version, const char* upload_data, size_t* upload_data_size, void** con_cls) {
  struct MHD_Response* response;
  int ret;
  
  if(std::string(url) == "/window.jpg") {
    std::string strIndex(MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "index"));
    
    int nIndex;
    sscanf(strIndex.c_str(), "%d", &nIndex);
    
    mtxPicture.lock();
    
    if(nIndex < vecWindowCaptures.size()) {
      WindowCapture wc = vecWindowCaptures[nIndex];
      
      if(wc.ucData) {
	response = MHD_create_response_from_buffer(wc.nSize, (void*)wc.ucData, MHD_RESPMEM_PERSISTENT);
      
	MHD_add_response_header(response, "Cache-Control", "no-store");
      } else {
	response = MHD_create_response_from_buffer(0, (void*)"", MHD_RESPMEM_PERSISTENT);
      }
    }
    
    mtxPicture.unlock();
  } else if(std::string(url) == "/mousemove") {
    mtxPicture.lock();
    
    std::string strX(MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "x"));
    std::string strY(MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "y"));
    std::string strIndex(MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "index"));
    
    int nX, nY, nIndex;
    sscanf(strX.c_str(), "%d", &nX);
    sscanf(strY.c_str(), "%d", &nY);
    sscanf(strIndex.c_str(), "%d", &nIndex);
    
    Window winXWindow = vecWindowCaptures[nIndex].winXWindow;
    
    XEvent evtEvent = xhlp->createEvent(MotionNotify);
    evtEvent.xmotion.x = nX;
    evtEvent.xmotion.y = nY;
    evtEvent.xmotion.same_screen = True;
    evtEvent.xmotion.subwindow = xhlp->rootWindow();
    evtEvent.xmotion.send_event = True;
    
    if(!xhlp->sendAdvancedEvent(winXWindow, &evtEvent)) {
      std::cerr << "Error" << std::endl;
    }
    
    response = MHD_create_response_from_buffer(6, (void*)"200 OK", MHD_RESPMEM_PERSISTENT);
    
    mtxPicture.unlock();
  } else if(std::string(url) == "/mouseclick") {
    mtxPicture.lock();
    
    std::string strIndex(MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "index"));
    
    int nIndex;
    sscanf(strIndex.c_str(), "%d", &nIndex);
    
    Window winXWindow = vecWindowCaptures[nIndex].winXWindow;
    
    XEvent evtEvent = xhlp->createEvent(ButtonPress);
    evtEvent.xbutton.button = 1;
    
    if(!xhlp->sendAdvancedEvent(winXWindow, &evtEvent)) {
      std::cerr << "Error" << std::endl;
    }
    
    usleep(10000);
    
    evtEvent.type = ButtonRelease;
    evtEvent.xbutton.state = 0x100;
    
    if(!xhlp->sendAdvancedEvent(winXWindow, &evtEvent)) {
      std::cerr << "Error" << std::endl;
    }
    
    response = MHD_create_response_from_buffer(6, (void*)"200 OK", MHD_RESPMEM_PERSISTENT);
    
    mtxPicture.unlock();
  } else if(std::string(url) == "/keydown") {
    mtxPicture.lock();
    
    std::string strIndex(MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "index"));
    std::string strKeyCode(MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "key"));
    
    int nIndex, nKeyCode;
    sscanf(strIndex.c_str(), "%d", &nIndex);
    sscanf(strKeyCode.c_str(), "%d", &nKeyCode);
    
    Window winXWindow = vecWindowCaptures[nIndex].winXWindow;
    
    XEvent evtEvent = xhlp->createEvent(KeyPress);
    evtEvent.xkey.keycode = xhlp->X11KeyCode(nKeyCode);
    
    if(!xhlp->sendAdvancedEvent(winXWindow, &evtEvent)) {
      std::cerr << "Error" << std::endl;
    }
    
    response = MHD_create_response_from_buffer(6, (void*)"200 OK", MHD_RESPMEM_PERSISTENT);
    
    mtxPicture.unlock();
  } else if(std::string(url) == "/keyup") {
     mtxPicture.lock();
    
    std::string strIndex(MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "index"));
    std::string strKeyCode(MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "key"));
    
    int nIndex, nKeyCode;
    sscanf(strIndex.c_str(), "%d", &nIndex);
    sscanf(strKeyCode.c_str(), "%d", &nKeyCode);
    
    Window winXWindow = vecWindowCaptures[nIndex].winXWindow;
    
    XEvent evtEvent = xhlp->createEvent(KeyRelease);
    evtEvent.xkey.keycode = xhlp->X11KeyCode(nKeyCode);
    
    if(!xhlp->sendAdvancedEvent(winXWindow, &evtEvent)) {
      std::cerr << "Error" << std::endl;
    }
    
    response = MHD_create_response_from_buffer(6, (void*)"200 OK", MHD_RESPMEM_PERSISTENT);
    
    mtxPicture.unlock();
  } else {
    const char* page = "\
<html onkeydown=\"keydown(event);\" onkeyup=\"keyup(event);\">\n\
  <head>\n\
    <script>\n\
      function mousemove(e) {\n\
        var image = document.getElementById(\"img\");\n\
        \n\
        xmlHttp = new XMLHttpRequest();\n\
        xmlHttp.open(\"GET\", \"mousemove?index=0&x=\" + (e.x - image.offsetLeft) + \"&y=\" + (e.y - image.offsetTop), false);\n\
        xmlHttp.send();\n\
      }\n\
      \n\
      function mouseclick(e) {\n\
        xmlHttp = new XMLHttpRequest();\n\
        xmlHttp.open(\"GET\", \"mouseclick?index=0\", false);\n\
        xmlHttp.send();\n\
      }\n\
      \n\
      function keydown(e) {\n\
        console.log(e);\n\
        xmlHttp = new XMLHttpRequest();\n\
        xmlHttp.open(\"GET\", \"keydown?index=0&key=\" + e.keyCode, false);\n\
        xmlHttp.send();\n\
      }\n\
      \n\
      function keyup(e) {\n\
        xmlHttp = new XMLHttpRequest();\n\
        xmlHttp.open(\"GET\", \"keyup?index=0&key=\" + e.keyCode, false);\n\
        xmlHttp.send();\n\
      }\n\
      \n\
      window.setInterval(function() {\n\
        var image = document.getElementById(\"img\");\n\
        image.src = image.src;\n\
      });\n\
    </script>\n\
  </head>\n\
  \n\
  <body>\n\
    <img onmousemove=\"mousemove(event);\" onclick=\"mouseclick(event);\" id=\"img\" src=\"window.jpg?index=0\">\n\
  </body>\n\
</html>";
    
    response = MHD_create_response_from_buffer(strlen(page), (void*)page, MHD_RESPMEM_PERSISTENT);
  }
  
  ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
  MHD_destroy_response(response);
  
  return ret;
}


int main(int argc, char** argv) {
  int nReturnvalue = EXIT_FAILURE;
  xhlp = new XHelper();
  
  if(argc == 1) {
    std::cerr << "Usage: " << argv[0] << " <command> [arguments]" << std::endl;
  } else {
    g_nArgc = (int*)mmap(NULL, sizeof *g_nArgc, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    *g_nArgc = argc;
    
    g_carrArgv = (char***)mmap(NULL, sizeof *g_carrArgv, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    *g_carrArgv = argv;
    
    pid_t pidFork = fork();
    
    if(pidFork == 0) {
      // Child
      std::cout << "Running: " << g_carrArgv[0][1] << std::endl;
      std::cout << (*g_nArgc - 1) << std::endl;
      char* carrArgv[*g_nArgc - 1];
      for(int nI = 2; nI < *g_nArgc; nI++) {
	std::cout << g_carrArgv[0][nI] << std::endl;
	carrArgv[nI - 2] = g_carrArgv[0][nI];
      }
      
      carrArgv[*g_nArgc - 1] = NULL;
      
      std::cout << "Parameters:";
      
      for(int nI = 0; nI < *g_nArgc - 2; nI++) {
	std::cout << " " << carrArgv[nI];
      }
      std::cout << std::endl;
      
      int nResult = execvp(g_carrArgv[0][1], carrArgv);
      
      if(nResult == -1) {
	std::cerr << "Error: " << strerror(errno) << std::endl;
      }
    } else {
      // Parent
      std::cout << "Got child with PID = " << pidFork << std::endl;
      
      std::vector<Window> vecWindowsMatching;
      vecWindowsMatching.push_back(xhlp->getNextMappedWindow());
      
      std::cout << "Found " << vecWindowsMatching.size() << " window(s)" << std::endl;
      vecWindowCaptures.resize(vecWindowsMatching.size());
      
      for(int nI = 0; nI < vecWindowCaptures.size(); nI++) {
	vecWindowCaptures[nI].ucData = NULL;
      }
      
      m_mhddDaemon = MHD_start_daemon(MHD_USE_SELECT_INTERNALLY, 8889, NULL, NULL, &httpRequestCallback, NULL, MHD_OPTION_END);
      
      // Endless loop, capturing the window
      while(true) {
	for(int nI = 0; nI < vecWindowsMatching.size(); nI++) {
	  Window winCapture = vecWindowsMatching[nI];
	  WindowCapture wc = xhlp->captureWindow(winCapture);
	  
	  unsigned char* compressedImage = NULL;
	  long unsigned int _jpegSize = 0;
	  
	  tjhandle _jpegCompressor = tjInitCompress();
	  tjCompress2(_jpegCompressor, wc.ucData, wc.nWidth, 0, wc.nHeight, TJPF_RGB, &compressedImage, &_jpegSize, TJSAMP_444, JPEG_QUALITY, TJFLAG_FASTDCT);
	  
	  tjDestroy(_jpegCompressor);
	  
	  mtxPicture.lock();
	  
	  vecWindowCaptures[nI].nSize = _jpegSize;
	  
	  if(vecWindowCaptures[nI].ucData) {
	    delete[] vecWindowCaptures[nI].ucData;
	  }
	  
	  vecWindowCaptures[nI].winXWindow = winCapture;
	  vecWindowCaptures[nI].ucData = new unsigned char[_jpegSize];
	  memcpy(vecWindowCaptures[nI].ucData, compressedImage, _jpegSize);
	  
	  mtxPicture.unlock();
	  
	  tjFree(compressedImage);
	  
	  delete[] wc.ucData;
	}
	
	usleep(10000);
      }
    }
    
    MHD_stop_daemon(m_mhddDaemon);
    
    munmap(g_nArgc, sizeof *g_nArgc);
    munmap(g_carrArgv, sizeof *g_carrArgv);
    
    nReturnvalue = EXIT_SUCCESS;
  }
  
  delete xhlp;
  
  return nReturnvalue;
}
