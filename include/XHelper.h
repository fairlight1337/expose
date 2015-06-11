#ifndef __XHELPER_H__
#define __XHELPER_H__


#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/X.h>
#include <X11/Xutil.h>

#include <string.h>

#include <iostream>
#include <vector>


typedef struct {
  int nWidth;
  int nHeight;
  unsigned char* ucData;
  int nSize;
  Window winXWindow;
} WindowCapture;


class XHelper {
 private:
  Display* m_dspDisplay;
  Window m_winRoot;
  
 protected:
 public:
  XHelper();
  ~XHelper();
  
  KeyCode X11KeyCode(int nAsciiKeyCode);
  
  Display* display();
  Window rootWindow();
  
  Window subWindow(Window winWindow, bool& bSuccess);
  Window subWindow(Window winWindow);
  
  void printWindowStructure(Window winWindow, int nLevel = 0);
  void xflush();
  
  Window getNextMappedWindow();
  WindowCapture captureWindow(Window winCapture);
  std::vector<Window> findXWindowForPID(Window winParent, pid_t pidFind);
  
  bool sendEvent(Window winWindow, XEvent* evtEvent);
  bool sendAdvancedEvent(Window winWindow, XEvent* evtEvent);
  XEvent createEvent(int nType);
};


#endif /* __XHELPER_H__ */
