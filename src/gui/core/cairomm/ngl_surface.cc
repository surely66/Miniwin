/* Copyright (C) 2005 The cairomm Development Team
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include <cairomm/ngl_surface.h>
#include <cairomm/private.h>

namespace Cairo
{

#ifdef CAIRO_HAS_NGL_SURFACE

NGLSurface::NGLSurface(cairo_surface_t* cobject, bool has_reference) :
    Surface(cobject, has_reference)
{}

NGLSurface::~NGLSurface()
{
  // surface is destroyed in base class
}

DWORD NGLSurface::getSurface() const
{
  return cairo_ngl_surface_get_surface(m_cobject);
}

RefPtr<ImageSurface> NGLSurface::get_image()
{
  RefPtr<ImageSurface> surface(new ImageSurface(cobj(), false /* no reference, owned by this win32surface*/));
  check_object_status_and_throw_exception(*this);
  return surface;
}

RefPtr<NGLSurface> NGLSurface::create(DWORD nglsurface)
{
  auto cobject = cairo_ngl_surface_create(nglsurface);
  check_status_and_throw_exception(cairo_surface_status(cobject));
  return RefPtr<NGLSurface>(new NGLSurface(cobject, true /* has reference */));
}

RefPtr<NGLSurface> NGLSurface::create(Format format, int width, int height)
{
  DWORD nglsurface;
  nglCreateSurface(&nglsurface,width,height,0,0);
  auto cobject=cairo_ngl_surface_create(nglsurface);
  return RefPtr<NGLSurface> (new NGLSurface(cobject,true));//create_with_dib(format, width, height);
}

#endif // CAIRO_HAS_NGL_SURFACE

} //namespace Cairo

// vim: ts=2 sw=2 et
