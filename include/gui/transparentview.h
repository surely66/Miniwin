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

#ifndef __NGLUI_TRANSPARENTVIEW_H__
#define __NGLUI_TRANSPARENTVIEW_H__

#include "view.h"

namespace nglui {

class TransparentView : public View {
 public:
  static constexpr const char* VIEW_NAME = "TransparentView";

 public:
  TransparentView();
  virtual ~TransparentView();
  
  virtual void onDraw(GraphContext& canvas) override;
  virtual void onResize(SIZE& size) override;
  virtual void invalidate(const RECT* bound) override;
  virtual void onAnimate() override;

  virtual bool onMousePress(Event& evt) override;
  virtual bool onMouseRelease(Event& evt) override;
  virtual bool onMouseMotion(Event& evt) override;
  virtual bool onKeyPress(Event& evt) override;
  virtual bool onKeyRelease(Event& evt) override;
  virtual bool onKeyChar(Event& evt) override;

 private:
  typedef View INHERITED;
  DISALLOW_COPY_AND_ASSIGN(TransparentView);
};

}  // namespace ui

#endif  // __NGLUI_TRANSPARENTVIEW_H__
