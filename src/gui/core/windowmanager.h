/*
 * Copyright (C) 2015 UI project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __NGLUI_WINDOWMANAGER_H__
#define __NGLUI_WINDOWMANAGER_H__

#include <view.h>
#include <window.h>
#include <vector>
#include <stdint.h>
#include <ngl_ir.h>
#include <queue>

namespace nglui {

typedef struct UIMSG_{
   std::weak_ptr<View>view;
   DWORD msgid;
   DWORD wParam;
   ULONG lParam;
   ULONG time;
   bool operator<(const UIMSG_&o)const{
      return time<o.time;
   }
   bool operator>(const UIMSG_&o)const{
      return time>o.time;
   }
}UIMSG;

class WindowManager {
 public:
  virtual ~WindowManager();

  virtual void onResize(int width, int height);
  virtual void onReposition(uint32_t x, uint32_t y);
  virtual void onBtnPress(uint32_t x, uint32_t , uint32_t modi);
  virtual void onBtnRelease(uint32_t x, uint32_t , uint32_t modi);
  virtual void onMotion(uint32_t x, uint32_t y, uint32_t modi);
  virtual void onKeyPress(uint32_t key);
  virtual void onKeyRelease(uint32_t key);
  virtual void onKeyChar(uint32_t key);
  virtual void drawWindows();
  static WindowManager* getInstance();
  void addWindow(Window*w);
  void sendMessage(Window*,DWORD msgid,DWORD wp,ULONG lp,DWORD delayedtime=0);
  void sendMessage(std::shared_ptr<View>,DWORD msgid,DWORD wp,ULONG lp,DWORD delayedtime=0);
  void runOnce();
  void run();
 private:
  WindowManager();
  void removeWindow(std::shared_ptr<Window>w);
  std::vector< std::shared_ptr<Window> > windows_;
  std::queue<UIMSG>msg_queue_;
  std::priority_queue<UIMSG,std::vector<UIMSG>,std::greater<UIMSG> >delayed_msgq_;
  void popMessage();
  static WindowManager* instance_;
  DISALLOW_COPY_AND_ASSIGN(WindowManager);
};

}  // namespace ui

#endif  // __NGLUI_WINDOWMANAGER_H__
