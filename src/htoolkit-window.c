/* htoolkit-window.c
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

    int i;
    gboolean is_korean = FALSE;
    const char * const *langs_pointer;
    langs_pointer = g_get_language_names ();
    for (i = 0; langs_pointer[i] != NULL; i++)
    {
        if (g_strcmp0 (langs_pointer[i], "ko") == 0)
        {
            is_korean = TRUE;
            break;
        }
    }

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

            gchar* package = NULL;
            gchar* version = NULL;
            HToolkitApp *app;
            gboolean is_update;

            is_update = FALSE;

            package = g_strdup (json_node_get_string (json_node));

            json_node = json_object_get_member (json_item, "version");
            if (json_node != NULL)
            {
                version = g_strdup (json_node_get_string (json_node));
            }

            gchar* check = NULL;
            json_node = json_object_get_member (json_item, "update-package");
            if (json_node != NULL)
            {
                check = g_strdup (json_node_get_string (json_node));

                if (check_package (check))
                    is_update = TRUE;
            }

            if (!is_update && check_package (package))
            {
                if (!version || strlen (version) == 0)
                    continue;

                if (!check_version (package, version))
                    continue;

                is_update = TRUE;
            }

            gchar* remove = NULL;
            json_node = json_object_get_member (json_item, "remove-package");
            if (json_node != NULL)
            {
                remove = g_strdup (json_node_get_string (json_node));
            }

            shutdown = FALSE;
            app = htoolkit_app_new (package);
            if (is_update)
            {
                htoolkit_app_set_update (app, TRUE);
                if (version)
                {
                    htoolkit_app_set_version (app, version);
                    g_free (version);
                }

                if (check)
                {
                    htoolkit_app_set_update_package (app, check);
                    g_free (check);
                }
            }
            g_free (package);

            if (remove && strlen (remove)!= 0)
            {
                htoolkit_app_set_remove_package (app, remove);
                g_free (remove);
            }

            if (json_object_has_member (json_item, "name"))
            {
                JsonObject *json_subitem;
                json_node = json_object_get_member (json_item, "name");
                json_subitem = json_node_get_object (json_node);

                gchar* name;
                if (is_korean)
                {
                    json_node = json_object_get_member (json_subitem, "ko");
                }
                else
                {
                    json_node = json_object_get_member (json_subitem, "en");
                }

                if (json_node != NULL)
                {
                    name = g_strdup (json_node_get_string (json_node));
                    htoolkit_app_set_name (app, name);
                    g_free (name);
                }
            }

            gchar* image_file;
            json_node = json_object_get_member (json_item, "image-file");
            if (json_node != NULL)
            {
                image_file = g_strdup (json_node_get_string (json_node));
                htoolkit_app_set_image_from_file (app, image_file);
                g_free (image_file);
            }

            gchar* image_resource;
            json_node = json_object_get_member (json_item, "image-resource");
            if (json_node != NULL)
            {
                image_resource = g_strdup (json_node_get_string (json_node));
                htoolkit_app_set_image_from_resource (app, image_resource);
                g_free (image_resource);
            }

            gchar* url;
            json_node = json_object_get_member (json_item, "url");
            if (json_node != NULL)
            {
                url = g_strdup (json_node_get_string (json_node));
                htoolkit_app_set_uri (app, url);
                g_free (url);
            }

            gchar* referer;
            json_node = json_object_get_member (json_item, "referer");
            if (json_node != NULL)
            {
                referer = g_strdup (json_node_get_string (json_node));
                htoolkit_app_set_referer (app, referer);
                g_free (referer);
            }

            gchar* dest;
            json_node = json_object_get_member (json_item, "file-name");
            if (json_node != NULL)
            {
                dest = g_strdup (json_node_get_string (json_node));
                htoolkit_app_set_dest (app, dest);
                g_free (dest);
            }

            gchar* md5;
            json_node = json_object_get_member (json_item, "MD5");
            if (json_node != NULL)
            {
                md5 = g_strdup (json_node_get_string (json_node));
                htoolkit_app_set_md5 (app, md5);
                g_free (md5);
            }

            gchar* sha256;
            json_node = json_object_get_member (json_item, "SHA256");
            if (json_node != NULL)
            {
                sha256 = g_strdup (json_node_get_string (json_node));
                htoolkit_app_set_sha256 (app, sha256);
                g_free (sha256);
            }

            if (json_object_has_member (json_item, "install-message"))
            {
                JsonObject *json_subitem;
                json_node = json_object_get_member (json_item, "install-message");
                json_subitem = json_node_get_object (json_node);

                gchar *message;
                if (is_korean)
                {
                    json_node = json_object_get_member (json_subitem, "ko");
                }
                else
                {
                    json_node = json_object_get_member (json_subitem, "en");
                }

                if (json_node != NULL)
                {
                    message = g_strdup (json_node_get_string (json_node));
                    htoolkit_app_set_install_message (app, message);
                    g_free (message);
                }
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

    if (shutdown)
    {
        g_timeout_add (100, (GSourceFunc)htoolkit_window_timeout_shutdown_cb, win); 
    }
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
