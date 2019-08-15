#include <ngl_types.h>
#include <ngl_log.h>
#include <svg.h>
#include<cairomm/context.h>
NGL_MODULE(SVG_RENDER)
//reference https://github.com/pltxtra/libsvgandroid/blob/master/src_jni/libsvg-android/svg_android_render_helper.c
//
using namespace Cairo;
typedef struct SVGSTATE_{
    struct SVGSTATE_ *next;
    float dpi;
    
    const char *id;
    const char *klass;
    
    double viewport_x, unscaled_viewport_x;
    double viewport_y, unscaled_viewport_y;
    double viewport_width, unscaled_viewport_width;
    double viewport_height, unscaled_viewport_height;

    double viewbox_width, unscaled_viewbox_width;
    double viewbox_height, unscaled_viewbox_height;
    
    svg_color_t color;
    double fill_opacity;
    svg_paint_t fill_paint;
    svg_fill_rule_t fill_rule;
    
    const char *font_family_set;
    double font_size, unscaled_font_size;
    svg_font_style_t font_style;
    unsigned int font_weight;
    
    double opacity;
    
    int viewport_set;
    
    const double *dash_array;
    int num_dashes;
    svg_length_t dash_offset;
    svg_stroke_line_cap_t stroke_line_cap;
    svg_stroke_line_join_t stroke_line_join;
    double stroke_miter_limit;
    double stroke_opacity;
    svg_paint_t stroke_paint;
    svg_length_t stroke_width;
    
    svg_text_anchor_t text_anchor;
    
    int visibility;
    
    const svg_clip_path_t *clip_path;
    svg_clip_rule_t clip_rule;
    
    const svg_mask_t *mask;
    
    const svg_marker_t *marker_end;
    const svg_marker_t *marker_mid;
    const svg_marker_t *marker_start;
    
    cairo_matrix_t matrix;
    cairo_matrix_t viewport_matrix;

    double units_scale_factor;
    double viewport_units_scale_factor;
    
    int bbox_user_units;
    double pre_bbox_units_scale_factor;
}SVGSTATE;

typedef struct{
   svg_t *svg;
   int state_store_level;
   RefPtr<Context>cr;
   RefPtr<ImageSurface>surface;
   SVGSTATE*states; 
}SVGCONTEXT;

#define GETVARS(x) SVGCONTEXT*svgctx=(SVGCONTEXT*)(x);\
    SVGSTATE*ss=svgctx->states;Context*cr=svgctx->cr;\


