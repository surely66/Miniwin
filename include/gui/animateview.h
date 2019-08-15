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

#ifndef __NGLUI_ANIMATEVIEW_H__
#define __NGLUI_ANIMATEVIEW_H__

#include "view.h"

namespace nglui {

class AnimateView : public View {
 public:
  static constexpr const char* VIEW_NAME = "AnimateView";

 public:
  explicit AnimateView(int w,int h);
  virtual ~AnimateView();

  // Painting events
  //virtual void OnPreDraw() override;
  //virtual void OnPostDraw() override;

  // Animation
  static void* AnimatorMain(void* arg);

  virtual void onAnimate() override;
  virtual bool onAnimateStart();
  virtual AnimState onAnimateProgress();
  virtual bool onAnimateCancelStart();
  virtual AnimState onAnimateCancelProgress();
  virtual void onAnimateEnd();

 protected:
  //Mutex anim_lock_;
  bool anim_end_;

 private:
  typedef View INHERITED;
  DISALLOW_COPY_AND_ASSIGN(AnimateView);
};

}  // namespace ui

#endif  // __NGLBUI_ANIMATEVIEW_H__
