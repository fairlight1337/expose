#include <XHelper.h>


XHelper::XHelper() {
  m_dspDisplay = XOpenDisplay(0);
  m_winRoot = XDefaultRootWindow(m_dspDisplay);
  
  XInitThreads();
}

XHelper::~XHelper() {
  XCloseDisplay(m_dspDisplay);
}

KeyCode XHelper::X11KeyCode(int nAsciiKeyCode) {
  unsigned int unUse = 0x0;
  
  switch(nAsciiKeyCode) {
  case 37: // left
    unUse = XK_Left;
    break;
      
  case 38: // up
    unUse = XK_Up;
    break;
      
  case 39: // right
    unUse = XK_Right;
    break;
    
  case 40: // down
    unUse = XK_Down;
    break;
    
  case 13: // enter
    unUse = XK_Return;
    break;
    
  case 27: // escape
    unUse = XK_Escape;
    break;
    
  default: // everything else
    unUse = nAsciiKeyCode;
    break;
  }
  
  return XKeysymToKeycode(this->display(), unUse);
}

Display* XHelper::display() {
  return m_dspDisplay;
}

Window XHelper::rootWindow() {
  return m_winRoot;
}

Window XHelper::subWindow(Window winWindow, bool& bSuccess) {
  Window winRoot, winParent;
  Window* winChildren;
  unsigned int nNumChildren;
  
  XQueryTree(this->display(), winWindow, &winRoot, &winParent, &winChildren, &nNumChildren);
  
  Window winReturn = winWindow;
  
  if(winChildren) {
    winReturn = winChildren[0];
    XFree(winChildren);
    bSuccess = true;
  } else {
    bSuccess = false;
  }
  
  return winReturn;
}

Window XHelper::subWindow(Window winWindow) {
  bool bSuccess;
  
  return this->subWindow(winWindow, bSuccess);
}

void XHelper::printWindowStructure(Window winWindow, int nLevel) {
  std::string strIndent = "";
  for(int nI = 0; nI < nLevel; nI++) {
    strIndent += "  ";
  }
  
  std::cout << strIndent << "Window" << std::endl;
  
  Window winRoot, winParent;
  Window* winChildren;
  unsigned int nNumChildren;
  
  XQueryTree(this->display(), winWindow, &winRoot, &winParent, &winChildren, &nNumChildren);
  
  for(int nI = 0; nI < nNumChildren; nI++) {
    this->printWindowStructure(winChildren[nI], nLevel + 1);
  }
  
  if(winChildren) {
    XFree(winChildren);
  }
}

void XHelper::xflush() {
  XFlush(this->display());
}

Window XHelper::getNextMappedWindow() {
  Window winReturn;
  
  XSetWindowAttributes attributes;
  attributes.event_mask = SubstructureNotifyMask | StructureNotifyMask;
  XChangeWindowAttributes(this->display(), this->rootWindow(), CWEventMask, &attributes);
  
  while(true) {
    XEvent evtEvent;
    XNextEvent(this->display(), &evtEvent);
    
    if(evtEvent.type == MapNotify) {
      winReturn = evtEvent.xmap.window;
      break;
    }
  }
  
  return winReturn;
}

WindowCapture XHelper::captureWindow(Window winCapture) {
  WindowCapture wc;
  winCapture = this->subWindow(winCapture);
  
  XWindowAttributes gwa;
  XGetWindowAttributes(this->display(), winCapture, &gwa);
  
  wc.nWidth = gwa.width;
  wc.nHeight = gwa.height;
  wc.ucData = new unsigned char[wc.nWidth * wc.nHeight * 3];
  
  XImage* ximgImage = XGetImage(this->display(), winCapture, 0, 0, wc.nWidth, wc.nHeight, AllPlanes, ZPixmap);
  
  unsigned long ulMaskRed = ximgImage->red_mask;
  unsigned long ulMaskGreen = ximgImage->green_mask;
  unsigned long ulMaskBlue = ximgImage->blue_mask;
  
  for(int nX = 0; nX < wc.nWidth; nX++) {
    for(int nY = 0; nY < wc.nHeight; nY++) {
      unsigned long ulPixel = XGetPixel(ximgImage, nX, nY);
      
      unsigned char ucRed = (ulPixel & ulMaskRed) >> 16;
      unsigned char ucGreen = (ulPixel & ulMaskGreen) >> 8;
      unsigned char ucBlue = (ulPixel & ulMaskBlue);
      
      wc.ucData[(nX + nY * wc.nWidth) * 3 + 0] = ucRed;
      wc.ucData[(nX + nY * wc.nWidth) * 3 + 1] = ucGreen;
      wc.ucData[(nX + nY * wc.nWidth) * 3 + 2] = ucBlue;
    }
  }
  
  return wc;
}

std::vector<Window> XHelper::findXWindowForPID(Window winParent, pid_t pidFind) {
  std::vector<Window> vecWindowsMatching;
  
  Atom atomPID = XInternAtom(this->display(), "_NET_WM_PID", True);
  
  Atom type;
  int format;
  unsigned long nItems;
  unsigned long bytesAfter;
  unsigned char* propPID = 0;
  
  if(Success == XGetWindowProperty(this->display(), winParent, atomPID, 0, 1, False, XA_CARDINAL, &type, &format, &nItems, &bytesAfter, &propPID)) {
    if(propPID != 0) {
      if(pidFind == *((unsigned long*)propPID)) {
	vecWindowsMatching.push_back(winParent);
      }
      
      XFree(propPID);
    }
  }
  
  Window wRoot;
  Window wParent;
  Window* wChild;
  unsigned nChildren;
  
  if(0 != XQueryTree(this->display(), winParent, &wRoot, &wParent, &wChild, &nChildren)) {
    for(int nI = 0; nI < nChildren; nI++) {
      std::vector<Window> vecTemp = findXWindowForPID(wChild[nI], pidFind);
      
      for(Window winTemp : vecTemp) {
	vecWindowsMatching.push_back(winTemp);
      }
    }
  }
  
  return vecWindowsMatching;
}

bool XHelper::sendEvent(Window winWindow, XEvent* evtEvent) {
  return XSendEvent(this->display(), winWindow, False, 0xfff, evtEvent) != 0;
}

bool XHelper::sendAdvancedEvent(Window winWindow, XEvent* evtEvent) {
  bool bReturnvalue = false;
  
  if(this->sendEvent(winWindow, evtEvent)) {
    this->xflush();
    
    if(this->sendEvent(this->subWindow(winWindow), evtEvent)) {
      this->xflush();
      
      bReturnvalue = true;
    }
  }
  
  return bReturnvalue;
}

XEvent XHelper::createEvent(int nType) {
  XEvent evtEvent;
  memset(&evtEvent, 0x00, sizeof(evtEvent));
  
  evtEvent.type = nType;
  
  return evtEvent;
}
