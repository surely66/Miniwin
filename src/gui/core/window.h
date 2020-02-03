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

#ifndef __NGLUI_WINDOW_H__
#define __NGLUI_WINDOW_H__
#include <ngl_types.h>
#include <groupview.h>

namespace nglui {
class Window : public GroupView {
friend class WindowManager;
public:
  Window(int x,int y,int w,int h);
  virtual ~Window();
  virtual void show();
  virtual void hide();
  virtual GraphContext*getCanvas()override;
  virtual View& setPos(int x,int y)override;
  virtual View& setSize(int cx,int cy)override;
  virtual bool onKeyRelease(KeyEvent& evt) override;
  virtual void sendMessage(DWORD msgid,DWORD wParam,ULONG lParam,DWORD delayedtime=0)override;
  virtual void sendMessage(std::shared_ptr<View>w,DWORD msgid,DWORD wParam,ULONG lParam,DWORD delayedtime=0)override;
 protected:
  virtual void draw(bool flip=true);
  GraphContext*canvas;
  typedef GroupView INHERITED;
  DISALLOW_COPY_AND_ASSIGN(Window);
};
void closeWindow(Window*w);

}  // namespace nglui

#endif  // UI_LIBUI_WINDOW_H_
