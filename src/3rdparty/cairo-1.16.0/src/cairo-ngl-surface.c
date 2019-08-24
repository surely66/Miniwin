/* cairo - a vector graphics library with display and print output
 *
 * Copyright © 2012 Intel Corporation
 *
 * This library is free software; you can redistribute it and/or
 * modify it either under the terms of the GNU Lesser General Public
 * License version 2.1 as published by the Free Software Foundation
 * (the "LGPL") or, at your option, under the terms of the Mozilla
 * Public License Version 1.1 (the "MPL"). If you do not alter this
 * notice, a recipient may use your version of this file under either
 * the MPL or the LGPL.
 *
 * You should have received a copy of the LGPL along with this library
 * in the file COPYING-LGPL-2.1; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Suite 500, Boston, MA 02110-1335, USA
 * You should have received a copy of the MPL along with this library
 * in the file COPYING-MPL-1.1
 *
 * The contents of this file are subject to the Mozilla Public License
 * Version 1.1 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY
 * OF ANY KIND, either express or implied. See the LGPL or the MPL for
 * the specific language governing rights and limitations.
 *
 * The Original Code is the cairo graphics library.
 *
 * The Initial Developer of the Original Code is Chris Wilson
 *
 * Contributor(s):
 *    Chris Wilson <chris@chris-wilson.co.uk>
 */

#include "cairoint.h"
#include "cairo-ngl.h"

#include "cairo-clip-private.h"
#include "cairo-compositor-private.h"
#include "cairo-default-context-private.h"
#include "cairo-error-private.h"
#include "cairo-image-surface-inline.h"
#include "cairo-pattern-private.h"
#include "cairo-surface-backend-private.h"
#include "cairo-surface-fallback-private.h"

#include <pixman.h>

#include <ngl_types.h>
#include <ngl_graph.h>
#include <ngl_log.h>

NGL_MODULE(CAIRO-NGL)

slim_hidden_proto(cairo_ngl_surface_create);

typedef struct _cairo_ngl_surface {
    cairo_image_surface_t image;
    DWORD  nglsurface;
} cairo_ngl_surface_t;

static cairo_content_t
_ngl_format_to_content (int  format)
{
    cairo_content_t content = 0;

    /*if (DFB_PIXELFORMAT_HAS_ALPHA (format))
	content |= CAIRO_CONTENT_ALPHA;
    if (DFB_COLOR_BITS_PER_PIXEL (format))
	content |= CAIRO_CONTENT_COLOR_ALPHA;*/
    content|=CAIRO_CONTENT_ALPHA;
    assert(content);
    return content;
}

static inline pixman_format_code_t
_ngl_to_pixman_format (int format)
{
    switch (format) {
    case GPF_UNKNOWN: return 0;
    case GPF_ARGB1555: return PIXMAN_a1r5g5b5;
    case GPF_ARGB: return PIXMAN_a8r8g8b8;
    case GPF_ARGB4444: return PIXMAN_a4r4g4b4;
    default:return 0;
    }
    return 0;
}

static cairo_surface_t *
_cairo_ngl_surface_create_similar (void            *abstract_src,
				   cairo_content_t  content,
				   int              width,
				   int              height)
{
    cairo_ngl_surface_t *other  = abstract_src;
    cairo_surface_t *surface;
    DWORD nglsurface;
    INT format;
    if (width <= 0 || height <= 0)
	return _cairo_image_surface_create_with_content (content, width, height);

    switch (content) {
    default:
	ASSERT_NOT_REACHED;
    case CAIRO_CONTENT_COLOR_ALPHA:
	format = GPF_ARGB;
	break;
    case CAIRO_CONTENT_COLOR:
	format = GPF_RGB32;
	break;
//    case CAIRO_CONTENT_ALPHA:
//	format = GPF_A8;
	break;
    }

    if (nglCreateSurface(&nglsurface,width,height,format,0))//other->dfb->CreateSurface (other->dfb, &dsc, &buffer))
	return _cairo_surface_create_in_error (_cairo_error (CAIRO_STATUS_DEVICE_ERROR));

    surface = cairo_ngl_surface_create (nglsurface);
    return surface;
}

static cairo_status_t
_cairo_ngl_surface_finish (void *abstract_surface)
{
    cairo_ngl_surface_t *surface = abstract_surface;
    NGLOG_VERBOSE("destroy nglsurface %p",surface->nglsurface);
    nglDestroySurface(surface->nglsurface);
    return _cairo_image_surface_finish (abstract_surface);
}