#define SVG2PX(x) (x)->value
/////////////////////////////state//////////////////////////////
static int init_state(SVGSTATE *context)
{
    memset(context, 0, sizeof(*context));
    /* non-zero initializations */
    context->dpi = 90;
    /* same arbitrary viewport dimensions as libsvg-cairo */
    context->unscaled_viewport_width = 450;
    context->unscaled_viewport_height = 450;
    context->viewport_width = 450;
    context->viewport_height = 450;
    
    context->viewbox_width = context->viewport_width;
    context->viewbox_height = context->viewport_height;
    context->unscaled_viewbox_width = context->unscaled_viewport_width;
    context->unscaled_viewbox_height = context->unscaled_viewport_height;

    context->fill_paint.type = SVG_PAINT_TYPE_COLOR;
    context->fill_opacity = 1.0;
    context->fill_rule = SVG_FILL_RULE_NONZERO;
    
    context->font_family_set = "sans-serif";
    context->font_size = 12.0; /* medium */
    context->unscaled_font_size = 12.0; /* medium */
    context->font_style = SVG_FONT_STYLE_NORMAL;
    context->font_weight = 400; /* normal */

    context->opacity = 1.0;

    context->stroke_line_cap = SVG_STROKE_LINE_CAP_BUTT;
    context->stroke_line_join = SVG_STROKE_LINE_JOIN_MITER;
    context->stroke_miter_limit = 4.0;
    context->stroke_opacity = 1.0;
    context->stroke_width.value = 1.0;
    context->stroke_width.unit = SVG_LENGTH_UNIT_PX;
    context->stroke_width.orientation = SVG_LENGTH_ORIENTATION_OTHER;
    context->stroke_paint.type = SVG_PAINT_TYPE_NONE;
    
    context->text_anchor = SVG_TEXT_ANCHOR_START;
    
    context->visibility = 1;

    cairo_matrix_init_identity(&context->matrix);//_sts_matrix_identity(&context->matrix);
    cairo_matrix_init_identity(&context->viewport_matrix);////_sts_matrix_identity(&context->viewport_matrix);
    
    context->units_scale_factor = 1.0;
    context->viewport_units_scale_factor = 1.0;

    return 1;
}
static int dup_state(const SVGSTATE*from_context,SVGSTATE*to_context)
{
    *to_context = *from_context;
    to_context->next = NULL;
    to_context->id = NULL;
    to_context->klass = NULL;
    to_context->viewport_set = 0;
    to_context->viewport_matrix = from_context->matrix;
    to_context->viewport_units_scale_factor = from_context->units_scale_factor;
    return 1;
}
static int svg_push_state(SVGCONTEXT*ctx,SVGSTATE**st){
   SVGSTATE*last_state=ctx->states;
   SVGSTATE*new_state=(SVGSTATE*)malloc(sizeof(SVGSTATE));
   if(last_state==NULL)
      init_state(new_state);
   else dup_state(last_state,new_state);
   ctx->states=new_state;
   ctx->states->next=last_state;
   ctx->state_store_level++;
   NGLOG_VERBOSE(">>>>>>>>state_store_level=%d",ctx->state_store_level);
   return 1;
}
static void svg_pop_state(SVGCONTEXT*ctx){
   SVGSTATE*last=ctx->states;
   if(last==NULL)return;
   ctx->states=last->next;
   ctx->state_store_level--;
   NGLOG_VERBOSE("<<<<<<<<state_store_level=%d",ctx->state_store_level);
}
////////////////////////////////////////////////////////////////

static svg_status_t eng_begin_group (void *closure, double opacity,const char *id, const char *klass){
     //RefPtr<ImageSurface>surface=ImageSurface::create(FORMAT_ARGB32,200,200);//ss->viewport_width,ss->viewport_height);
     SVGCONTEXT*svgctx=(SVGCONTEXT*)closure;
     SVGSTATE*ss;
     svg_push_state(svgctx,&ss);//surface,nullptr);
     ss->opacity=opacity;
     //ss->id=id;
     //ss->klass=klass;
     return SVG_STATUS_SUCCESS;
}

static svg_status_t eng_begin_element (void *closure, const char *id, const char *klass){
     GETVARS(closure);
     svg_push_state(svgctx,&ss);
     //ss->id=id;
     //ss->klass=klass;
     return SVG_STATUS_SUCCESS;
}

static svg_status_t eng_end_element (void *closure){
    GETVARS(closure);
    svg_pop_state(svgctx);
    return SVG_STATUS_SUCCESS;
}
static svg_status_t eng_end_group (void *closure, double opacity){
    GETVARS(closure);
    svg_pop_state(svgctx);
    return SVG_STATUS_SUCCESS;
}
/* transform */
static svg_status_t eng_set_viewport(void *closure, const svg_length_t *x, const svg_length_t *y,const svg_length_t *width, const svg_length_t *height){
    GETVARS(closure);
    ss->viewport_set = 1;
    return SVG_STATUS_SUCCESS;
}

static svg_status_t eng_apply_view_box (void *closure,const svg_view_box_t* view_box, const svg_length_t *width,  const svg_length_t *height){
    GETVARS(closure);
    svgctx->surface=ImageSurface::create(FORMAT_ARGB32,SVG2PX(width),SVG2PX(height));
    svgctx->cr=Context::create(svgctx->surface);
    ss->viewport_width =SVG2PX(width);
    ss->viewport_height=SVG2PX(height);
    return SVG_STATUS_SUCCESS;
}
static svg_status_t eng_viewport_clipping_path(void *closure,const svg_length_t *top, const svg_length_t *right,
			const svg_length_t *bottom, const svg_length_t *left){
    GETVARS(closure);
    if(ss->viewport_set)
        return SVG_STATUS_SUCCESS;
    return SVG_STATUS_SUCCESS;
}

