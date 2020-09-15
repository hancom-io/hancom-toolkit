/* htoolkit-window.c
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

#include <glib/gi18n.h>

#include "utils.h"
#include "define.h"
#include "hancom-toolkit-config.h"
#include "htoolkit-window.h"
#include "htoolkit-app.h"
#include "htoolkit-app-list.h"

#include <json-glib/json-glib.h>

#define PACKAGE_RESOURCE_PATH "/kr/hancom/hancom-toolkit/hancom-toolkit.json"

struct _HToolkitWindow
{
    GtkApplicationWindow  parent_instance;
};

typedef struct
{
    /* Template widgets */
    HToolkitAppList *listbox;

} HToolkitWindowPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (HToolkitWindow, htoolkit_window, GTK_TYPE_APPLICATION_WINDOW)

#if 0
static void
htoolkit_window_check_package (HToolkitWindow *win, gchar* package)
{
    gchar* msg;
    //gchar *package;
    gchar* filename;

    HToolkitWindowPrivate *priv = htoolkit_window_get_instance_private (win);

    package = htoolkit_window_view_get_package (priv->view);
    if (check_package (package))
    {
#if 0
        filename = htoolkit_window_view_get_file_name (priv->view);
        if (check_version (package, filename))
        {
            msg = g_strdup (_("Update to the hancom 2020 Viewer Beta"));
            gtk_label_set_text (priv->update_label, msg);
            gtk_widget_show (GTK_WIDGET (priv->update_label));
        }
        else
#endif
        {
            msg = g_strdup (_("Hangul 2020 Viewer Beta is installed"));
            gtk_label_set_text (priv->error_label, msg);
            gtk_stack_set_visible_child (GTK_STACK (priv->bar_stack), priv->end_bar);
        }
    }
}
#endif

static void
htoolkit_window_shutdown_cb (HToolkitAppList *list, gpointer data)
{
    g_signal_emit_by_name (GTK_WINDOW (data), "shutdown");
}

static gboolean 
htoolkit_window_timeout_shutdown_cb (gpointer user_data)
{
    g_signal_emit_by_name (GTK_WINDOW (user_data), "shutdown");

    return G_SOURCE_REMOVE;
}

