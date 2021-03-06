#ifndef __UI_WIDGET_H__
#define __UI_WIDGET_H__
#include <view.h>

namespace nglui{

class Widget : public View {
protected:
  int text_alignment_;
 public:
  static constexpr const char* VIEW_NAME = "Widget";

 public:
  explicit Widget(const std::string& text);
  explicit Widget(const std::string& text, int width, int height);
  explicit Widget(const std::string& text, RECT& bound);
  explicit Widget(int width, int height);

  virtual ~Widget();
  virtual void setText(const std::string& text);
  virtual const std::string& getText() const;
  virtual void setAlignment(int alignment);
  virtual int getAlignment();
 protected:
  std::string text_;

 private:
  typedef View INHERITED;
  DISALLOW_COPY_AND_ASSIGN(Widget);
};

}
#endif