static svg_status_t eng_transform (void *closure,const svg_transform_t*transform){
    GETVARS(closure);
    cairo_matrix_t matrix=*(cairo_matrix_t*)transform;//svg_transform_t is same
    matrix.x0*=ss->units_scale_factor;
    matrix.y0*=ss->units_scale_factor;
    cairo_matrix_multiply(&ss->matrix,&matrix,&ss->matrix);
    //cr->transform(ss->matrix);
    cr->set_matrix(ss->matrix);
    return SVG_STATUS_SUCCESS;
}

static svg_status_t eng_end_transform(void *closure){
    GETVARS(closure);
    NGLOG_DEBUG("");
    return SVG_STATUS_SUCCESS;
}
    /* style */
static svg_status_t eng_set_clip_path (void *closure, const svg_clip_path_t *clip_path){
    GETVARS(closure);
    ss->clip_path = clip_path;
    NGLOG_ERROR("clip_path=%p",clip_path);
    return SVG_STATUS_SUCCESS;
}

static svg_status_t eng_set_clip_rule (void *closure, svg_clip_rule_t clip_rule){
    GETVARS(closure);
    ss->clip_rule = clip_rule;
    NGLOG_ERROR("clip_rule=%d",clip_rule);
    return SVG_STATUS_SUCCESS;
}

static svg_status_t eng_set_color (void *closure, const svg_color_t *color){
    GETVARS(closure);
    ss->color=*color;
    return SVG_STATUS_SUCCESS;
}

static svg_status_t eng_set_fill_opacity (void *closure, double fill_opacity){
    GETVARS(closure);
    ss->fill_opacity=fill_opacity;
    return SVG_STATUS_SUCCESS;
}

static void set_painter(SVGCONTEXT*svgctx,svg_paint_t*paint);
static svg_status_t eng_set_fill_paint (void *closure, const svg_paint_t *paint){
    GETVARS(closure);
    ss->fill_paint=*paint;
    //set_painter(svgctx,&ss->fill_paint);
    return SVG_STATUS_SUCCESS;
}

static svg_status_t eng_set_fill_rule (void *closure, svg_fill_rule_t fill_rule){
    GETVARS(closure);
    ss->fill_rule=fill_rule;
    return SVG_STATUS_SUCCESS;
}

static svg_status_t eng_set_font_family (void *closure, const char *family){
    GETVARS(closure);
    NGLOG_VERBOSE("family=%s",family);
    return SVG_STATUS_SUCCESS;
}

static svg_status_t eng_set_font_size (void *closure, double size){
    GETVARS(closure);
    ss->font_size=size;
    cr->set_font_size(size);
    return SVG_STATUS_SUCCESS;
}

static svg_status_t eng_set_font_style (void *closure, svg_font_style_t font_style){
    GETVARS(closure);
    ss->font_style=font_style;
    return SVG_STATUS_SUCCESS;
}

static svg_status_t eng_set_font_weight (void *closure, unsigned int font_weight){
    GETVARS(closure);
    ss->font_weight=font_weight;
    return SVG_STATUS_SUCCESS;
}

static svg_status_t eng_set_marker_end (void *closure, const svg_marker_t *marker){
    GETVARS(closure);
    ss->marker_end = marker;
    NGLOG_ERROR("marker=%p",marker);
    return SVG_STATUS_SUCCESS;
}
static svg_status_t eng_set_marker_mid (void *closure, const svg_marker_t *marker){
    GETVARS(closure);
    ss->marker_mid = marker;
    NGLOG_ERROR("marker=%p",marker);
    return SVG_STATUS_SUCCESS;
}
static svg_status_t eng_set_marker_start (void *closure, const svg_marker_t *marker){
    GETVARS(closure);
    ss->marker_start = marker;
    NGLOG_ERROR("marker=%p",marker);
    return SVG_STATUS_SUCCESS;
}

