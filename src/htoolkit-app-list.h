/* htoolkit-app-list.c
 *
 * Copyright (C) 2020 Hancom Gooroom <gooroom@hancom.com>
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

#ifndef HTOOLKIT_APP_LIST_H
#define HTOOLKIT_APP_LIST_H

#include <gtk/gtk.h>
#include "htoolkit-app.h"

G_BEGIN_DECLS

#define HTOOLKIT_TYPE_APP_LIST (htoolkit_app_list_get_type ())

G_DECLARE_DERIVABLE_TYPE (HToolkitAppList, htoolkit_app_list, HTOOLKIT, APP_LIST, GtkListBox)

struct _HToolkitAppListClass
{
    GtkListBoxClass parent_class;
};

GtkWidget     *htoolkit_app_list_new (void);
void           htoolkit_app_list_add_app    (HToolkitAppList	*list,
                                             HToolkitApp		*app);

G_END_DECLS
#endif /* HTOOLKIT_APP_LIST_H */
