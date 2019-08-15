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

#include "animateview.h"
#include "window.h"
#include <ngl_os.h>

namespace nglui {

AnimateView::AnimateView(int w,int h)
  : INHERITED(w,h) {
  setAnimateState(AnimState::ANIM_INITIAL);
}

AnimateView::~AnimateView() {
}

/*void AnimateView::onPreDraw() {
    anim_lock_.ExclusiveLock();
}

void AnimateView::onPostDraw() {
    anim_lock_.ExclusiveUnlock();
}*/

void* AnimateView::AnimatorMain(void* arg) {
  Window* window = reinterpret_cast<Window*>(arg);
  for (;;) {
    /*for (auto child : window->children_) {
      child->onAnimate();
    }*/
    nglSleep(10);
  }
  return nullptr;
}

void AnimateView::onAnimate() {
  for (auto it : children_) {
    // If its Animate State if NONE, then it means that it's not
    // an AnimateView, simply ignore this child
    if (it->getAnimateState() == AnimState::ANIM_NONE) {
      continue;
    }

    // Case this child from View* to AnimateView*
    AnimateView* child = nullptr;//reinterpret_cast<AnimateView*>(it);

    // Standard animate state machine
    if (child->getAnimateState() == AnimState::ANIM_INITIAL) {
      // If it's in INITIAL state, call OnAnimateStart() and then
      // if succeed, turn it to PROGRESS state.
      anim_end_ = false;
      if (child->onAnimateStart() == false) {
        // If not succeed, turn it to END state and prevent OnAnimateEnd()
        // from being called.
        anim_end_ = true;
        setAnimateState(AnimState::ANIM_END);
      } else {
        setAnimateState(AnimState::ANIM_PROGRESS);
      }
    }
    // If it's in PROGRESS state, continue to call OnAnimateProgress()
    // until it reaches a new state
    if (child->getAnimateState() == AnimState::ANIM_PROGRESS) {
      child->onAnimateProgress();
    }
    // If it's in CANCAL state, call the OnAnimateCancelStart() first,
    // then turn it to CANCEL_PROGRESS state.
    if (child->getAnimateState() == AnimState::ANIM_CANCEL) {
      if (child->onAnimateCancelStart() == false) {
        // If not succeed, turn it to END state and prevent OnAnimateEnd()
        // from being called.
        anim_end_ = true;
        setAnimateState(AnimState::ANIM_END);
      } else {
        // If succeed, turn it to CANCEL_PROGRESS state
        setAnimateState(AnimState::ANIM_CANCEL_PROGRESS);
      }
    }
    // If it's in CANCEL_PROGRESS state, continue to call the
    // OnAnimateCancelProgress() until it reaches a new state
    if (child->getAnimateState() == AnimState::ANIM_CANCEL_PROGRESS) {
      child->onAnimateCancelProgress();
    }
    // If it's first time in END state, call the OnAnimateEnd()
    if (anim_end_ == false && child->getAnimateState() == AnimState::ANIM_END) {
      anim_end_ = true;
      child->onAnimateEnd();
    }
  }
}

bool AnimateView::onAnimateStart() {
  return false;
}

AnimateView::AnimState AnimateView::onAnimateProgress() {
  return AnimState::ANIM_END;
}

bool AnimateView::onAnimateCancelStart() {
  return false;
}

AnimateView::AnimState AnimateView::onAnimateCancelProgress() {
  return AnimState::ANIM_END;
}

void AnimateView::onAnimateEnd() {
}

}  // namespace ui
