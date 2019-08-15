#ifndef __UI_LABEL_H__
#define __UI_LABEL_H__
#include <widget.h>

namespace nglui{

class Label : public Widget{
 protected:
   std::string text_;
 public:
  Label();
  explicit Label(std::string& text);
  explicit Label(std::string& text, int w, int h);
  virtual ~Label();

  virtual void onDraw(GraphContext& canvas) override;
  virtual const SIZE& getPreferSize() override;
 private:
  typedef Widget INHERITED;
  DISALLOW_COPY_AND_ASSIGN(Label);
};

}
#endif