static svg_status_t eng_set_mask (void *closure, const svg_mask_t *mask){
    GETVARS(closure);
    ss->mask=mask;
    NGLOG_ERROR("mask=%p",mask);
    return SVG_STATUS_SUCCESS;
}

static svg_status_t eng_set_opacity (void *closure, double opacity){
    GETVARS(closure);
    ss->opacity=opacity;
    return SVG_STATUS_SUCCESS;
}

static svg_status_t eng_set_stroke_dash_array (void *closure, const double *dash_array, int num_dashes){
    GETVARS(closure);
    ss->dash_array=dash_array;
    ss->num_dashes = num_dashes;
    return SVG_STATUS_SUCCESS;
}

static svg_status_t eng_set_stroke_dash_offset (void *closure, const svg_length_t *offset){
    GETVARS(closure);
    ss->dash_offset=*offset;
    return SVG_STATUS_SUCCESS;
}

static svg_status_t eng_set_stroke_line_cap (void *closure, svg_stroke_line_cap_t line_cap){
    GETVARS(closure);
    ss->stroke_line_cap=line_cap;
    return SVG_STATUS_SUCCESS;
}

static svg_status_t eng_set_stroke_line_join (void *closure, svg_stroke_line_join_t line_join){
    GETVARS(closure);
    ss->stroke_line_join=line_join;
    return SVG_STATUS_SUCCESS;
}

static svg_status_t eng_set_stroke_miter_limit (void *closure, double limit){
    GETVARS(closure);
    ss->stroke_miter_limit=limit;
    return SVG_STATUS_SUCCESS;
}

static svg_status_t eng_set_stroke_opacity (void *closure, double stroke_opacity){
    GETVARS(closure);
    ss->stroke_opacity=stroke_opacity;
    return SVG_STATUS_SUCCESS;
}

static svg_status_t eng_set_stroke_paint (void *closure, const svg_paint_t *paint){
    GETVARS(closure);
    ss->stroke_paint=*paint;
    //set_painter(svgctx,&ss->stroke_paint);
    return SVG_STATUS_SUCCESS;
}

static svg_status_t eng_set_stroke_width (void *closure, const svg_length_t *width){
    GETVARS(closure);
    cr->set_line_width(SVG2PX(width));
    return SVG_STATUS_SUCCESS;
}

static svg_status_t eng_set_text_anchor (void *closure, svg_text_anchor_t text_anchor){
    GETVARS(closure);
    return SVG_STATUS_SUCCESS;
}

static svg_status_t eng_set_visibility (void *closure, int visible){
    return SVG_STATUS_SUCCESS;
}
static svg_status_t eng_end_style (void *closure){
    return SVG_STATUS_SUCCESS;
}

/* text positioning */
static svg_status_t eng_text_advance_x(void *closure, const char *utf8, double *advance){
    return SVG_STATUS_SUCCESS;
}
static svg_status_t eng_set_text_position_x (void *closure, const svg_length_t *x){
    return SVG_STATUS_SUCCESS;
}
static svg_status_t eng_set_text_position_y (void *closure, const svg_length_t *y){
    return SVG_STATUS_SUCCESS;
}
static svg_status_t eng_adjust_text_position (void *closure,const svg_length_t *dx, const svg_length_t *dy){
    return SVG_STATUS_SUCCESS;
}
static svg_status_t eng_set_text_chunk_width (void *closure, double width){
    return SVG_STATUS_SUCCESS;
}

