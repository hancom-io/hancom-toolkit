/* htoolkit-application.h
 *
 * Copyright (C) 2020 Hancom Inc. <gooroom@hancom.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define HTOOLKIT_TYPE_APPLICATION (htoolkit_application_get_type ())
#define HTOOLKIT_APPLICATION(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), HTOOLKIT_TYPE_APPLICATION, HToolkitApplication))

typedef struct _HToolkitApplication           HToolkitApplication;
typedef struct _HToolkitApplicationClass      HToolkitApplicationClass;
typedef struct _HToolkitApplicationPrivate    HToolkitApplicationPrivate;

struct _HToolkitApplication
{
  GtkApplication parent;
  HToolkitApplicationPrivate *priv;
};

struct _HToolkitApplicationClass
{
  GtkApplicationClass parent_class;
};

GType                               htoolkit_application_get_type        (void);
HToolkitApplication                *htoolkit_application_new             (void);

G_END_DECLS
