/* htoolkit-app.h
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

#ifndef HTOOLKIT_APP_H
#define HTOOLKIT_APP_H

#include <gtk/gtk.h>
#include <glib-object.h>

G_BEGIN_DECLS

#define HTOOLKIT_TYPE_APP (htoolkit_app_get_type ())

G_DECLARE_DERIVABLE_TYPE (HToolkitApp, htoolkit_app, HTOOLKIT, APP, GObject)

struct _HToolkitAppClass
{
  GObjectClass    parent_class;
};

HToolkitApp        *htoolkit_app_new (const gchar* id);

void                htoolkit_app_set_error_msg (HToolkitApp *app, const gchar* error);
void                htoolkit_app_set_download_path (HToolkitApp *app, const gchar* path);

void                htoolkit_app_set_package        (HToolkitApp *app, const gchar* package);
void                htoolkit_app_set_update_package  (HToolkitApp *app, const gchar* package);
void                htoolkit_app_set_remove_package  (HToolkitApp *app, const gchar* package);
void                htoolkit_app_set_version   (HToolkitApp *app, const gchar* version);
void                htoolkit_app_set_name      (HToolkitApp *app, const gchar* name);
void                htoolkit_app_set_image_from_file     (HToolkitApp *app, const gchar* image);
void                htoolkit_app_set_image_from_resource (HToolkitApp *app, const gchar* image);

void                htoolkit_app_set_install_message (HToolkitApp *app, const gchar* msg);

void                htoolkit_app_set_uri       (HToolkitApp *app, const gchar* uri);
void                htoolkit_app_set_referer   (HToolkitApp *app, const gchar* referer);
void                htoolkit_app_set_dest      (HToolkitApp *app, const gchar* dest);
void                htoolkit_app_set_md5       (HToolkitApp *app, const gchar* md5);
void                htoolkit_app_set_sha256    (HToolkitApp *app, const gchar* sha256);

void                htoolkit_app_set_state     (HToolkitApp *app, guint state);
void                htoolkit_app_set_progress  (HToolkitApp *app, guint progress);

void                htoolkit_app_set_installed (HToolkitApp *app, gboolean installed);
void                htoolkit_app_set_update (HToolkitApp *app, gboolean update);
void                htoolkit_app_set_valid (HToolkitApp *app, gboolean valid);

void                htoolkit_app_set_thread (HToolkitApp *app, GThread *thread);

gchar*              htoolkit_app_get_error_msg (HToolkitApp *app);
gchar*              htoolkit_app_get_download_path (HToolkitApp *app);

gchar*              htoolkit_app_get_package       (HToolkitApp *app);
gchar*              htoolkit_app_get_update_package (HToolkitApp *app);
gchar*              htoolkit_app_get_remove_package (HToolkitApp *app);
gchar*              htoolkit_app_get_version   (HToolkitApp *app);
gchar*              htoolkit_app_get_name      (HToolkitApp *app);
gchar*              htoolkit_app_get_image_from_file         (HToolkitApp *app);
gchar*              htoolkit_app_get_image_from_resource     (HToolkitApp *app);
gchar*              htoolkit_app_get_install_message         (HToolkitApp *app);
gchar*              htoolkit_app_get_uri       (HToolkitApp *app);
gchar*              htoolkit_app_get_referer   (HToolkitApp *app);
gchar*              htoolkit_app_get_dest      (HToolkitApp *app);
gchar*              htoolkit_app_get_md5       (HToolkitApp *app);
gchar*              htoolkit_app_get_sha256    (HToolkitApp *app);

guint               htoolkit_app_get_state (HToolkitApp *app);
guint               htoolkit_app_get_progress (HToolkitApp *app);

gboolean            htoolkit_app_get_installed (HToolkitApp *app);
gboolean            htoolkit_app_get_update (HToolkitApp *app);
gboolean            htoolkit_app_get_valid (HToolkitApp *app);

GThread*            htoolkit_app_get_thread (HToolkitApp *app);

G_END_DECLS

#endif