/* drawing */
static svg_status_t eng_render_line (void *closure, const svg_length_t *x1, const svg_length_t *y1,
		const svg_length_t *x2, const svg_length_t *y2){
    GETVARS(closure);
    cr->move_to(SVG2PX(x1),SVG2PX(y1));
    cr->line_to(SVG2PX(x2),SVG2PX(y2));
    return SVG_STATUS_SUCCESS;
}
static svg_status_t eng_render_path (void *closure, const svg_path_t *path){
    GETVARS(closure);
    int path_is_open=0;
    double path_start_x = 0.0;
    double path_start_y = 0.0;
    double current_x = 0.0;
    double current_y = 0.0;
    for(const svg_path_t*p=path;p;p=p->next){
        switch(p->op){
        case SVG_PATH_OP_MOVE_TO:
              if(path_is_open&&ss->fill_paint.type != SVG_PAINT_TYPE_NONE)
                 cr->line_to(path_start_x, path_start_y);
              cr->move_to(p->p.move_to.x,p->p.move_to.y);
              current_x = p->p.move_to.x;
              current_y = p->p.move_to.y;
              path_start_x = p->p.move_to.x;
              path_start_y = p->p.move_to.y;
              break;
        case SVG_PATH_OP_LINE_TO:
              cr->line_to(p->p.line_to.x,p->p.line_to.y);
              current_x = p->p.line_to.x;
              current_y = p->p.line_to.y;
              path_is_open = 1;
              break;
        case SVG_PATH_OP_CURVE_TO:
              cr->curve_to(p->p.curve_to.x1,p->p.curve_to.y1,
                  p->p.curve_to.x2,p->p.curve_to.y2,p->p.curve_to.x3,p->p.curve_to.y3);
              current_x = p->p.curve_to.x3;
              current_y = p->p.curve_to.y3;
              path_is_open = 1;
              break;
        case SVG_PATH_OP_QUAD_CURVE_TO:NGLOG_ERROR("todo:SVG_PATH_OP_QUAD_CURVE_TO");break;
        case SVG_PATH_OP_ARC_TO:       NGLOG_ERROR("todo:SVG_PATH_OP_ARC_TO");break;
        case SVG_PATH_OP_CLOSE_PATH:
             cr->line_to(path_start_x, path_start_y);
             current_x = path_start_x;
             current_y = path_start_y;
             path_is_open = 0;
             break;
        default:break;
        }
    }
    if(ss->fill_paint.type){
         set_painter(svgctx,&ss->fill_paint);
         cr->fill_preserve();
    }
    if(ss->stroke_paint.type){
         set_painter(svgctx,&ss->stroke_paint);
         cr->stroke();
    }
    return SVG_STATUS_SUCCESS;
}
static svg_status_t eng_render_circle (void *closure, const svg_length_t *cx, const svg_length_t *cy,
			const svg_length_t *r){
    GETVARS(closure);
    cr->arc(SVG2PX(cx),SVG2PX(cy),SVG2PX(r),0,2*M_PI);
    if(ss->fill_paint.type){
         set_painter(svgctx,&ss->fill_paint);
         cr->fill_preserve();
    }
    if(ss->stroke_paint.type){
         set_painter(svgctx,&ss->stroke_paint);
         cr->stroke();
    }

    return SVG_STATUS_SUCCESS;
}
static 	svg_status_t eng_render_ellipse (void *closure, const svg_length_t *cx, const svg_length_t *cy,
			const svg_length_t *rx, const svg_length_t *ry){
    GETVARS(closure);
    cr->save();
    cr->scale(SVG2PX(rx),SVG2PX(ry));
    cr->arc(SVG2PX(cx),SVG2PX(cy),SVG2PX(rx),.0,2*M_PI);
    cr->restore();
    return SVG_STATUS_SUCCESS;
}
static 	svg_status_t eng_render_rect (void *closure, const svg_length_t *x, const svg_length_t *y,
			const svg_length_t *width, const svg_length_t *height,
			const svg_length_t *rx, const svg_length_t *ry){
    
    GETVARS(closure); 
    cr->rectangle(SVG2PX(rx),SVG2PX(ry),SVG2PX(width),SVG2PX(height));
    if(ss->fill_paint.type){
         set_painter(svgctx,&ss->fill_paint);
         cr->fill_preserve();
    }
    if(ss->stroke_paint.type){
         set_painter(svgctx,&ss->stroke_paint);
         cr->stroke();
    }
    return SVG_STATUS_SUCCESS;
}

