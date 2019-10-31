#ifndef __UI_ABSLIST_H__
#define __UI_ABSLIST_H__
#include <widget.h>
namespace nglui{

class AbsListView:public Widget{
public:
   class ListItem{
   private:
     std::string text_;
     int value_;
   public:
     RECT rect;//used to store item's rectangle(last draw rect)
   public:
     ListItem(const std::string&txt,int v=0);
     virtual ~ListItem();
     virtual const std::string& getText()const;
     virtual void setText(const std::string&);
     virtual void onGetSize(AbsListView&lv,int* w,int* h);
     virtual void setValue(int v){value_=v;}
     virtual int getValue()const{return value_;}
   };
   typedef std::function<void(AbsListView&,int)>ItemSelectListener;
   typedef std::function<void(AbsListView&,const ListItem&,int state,GraphContext&)>ItemPainter;
   static void DefaultPainter(AbsListView&,const ListItem&,int state,GraphContext&);
   typedef std::function<int (const ListItem&a,const ListItem&b)>ItemCompare;
protected:
   int index_;
   int top_;
   ItemPainter item_painter_;
   ItemSelectListener item_select_listener_;
   std::vector< std::shared_ptr<ListItem> >list_;
public:
   AbsListView(int w,int h);
   AbsListView(const std::string&txt,int w,int h);
   virtual void sort(ItemCompare ,bool reverse=false);
   virtual int getIndex();
   virtual int getTop();
   virtual void setIndex(int idx);
   virtual void setTop(int idx);//the first visibal item
   virtual unsigned int getItemCount();
   virtual void setItemPainter(ItemPainter painter);
   virtual void setItemSelectListener( ItemSelectListener lis);
   virtual ListItem* getItem(int idx);
   virtual void addItem(ListItem*itm);
   virtual void addItem(std::shared_ptr<ListItem>itm);
   virtual void removeItem(int idx);
   virtual void removeItem(ListItem*itm);
   virtual void clearAllItems();
   typedef Widget INHERITED;

};
}//namespace
#endif