static cairo_image_surface_t *
_cairo_ngl_surface_map_to_image (void *abstract_surface,
				 const cairo_rectangle_int_t *extents)
{
    cairo_ngl_surface_t *surface = abstract_surface;

    if (surface->image.pixman_image == NULL) {
	pixman_image_t *image;
	void *data;
	int pitch;

	if (nglLockSurface (surface->nglsurface,&data, &pitch))
	    return _cairo_image_surface_create_in_error (_cairo_error (CAIRO_STATUS_NO_MEMORY));
	image = pixman_image_create_bits (surface->image.pixman_format,
					  surface->image.width,
					  surface->image.height,
					  data, pitch);
        NGLOG_VERBOSE("nglsurface=%p buffer=%p pitch=%d pixman_img=%p",surface->nglsurface,data,pitch,image);
	if (image == NULL) {
	    nglUnlockSurface(surface->nglsurface);
	    return _cairo_image_surface_create_in_error (_cairo_error (CAIRO_STATUS_NO_MEMORY));
	}
	_cairo_image_surface_init (&surface->image, image, surface->image.pixman_format);
    }

    return _cairo_image_surface_map_to_image (&surface->image.base, extents);
}

static cairo_int_status_t
_cairo_ngl_surface_unmap_image (void *abstract_surface,
				cairo_image_surface_t *image)
{
    cairo_ngl_surface_t *surface = abstract_surface;
     nglUnlockSurface(surface->nglsurface);
    return _cairo_image_surface_unmap_image (&surface->image.base, image);
}

static cairo_status_t
_cairo_ngl_surface_flush (void *abstract_surface,
			  unsigned flags)
{
    cairo_ngl_surface_t *surface = abstract_surface;

    if (flags)
	return CAIRO_STATUS_SUCCESS;
    NGLOG_VERBOSE("nglsurface=%p surface->image.pixman_image=%p",surface->nglsurface,surface->image.pixman_image);
    if (surface->image.pixman_image) {
	//nglUnlockSurface(surface->nglsurface);//surface->dfb_surface->Unlock (surface->dfb_surface);

	pixman_image_unref (surface->image.pixman_image);
	surface->image.pixman_image = NULL;
	surface->image.data = NULL;
    }

    return CAIRO_STATUS_SUCCESS;
}


static cairo_surface_backend_t
_cairo_ngl_surface_backend = {
    CAIRO_SURFACE_TYPE_NGL, /*type*/
    _cairo_ngl_surface_finish, /*finish*/
    _cairo_default_context_create,

    _cairo_ngl_surface_create_similar,/*create_similar*/
    NULL, /* create similar image */
    _cairo_ngl_surface_map_to_image,
    _cairo_ngl_surface_unmap_image,

    _cairo_surface_default_source,
    _cairo_surface_default_acquire_source_image,
    _cairo_surface_default_release_source_image,
    NULL,

    NULL, /* copy_page */
    NULL, /* show_page */

    _cairo_image_surface_get_extents,
    _cairo_image_surface_get_font_options,

    _cairo_ngl_surface_flush,
    NULL, /* mark_dirty_rectangle */

    _cairo_surface_fallback_paint,
    _cairo_surface_fallback_mask,
    _cairo_surface_fallback_stroke,
    _cairo_surface_fallback_fill,
    NULL, /* fill-stroke */
    _cairo_surface_fallback_glyphs,
};

cairo_surface_t *cairo_ngl_surface_create (DWORD nglsurface)
{
    cairo_ngl_surface_t *surface;
    pixman_format_code_t pixman_format;
    int width, height;
    int format=GPF_ARGB;
    
    nglGetSurfaceInfo(nglsurface,&width,&height,&format);
    NGLOG_VERBOSE("nglsurface=%p,size=%dx%d",nglsurface,width,height);
    pixman_format =_ngl_to_pixman_format (format);
    if (! pixman_format_supported_destination (pixman_format))
        return _cairo_surface_create_in_error (_cairo_error (CAIRO_STATUS_INVALID_FORMAT));

    surface = calloc (1, sizeof (cairo_ngl_surface_t));
    if (surface == NULL)
        return _cairo_surface_create_in_error (_cairo_error (CAIRO_STATUS_NO_MEMORY));

    /* XXX dfb -> device */
    _cairo_surface_init (&surface->image.base,
                         &_cairo_ngl_surface_backend,
			 NULL, /* device */
			 _ngl_format_to_content (format),
			 FALSE); /* is_vector */

    surface->image.pixman_format = pixman_format;
    surface->image.format = _cairo_format_from_pixman_format (pixman_format);

    surface->image.width = width;
    surface->image.height = height;
    surface->image.depth = PIXMAN_FORMAT_DEPTH(pixman_format);

    surface->nglsurface = nglsurface;

    return &surface->image.base;
}

DWORD cairo_ngl_surface_get_surface (cairo_surface_t *surface)
{
    if (surface->backend->type == CAIRO_SURFACE_TYPE_NGL)
        return ((cairo_ngl_surface_t*)surface)->nglsurface;

    return 0;
}

slim_hidden_def(cairo_ngl_surface_create);