static svg_status_t eng_render_text (void *closure, const char *utf8){
    GETVARS(closure);
    cr->text_path(utf8);
    if(ss->fill_paint.type){
         set_painter(svgctx,&ss->fill_paint);
         cr->fill_preserve();
    }
    if(ss->stroke_paint.type){
         set_painter(svgctx,&ss->stroke_paint);
         cr->stroke();
    }
    return SVG_STATUS_SUCCESS;
}
static svg_status_t eng_render_image (void *closure, const char *uri, int index, const svg_view_box_t *view_box_template,
			const svg_length_t *x, const svg_length_t *y,const svg_length_t *width, const svg_length_t *height){
    return SVG_STATUS_SUCCESS;
}

/* miscellaneous */
static svg_status_t eng_measure_position (void *closure, const svg_length_t *ix, const svg_length_t *iy,
			double *ox, double *oy){
    *ox=SVG2PX(ix);
    *oy=SVG2PX(iy);
    return SVG_STATUS_SUCCESS;
}
static svg_status_t eng_measure_font_size (void *closure, const char *font_family,
			double parent_font_size, const svg_length_t *in_size,
			double *out_size){
    *out_size=SVG2PX(in_size);
    return SVG_STATUS_SUCCESS;
}

static void set_painter(SVGCONTEXT*svgctx,svg_paint_t*paint){
    switch(paint->type){
    case SVG_PAINT_TYPE_COLOR:{
             uint32_t c=paint->p.color.rgb;
             NGLOG_DEBUG("c=%x opacity=%f ",c,svgctx->states->opacity);
             svgctx->cr->set_source_rgba(((c>>16)&0xFF)/255.,((c>>8)&0xFF)/255.,(c&0xFF)/255.,svgctx->states->opacity);
         }break;
    case SVG_PAINT_TYPE_GRADIENT:{
             NGLOG_DEBUG("gradienttype=%d",paint->p.gradient->type);
             svg_gradient_t*g=paint->p.gradient;
             RefPtr<Gradient>p;
             if(SVG_GRADIENT_LINEAR==g->type)
                p=LinearGradient::create(SVG2PX(&g->u.linear.x1),SVG2PX(&g->u.linear.y1),SVG2PX(&g->u.linear.x2),SVG2PX(&g->u.linear.y2));
             else if(SVG_GRADIENT_RADIAL==g->type){
                p=RadialGradient::create(SVG2PX(&g->u.radial.cx),SVG2PX(&g->u.radial.cy),SVG2PX(&g->u.radial.r),
                   SVG2PX(&g->u.radial.fx),SVG2PX(&g->u.radial.fy),SVG2PX(&g->u.radial.r));
             }
             for(int i=0;i<g->num_stops;i++){
                 uint32_t c=g->stops[i].color.rgb;
                 p->add_color_stop_rgba(g->stops[i].offset,(c>>16)/255.,(c>>8)/255.,(c&0xFF)/255.,g->stops[i].opacity);
             }
             svgctx->cr->set_source(p);
         }break;
    case SVG_PAINT_TYPE_PATTERN:{
             NGLOG_DEBUG("");
             //svg_pattern_t*p=paint->p.pattern_element;
         }break;
    case SVG_PAINT_TYPE_NONE:
    default:{
             uint32_t c=paint->p.color.rgb;
             svgctx->cr->set_source_rgb((c>>16)/255.,(c>>8)/255.,(c&0xFF)/255.);
         }break;
    }
}

