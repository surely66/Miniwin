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

#ifndef UI_LIBUI_TEXTFIELD_H_
#define UI_LIBUI_TEXTFIELD_H_

#include "widget.h"

namespace nglui {

class TextField : public Widget {
protected:
  RefPtr<ImageSurface>images[4];
  bool multiline;
public:
  static constexpr const char* VIEW_NAME = "TextField";
public:
  explicit TextField(const std::string& text);
  explicit TextField(const std::string& text, int width, int height);
  virtual ~TextField();
  virtual void setImage(int dir,const std::string&resname);
  virtual void onDraw(GraphContext& canvas) override;
  virtual const SIZE& getPreferSize() override;
  void setMultiLine(bool);
 private:
  typedef Widget INHERITED;
  DISALLOW_COPY_AND_ASSIGN(TextField);
};

}  // namespace ui

#endif  // UI_LIBUI_TEXTFIELD_H_
