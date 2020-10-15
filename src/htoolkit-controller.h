/* htoolkit-controller.c
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

#ifndef HTOOLKIT_CONTROLLER_H
#define HTOOLKIT_CONTROLLER_H

#include <gtk/gtk.h>
#include <glib-object.h>

#include "htoolkit-app.h"

#define HTOOLKIT_TYPE_CONTROLLER (htoolkit_controller_get_type ())

G_DECLARE_DERIVABLE_TYPE (HToolkitController, htoolkit_controller, HTOOLKIT, CONTROLLER, GObject)

struct _HToolkitControllerClass
{
    GObjectClass    parent_instance;
    void          (*update_data)        (HToolkitController *self);
};

HToolkitController *htoolkit_controller_new (void);

void                htoolkit_controller_add_package (HToolkitController *control, HToolkitApp *app);
void                htoolkit_controller_start_job (HToolkitController *control);
#endif /* __HTOOLKIT_WINDOW_VIEW_H */