void init_svg_engine(svg_render_engine_t*eng){
	/* hierarchy */
//        memset(eng,0,sizeof(svg_render_engine_t));
	eng->begin_group  =eng_begin_group;
	eng->begin_element=eng_begin_element;
	eng->end_element  =eng_end_element;
	eng->end_group    =eng_end_group;
        /* transform */
	eng->set_viewport=eng_set_viewport;
        eng->apply_view_box=eng_apply_view_box;
	eng->viewport_clipping_path=eng_viewport_clipping_path;
	eng->transform=eng_transform;
        eng->end_transform=NULL;
	/* style */
        eng->set_clip_path=eng_set_clip_path;
        eng->set_clip_rule=eng_set_clip_rule;
	eng->set_color =  eng_set_color;
	eng->set_fill_opacity = eng_set_fill_opacity;
	eng->set_fill_paint   =eng_set_fill_paint;
	eng->set_fill_rule    =eng_set_fill_rule;
	eng->set_font_family  =eng_set_font_family;
	eng->set_font_size    =eng_set_font_size;
	eng->set_font_style =  eng_set_font_style;
	eng->set_font_weight =  eng_set_font_weight;
        eng->set_marker_end= eng_set_marker_end;
        eng->set_marker_mid=eng_set_marker_mid;
        eng->set_marker_start=eng_set_marker_start;
        eng->set_mask=eng_set_mask;
	eng->set_opacity =  eng_set_opacity;
	eng->set_stroke_dash_array =  eng_set_stroke_dash_array;
	eng->set_stroke_dash_offset =  eng_set_stroke_dash_offset;
	eng->set_stroke_line_cap =  eng_set_stroke_line_cap;
	eng->set_stroke_line_join =  eng_set_stroke_line_join;
	eng->set_stroke_miter_limit =  eng_set_stroke_miter_limit;
	eng->set_stroke_opacity =  eng_set_stroke_opacity;
	eng->set_stroke_paint =  eng_set_stroke_paint;
	eng->set_stroke_width =  eng_set_stroke_width;
	eng->set_text_anchor =  eng_set_text_anchor;
        eng->set_visibility =eng_set_visibility;
        eng->end_style=eng_end_style;
        	
/* text positioning */
        eng->text_advance_x=eng_text_advance_x;
        eng->set_text_position_x=eng_set_text_position_x;
        eng->set_text_position_y=eng_set_text_position_y;
        eng->adjust_text_position=eng_adjust_text_position;
        eng->set_text_chunk_width=eng_set_text_chunk_width;
        
/* drawing */
	eng->render_line =  eng_render_line;
	eng->render_path =  eng_render_path;
        eng->render_circle=eng_render_circle;
	eng->render_ellipse =eng_render_ellipse;
	eng->render_rect =  eng_render_rect;
	eng->render_text =  eng_render_text;
	eng->render_image = eng_render_image;
 /* miscellaneous */
        eng->measure_position=eng_measure_position;
        eng->measure_font_size=eng_measure_font_size;
}

static const char*svgxml="<svg viewBox=\"0 0 100 100\" xmlns=\"http://www.w3.org/2000/svg\">\
    <g id=\"ss\" stroke=\"green\" fill=\"white\" stroke-width=\"5\">\
    <circle cx=\"50\" cy=\"50\" r=\"35\" fill=\"red\" stroke=\"blue\" stroke-width=\"10\"/>\
    <path d=\"M 40 40 L 30 10 L 20 30 z\" fill=\"orange\" stroke=\"black\" stroke-width=\"3\" />\
  </g></svg> ";

RefPtr<ImageSurface> loadSVG(const char*buffer,size_t buffer_size){
    SVGCONTEXT*ctx=new SVGCONTEXT();
    memset(ctx,0,sizeof(SVGCONTEXT));
    svg_status_t rc;
    svg_render_engine_t svg_render_engine;
    init_svg_engine(&svg_render_engine);
    ctx->state_store_level=0;
    svg_create(&ctx->svg);
    rc=svg_trace_render_engine(ctx->svg);
    NGLOG_DEBUG("\r\n\n===loadSVG===engine=%p ctx=%p trace=%x",&svg_render_engine,ctx,rc);
    NGLOG_DEBUG("svg=%p buffer=%p size=%d",ctx->svg,buffer,buffer_size);
    //rc=svg_parse(ctx->svg,"./button.svg");
    rc=svg_parse_buffer(ctx->svg,buffer,buffer_size);
    NGLOG_DEBUG("parse svg=%x",rc);
    rc=svg_render(ctx->svg,&svg_render_engine,ctx); 
    NGLOG_DEBUG("svg_render finished=%x",rc);
    return ctx->surface;
}