static void
htoolkit_window_app_load (HToolkitWindow *win)
{
    HToolkitWindowPrivate *priv = htoolkit_window_get_instance_private (win);

    GError *error = NULL;

    gsize size;
    gsize byte_read;
    guchar *buffer_data;

    GInputStream *stream;

    g_resources_get_info (PACKAGE_RESOURCE_PATH, 0, &size, NULL, &error);
    g_assert_no_error (error);

    buffer_data = g_new (guchar, size + 1);

    stream = g_resources_open_stream (PACKAGE_RESOURCE_PATH, G_RESOURCE_LOOKUP_FLAGS_NONE, &error);

    g_assert (stream != NULL);
    g_assert_no_error (error);

    if  (!g_input_stream_read_all (stream, buffer_data, size, &byte_read, NULL, &error))
    {
        g_assert_no_error (error);
        goto out;
    }

    buffer_data [byte_read] = '\0';

    gboolean shutdown = TRUE;

    JsonNode *json_root;
    JsonNode *json_node;
    JsonObject *json_item;

    g_autoptr (JsonParser) json_parser = NULL;

    json_parser = json_parser_new ();
    if (!json_parser_load_from_data (json_parser, (gchar*)buffer_data, -1, &error))
        goto out;

    json_root = json_parser_get_root (json_parser);
    if (json_root == NULL)
        goto out;

    if (json_node_get_node_type (json_root) != JSON_NODE_OBJECT)
        goto out;

    json_item = json_node_get_object (json_root);
    if (json_item == NULL)
        goto out;

    if (json_object_has_member (json_item, "packages"))
    {
        int i;
        JsonArray *array = json_object_get_array_member (json_item, "packages");
        for (i = 0; i < json_array_get_length (array); i++)
        {
            json_item = json_array_get_object_element (array, i);
            if (json_item == NULL)
                continue;

            json_node = json_object_get_member (json_item, "package");
            if (json_node == NULL)
                continue;

            gchar* package;
            HToolkitApp *app;

            package = g_strdup (json_node_get_string (json_node));
            if (check_package (package))
                continue;

            shutdown = FALSE;

            app = htoolkit_app_new (package);

            gchar* name;
            json_node = json_object_get_member (json_item, "name");
            if (json_node != NULL)
            {
                name = g_strdup (json_node_get_string (json_node));
                htoolkit_app_set_name (app, name);
            }

            gchar* image_file;
            json_node = json_object_get_member (json_item, "image-file");
            if (json_node != NULL)
            {
                image_file = g_strdup (json_node_get_string (json_node));
                htoolkit_app_set_image_from_file (app, image_file);
            }

            gchar* image_resource;
            json_node = json_object_get_member (json_item, "image-resource");
            if (json_node != NULL)
            {
                image_resource = g_strdup (json_node_get_string (json_node));
                htoolkit_app_set_image_from_resource (app, image_resource);
            }

            gchar* url;
            json_node = json_object_get_member (json_item, "url");
            if (json_node != NULL)
            {
                url = g_strdup (json_node_get_string (json_node));
                htoolkit_app_set_uri (app, url);
            }

            gchar* referer;
            json_node = json_object_get_member (json_item, "referer");
            if (json_node != NULL)
            {
                referer = g_strdup (json_node_get_string (json_node));
                htoolkit_app_set_referer (app, referer);
            }

            gchar* dest;
            json_node = json_object_get_member (json_item, "file-name");
            if (json_node != NULL)
            {
                dest = g_strdup (json_node_get_string (json_node));
                htoolkit_app_set_dest (app, dest);
            }

            gchar* md5;
            json_node = json_object_get_member (json_item, "MD5");
            if (json_node != NULL)
            {
                md5 = g_strdup (json_node_get_string (json_node));
                htoolkit_app_set_md5 (app, md5);
            }

            gchar* sha256;
            json_node = json_object_get_member (json_item, "SHA256");
            if (json_node != NULL)
            {
                sha256 = g_strdup (json_node_get_string (json_node));
                htoolkit_app_set_sha256 (app, sha256);
            }

            htoolkit_app_list_add_app (HTOOLKIT_APP_LIST(priv->listbox), app);
        }
    }

out: 
    g_input_stream_close (stream, NULL, &error);

    if (buffer_data)
        g_free (buffer_data);

    if (error != NULL)
        g_error_free (error);
#if 1
    if (shutdown)
    {
        g_timeout_add (100, (GSourceFunc)htoolkit_window_timeout_shutdown_cb, win); 
    }
#endif
}

static void
htoolkit_window_constructed (GObject *self)
{
    G_OBJECT_CLASS (htoolkit_window_parent_class)->constructed (self);
}

static void
htoolkit_window_dispose (GObject *self)
{
    G_OBJECT_CLASS (htoolkit_window_parent_class)->dispose (self);
}

static void
htoolkit_window_finalize (GObject *self)
{
    G_OBJECT_CLASS (htoolkit_window_parent_class)->finalize (self);
}

static void
htoolkit_window_class_init (HToolkitWindowClass *klass)
{
    GObjectClass *oclass = G_OBJECT_CLASS (klass);
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

    oclass->dispose = htoolkit_window_dispose;
    oclass->finalize = htoolkit_window_finalize;
    oclass->constructed = htoolkit_window_constructed;

    gtk_widget_class_set_template_from_resource (widget_class,
                                                 "/kr/hancom/hancom-toolkit/hancom-toolkit-window.ui");
    gtk_widget_class_bind_template_child_private (widget_class, HToolkitWindow, listbox);

    g_signal_new("shutdown",
                 G_TYPE_FROM_CLASS(klass),
                 G_SIGNAL_RUN_FIRST,
                 0,
                 NULL,
                 NULL,
                 g_cclosure_marshal_VOID__VOID,
                 G_TYPE_NONE,
                 0 );
}

static void
htoolkit_window_init (HToolkitWindow *self)
{
    gtk_widget_init_template (GTK_WIDGET (self));

    htoolkit_window_app_load (self);

    gtk_window_set_keep_above (GTK_WINDOW (self), 1);

    HToolkitWindowPrivate *priv = htoolkit_window_get_instance_private (self);

    g_signal_connect (priv->listbox,
                      "shutdown",
                      G_CALLBACK (htoolkit_window_shutdown_cb),
                      self);
}
