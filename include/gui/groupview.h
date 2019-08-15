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

#ifndef __NGLUI_GROUPVIEW_H__
#define __NGLUI_GROUPVIEW_H__

#include "view.h"

namespace nglui {

class GroupView : public View {
 public:
  static constexpr const char* VIEW_NAME = "GroupView";

 public:
  GroupView(int w,int h);
  GroupView(int x,int y,int w,int h);
  virtual ~GroupView();
  
  virtual void onDraw(GraphContext& canvas) override;
  virtual void onResize(SIZE& size) override;
  virtual void onAnimate() override;

  virtual bool onMousePress(Event& evt) override;
  virtual bool onMouseRelease(Event& evt) override;
  virtual bool onMouseMotion(Event& evt) override;
  virtual bool onKeyPress(KeyEvent& evt) override;
  virtual bool onKeyRelease(KeyEvent& evt) override;
  virtual bool onKeyChar(KeyEvent& evt) override;
  virtual const SIZE&getPreferSize()override;
  virtual bool isDirty();
protected:
  View*getFocused();
  virtual View* getNextFocus(View*cv,int key);
 private:
  typedef View INHERITED;
  DISALLOW_COPY_AND_ASSIGN(GroupView);
};

}  // namespace ui

#endif  // __NGLUI_GROUPVIEW_H__
