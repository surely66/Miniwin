#ifndef __NGL_VIEW_H__
#define __NGL_VIEW_H__
#include <uievents.h>
#include <graph_context.h>
#include <layout.h>
#include <memory>
#include <vector>
struct pixman_region32;
namespace nglui{
#define SIF_ALL             0xFF//:整个结构都有效
#define SIF_DISABLENOSCROLL 0x01//:该值仅在设定参数时使用，视控件参数设定的需要来对本结构的成员进行取舍。
#define SIF_PAGE            0x02//:nPage成员有效
#define SIF_POS             0x04//:nPos成员有效
#define SIF_RANGE           0x08//:nMin和nMax成员有效
#define SIF_TRACKPOS        0x10

#define SB_VERT 0
#define SB_HORZ 1
typedef struct tagSCROLLINFO {  // si 
    UINT cbSize; 
    UINT fMask; 
    int  nMin; 
    int  nMax; 
    UINT nPage; 
    int  nPos; 
    int  nTrackPos; 
}SCROLLINFO;

/*typedef struct tagSCROLLBARINFO {
  RECT  rcScrollBar;
  int   dxyLineButton;
  int   xyThumbTop;
  int   xyThumbBottom;
  int   reserved;
  DWORD rgstate[CCHILDREN_SCROLLBAR + 1];
} SCROLLBARINFO;*/

class View{
public:
  static constexpr UINT DefaultFgColor = 0xFF000000;
  static constexpr UINT DefaultBgColor = 0xFFFFFFFF;
  static constexpr UINT  DefaultFontSize = 20;
  enum class Attr : uint32_t {
    ATTR_NONE         = 0x0000,
    ATTR_ENABLE       = 0x0001,
    ATTR_VISIBLE      = 0x0002,
    ATTR_FOCUSABLE    = 0x0004,
    ATTR_FOCUSED      = 0x0008,
    ATTR_BORDER       = 0x0010,
    ATTR_TRANSPARENT  = 0x0020,
    ATTR_SCROLL_VERT  = 0x0040,
    ATTR_SCROLL_HORZ  = 0x0080,
    ATTR_DEBUG        = 0x8000,
  };
  enum class AnimState : uint8_t {
    ANIM_NONE = 0,
    ANIM_INITIAL,
    ANIM_PROGRESS,
    ANIM_CANCEL,
    ANIM_CANCEL_PROGRESS,
    ANIM_END,
  };
  enum{
    WM_CREATE =0,
    WM_DESTROY=1,//no param
    WM_INVALIDATE=2,
    WM_TIMER  =3,//wParam it timerid lParam unused
    WM_CLICK  =4, //wParam is view's id
    WM_CHAR   =5//wparam is unicode char
  };

typedef std::function<void(View&v)>ClickListener;//typedef void(*ClickListener)(View&);
typedef std::function<bool(View&,DWORD,DWORD,ULONG)>MessageListener;
protected:
    int id_;
    int font_size_;
    int fg_color_;
    //int bg_color_;
    RefPtr<const Pattern> bg_pattern_;
    AnimState anim_state_; 
    Attr attr_;
    View*parent_;
    RECT bound_;
    SIZE prefer_size_;
    SCROLLINFO scrollinfos[2];
    struct pixman_region32 *invalid_region_;
    std::vector< std::shared_ptr<View> > children_;
    std::unique_ptr<class Layout> layout_;
    ClickListener onclick_;
    MessageListener onmessage_;
    virtual void setAnimateState(AnimState state);
public:
    View(int w,int h);
    virtual ~View();
    virtual GraphContext*getCanvas();
    virtual void onDraw(GraphContext&ctx);
    virtual void invalidate(const RECT*);
    const RECT&getBound();
    virtual View& setBound(const RECT&);
    virtual View& setBound(int x,int y,int w,int h);
    virtual View& setPos(int x,int y);
    virtual View& setSize(int x,int y);
    int getX();//x pos to screen
    int getY();//y pos to screen
    int getWidth();
    int getHeight();
    void clip(GraphContext&canvas);
    void resetClip();
    const RECT getClientRect();
    virtual AnimState getAnimateState();
    virtual void onAnimate();
    virtual void setClickListener(ClickListener ls);
    virtual void setMessageListener(MessageListener ls);
  // Foreground color
    virtual View& setBgPattern(const RefPtr<const Pattern>& source);
    virtual RefPtr<const Pattern>getBgPattern();
    virtual View& setFgColor(UINT color);
    virtual UINT getFgColor() const;

  // Background color
    virtual View& setBgColor(UINT color);
    virtual UINT getBgColor() const;

    virtual const SIZE&getPreferSize(); 
    View& setId(int id);
    int getId()const;
    void setScrollInfo(int bar,const SCROLLINFO*info,bool redraw=true);
    virtual void drawScrollBar(GraphContext&canvas,int bar);
      // Font size
    virtual int32_t getFontSize() const;
    virtual View& setFontSize(int32_t sz);

    // Layout
    virtual Layout* setLayout(Layout* layout);
    virtual Layout* getLayout() const;
    virtual void onLayout();
 
   // Attribute
    virtual void setFlag(Attr flag);
    virtual void clearFlag(Attr flag);
    virtual void resetFlag();
    virtual bool hasFlag(Attr flag) const;
    virtual bool isFocused()const;
    // Enable & Visible
    virtual void setVisible(bool visable);
    virtual bool isVisible() const;
    virtual void setEnable(bool enable);
    virtual bool isEnable() const;

    // Parent and children views
    virtual int getViewOrder(View*v);
    virtual View*getParent();
    virtual void setParent(View*p);
    virtual View*getChildView(size_t idx);
    virtual View*findViewById(int id);
    virtual View* addChildView(View* view);
    virtual void removeChildView(View* view);
    virtual void removeChildView(size_t idx);
    virtual void removeChildren();
    virtual size_t getChildrenCount() const;

    virtual void onResize(SIZE& size);
    virtual bool onKeyChar(KeyEvent& evt);
    virtual bool onKeyPress(KeyEvent& evt);
    virtual bool onKeyRelease(KeyEvent& evt);
    virtual bool onMousePress(Event& evt);
    virtual bool onMouseRelease(Event& evt);
    virtual bool onMouseMotion(Event& evt);
    virtual void sendMessage(DWORD msgid,DWORD wParam,ULONG lParam,DWORD delayedtime=0);
    virtual void sendMessage(std::shared_ptr<View>w,DWORD msgid,DWORD wParam,ULONG lParam,DWORD delayedtime=0);
    virtual bool onMessage(DWORD msgid,DWORD wParam,ULONG lParam);
private:
    void invalidate_inner(const RECT*rc);
    DISALLOW_COPY_AND_ASSIGN(View);
};

}//endof namespace nglui
#endif
