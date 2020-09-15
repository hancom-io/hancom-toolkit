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

#include <gio/gio.h>
#include <glib/gi18n.h>
#include <glib/gstdio.h>
#include <curl/curl.h>
#include <json-glib/json-glib.h>

#include "define.h"
#include "utils.h"
#include "hancom-toolkit-config.h"
#include "htoolkit-app.h"
#include "htoolkit-controller.h"

#define OUT_PATH "/var/tmp"

typedef struct 
{
    guint      download_work_id;
    gboolean   download_working;
    GQueue    *download_packages;

}HToolkitControllerPrivate;

static GMutex thread_mutex;

static gboolean htoolkit_controller_download_job (gpointer user_data);

G_DEFINE_TYPE_WITH_PRIVATE (HToolkitController, htoolkit_controller, G_TYPE_OBJECT)

void
htoolkit_controller_add_package (HToolkitController *control, HToolkitApp *app)
{
    HToolkitControllerPrivate *priv;
    priv = htoolkit_controller_get_instance_private (control);

    GQueue *packages = priv->download_packages;

    g_queue_push_head (packages, app);

    if (priv->download_working == 0 )
    {
        priv->download_working = 1;
        if (0 < priv->download_work_id)
        {
            g_source_remove (priv->download_work_id);
            priv->download_work_id = 0;
        }

        priv->download_work_id = g_timeout_add (500,
                                               (GSourceFunc)htoolkit_controller_download_job,
                                               control);
    }
}

static void
htoolkit_controller_install (HToolkitApp *app, gchar *file)
{
    gchar **args;
    GError *error = NULL;
    gboolean update;
    g_autofree gchar *package;

    package = htoolkit_app_get_package (app);

    update = htoolkit_app_get_update (app);
    if (update)
    {
        g_autofree gchar *tmp;
        g_autofree gchar *packages;
        g_autofree gchar *check;
        g_autofree gchar *delete_command;
        g_autoptr(GString) str = g_string_new (NULL);

        tmp = NULL;
        check = htoolkit_app_get_check_package (app);
        if (check)
        {
            if (check_package(check))
            {
                tmp = g_strdup (check);
            }
        }

        if (check_package (package))
        {
            packages = g_strdup_printf ("%s %s", package, tmp);
        }
        else
        {
            packages = g_strdup (tmp);
        }

        delete_command = g_strdup_printf ("pkexec %s/%s/%s 0 %s", LIBDIR, GETTEXT_PACKAGE, HTOOLKIT_SCRIPT, packages);

        args = g_strsplit (delete_command, " ", -1);

        if (!g_spawn_sync (NULL, args, NULL, G_SPAWN_SEARCH_PATH, NULL, NULL, NULL, NULL, NULL, &error))
        {
            htoolkit_app_set_error_msg (app, error->message);
            g_object_set (G_OBJECT (app), "state", STATE_ERROR, NULL);

            g_error_free (error);
        }
        error = NULL;
    }

    g_autofree gchar *command;
    command = g_strdup_printf ("pkexec %s/%s/%s 1 %s", LIBDIR, GETTEXT_PACKAGE, HTOOLKIT_SCRIPT, file);

    args = g_strsplit (command, " ", -1);

    if (!g_spawn_sync (NULL, args, NULL, G_SPAWN_SEARCH_PATH, NULL, NULL, NULL, NULL, NULL, &error))
    {
        htoolkit_app_set_error_msg (app, error->message);
        g_object_set (G_OBJECT (app), "state", STATE_ERROR, NULL);

        g_error_free (error);
    }
    else
    {
        if (check_package (package))
        {
            g_object_set (G_OBJECT (app), "state", STATE_INSTALLED, NULL);
        }
        else
        {
            g_autofree gchar *msg;
            if (error)
                msg = g_strdup (error->message);
            else
                msg = g_strdup (_("Installation of Application is failed"));

            htoolkit_app_set_error_msg (app, msg);
            g_object_set (G_OBJECT (app), "state", STATE_ERROR, NULL);
        }
    }

    gchar *dir;
    dir = g_path_get_dirname (file);

    unlink (file);
    g_rmdir (dir);
    g_free (dir);
    g_free (file);
    g_strfreev (args);
}

static void
htoolkit_controller_install_work (gpointer user_data)
{
    HToolkitApp *app;
    app = HTOOLKIT_APP (user_data);

    gchar *out_file;
    g_object_set (G_OBJECT (app), "state", STATE_INSTALLING, NULL);

    out_file = htoolkit_app_get_download_path (app);

    if (!g_file_test (out_file, G_FILE_TEST_EXISTS))
        return;

    g_debug ("Install Thread Mutex Lock Start");
    g_object_set (G_OBJECT (app), "state", STATE_INSTALLING, NULL);
    g_mutex_lock (&thread_mutex);
    htoolkit_controller_install (app, out_file);

    g_mutex_unlock (&thread_mutex);
    g_debug ("Install Thread Mutex Lock End");
}

static size_t
htoolkit_controller_download_write (void *ptr, size_t size, size_t nmemb, FILE *stream)
{
    size_t written = fwrite(ptr, size, nmemb, stream);
    return written;
}

static int
htoolkit_controller_download_progress (void *user_data,
                          curl_off_t dltotal, curl_off_t dlnow,
                          curl_off_t ultotal, curl_off_t ulnow)
{
    g_return_val_if_fail (HTOOLKIT_APP (user_data), 0);

    HToolkitApp *app = HTOOLKIT_APP (user_data);

    gint state;
    state = htoolkit_app_get_state (app);

    if (state == STATE_ERROR)
        return 1;

    guint p;
    guint app_progress;

    p = ((double)dlnow / (double)dltotal ) * 100;
    app_progress = htoolkit_app_get_progress (app);

    if (app_progress != p)
    {
        app_progress = p;
        g_object_set (G_OBJECT (app), "progress", app_progress, NULL);
    }

   return 0;
}

static size_t
htoolkit_controller_download_check_cb (char* buffer, size_t size, size_t nmemb, void *app)
{
    g_return_val_if_fail (HTOOLKIT_APP (app), 0);
    g_autofree gchar *app_sha256;
    app_sha256 = htoolkit_app_get_sha256 (app);

    if (!app_sha256 || 0 == strlen (app_sha256))
    {
        htoolkit_app_set_valid (app, TRUE);
        return nmemb * size;
    }

    if (buffer)
    {
        gchar **data;
        gchar *sha256;

        data = g_strsplit (buffer, ":", -1);

        if (g_str_has_suffix (data[0], "Checksum"))
        {
            sha256 = g_strstrip (data[1]);

            if (g_strcmp0 (sha256, app_sha256) == 0)
            {
                htoolkit_app_set_valid (app, TRUE);
            }
        }

        if (data)
        {
            g_strfreev (data);
        }
    }
    return nmemb * size;
}

static gboolean
htoolkit_controller_download_check (HToolkitApp  *app, gchar* uri)
{
    CURL *curl;
    CURLcode res;
    gboolean result = FALSE;

    g_autofree gchar  *referer;
    g_autofree gchar  *md5;

    referer = htoolkit_app_get_referer (app);
    md5 = htoolkit_app_get_md5 (app);

    curl = curl_easy_init ();
    if (curl)
    {
        curl_easy_setopt(curl, CURLOPT_URL, uri);
        curl_easy_setopt(curl, CURLOPT_REFERER, referer);
        if (md5 && 0 < strlen (md5))
            curl_easy_setopt(curl, CURLOPT_SSH_HOST_PUBLIC_KEY_MD5, md5);
        curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);
        curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, htoolkit_controller_download_check_cb);
        curl_easy_setopt(curl, CURLOPT_HEADERDATA, app);

        res = curl_easy_perform(curl);
    }

    if (res == CURLE_OK)
    {
        result = htoolkit_app_get_valid (app);
    }

    curl_easy_cleanup(curl);

    return result;
}

static gpointer
htoolkit_controller_download_func (HToolkitApp *app)
{
    g_return_val_if_fail (HTOOLKIT_APP (app), NULL);
    FILE *fp;
    CURL *curl;
    CURLcode res;

    g_autofree gchar *uri;
    g_autofree gchar *out_file;

    g_autofree gchar *url;
    g_autofree gchar *dest;
    g_autofree gchar *md5;
    g_autofree gchar *referer;

    url  = htoolkit_app_get_uri (app);
    dest = htoolkit_app_get_dest (app);
    md5  = htoolkit_app_get_md5 (app);
    referer = htoolkit_app_get_referer (app);

    uri = g_strdup_printf ("%s/%s", url, dest);
    
    out_file = htoolkit_app_get_download_path (app);

    curl = curl_easy_init ();
    if (curl)
    {
        fp = fopen (out_file, "wb");

        curl_easy_setopt(curl, CURLOPT_URL, uri);
        curl_easy_setopt(curl, CURLOPT_USERNAME, "HancomGooroom");
        curl_easy_setopt(curl, CURLOPT_REFERER, referer);

        if (md5 && 0 < strlen (md5))
            curl_easy_setopt(curl, CURLOPT_SSH_HOST_PUBLIC_KEY_MD5, md5);

        curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, htoolkit_controller_download_progress);
        curl_easy_setopt(curl, CURLOPT_XFERINFODATA, app);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, htoolkit_controller_download_write);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
        res = curl_easy_perform(curl);

        curl_easy_cleanup(curl);
        fclose(fp);
    }

    if (res != CURLE_OK)
    {
        g_autofree gchar *dir;
        dir = g_path_get_dirname (out_file);
        unlink (out_file);
        g_rmdir (dir);

        gint state;
        state = htoolkit_app_get_state (app);
        if (state == STATE_ERROR)
            return NULL;

        g_autofree gchar *msg;
        msg = g_strdup (_("Error, Download"));
        htoolkit_app_set_error_msg (app, msg);
        g_object_set (G_OBJECT (app), "state", STATE_ERROR, NULL);
    }
    else
    {
        g_object_set (G_OBJECT (app), "state", STATE_DOWNLOADED, NULL);

        g_usleep (10000);
        htoolkit_controller_install_work (app);
    }

    return NULL;
}

static void
htoolkit_controller_download (HToolkitApp *app)
{
    gboolean is_connected;

    GNetworkMonitor *monitor = g_network_monitor_get_default();
    is_connected = g_network_monitor_get_network_available (monitor);

    if (!is_connected)
    {
        g_autofree gchar* msg;
        msg = g_strdup (_("Network is not active"));
        htoolkit_app_set_error_msg (app, msg);
        g_object_set (G_OBJECT (app), "state", STATE_ERROR, NULL);
        return;
    }

    g_autofree gchar *out_file;
    g_autofree gchar *dest;
    g_autofree gchar *uri;
    g_autofree gchar *uri_full;
    g_autofree gchar *package;

    uri = htoolkit_app_get_uri (app); 
    dest = htoolkit_app_get_dest (app);
    package = htoolkit_app_get_package (app);

    g_autofree gchar *dir;
    dir = g_build_filename (OUT_PATH, package, NULL);
    g_mkdir_with_parents (dir, 0700);
    out_file = g_build_filename (dir, dest, NULL);
    htoolkit_app_set_download_path (app, out_file);
    g_debug ("Downloaod Dest File  : %s\n", out_file);

    if (g_file_test (out_file, G_FILE_TEST_EXISTS))
    {
        unlink (out_file);
    }

    uri_full = g_build_filename (uri, dest, NULL);
    if (!htoolkit_controller_download_check (app, uri_full))
    {
        g_autofree gchar* error;
        error = g_strdup (_("File is not valid"));
        htoolkit_app_set_error_msg (app, error);
        g_object_set (G_OBJECT (app), "state", STATE_ERROR, NULL);
        unlink (out_file);
        g_rmdir (dir);
        return;
    }

    g_object_set (G_OBJECT (app), "state", STATE_DOWNLOADING, NULL);

    GThread *thread;
    thread = g_thread_new ("hancom-toolkit-download", (GThreadFunc)htoolkit_controller_download_func, app);
    htoolkit_app_set_thread (app, thread);
}

static gboolean
htoolkit_controller_download_job (gpointer user_data)
{
    HToolkitApp *app;
    HToolkitController *ctrl;
    HToolkitControllerPrivate *priv;

    ctrl = HTOOLKIT_CONTROLLER (user_data);
    priv = htoolkit_controller_get_instance_private (ctrl);

    while ((app = g_queue_pop_head (priv->download_packages)) != NULL)
    {
        htoolkit_controller_download (app);
    }

    if (g_queue_is_empty (priv->download_packages))
    {
        priv->download_working = 0;
        priv->download_work_id = 0;
        return G_SOURCE_REMOVE;
    }
    return G_SOURCE_CONTINUE;
}

static void
htoolkit_controller_dispose (GObject *object)
{
    g_return_if_fail (HTOOLKIT_CONTROLLER (object));
    HToolkitController *view;
    HToolkitControllerPrivate *priv;

    view = HTOOLKIT_CONTROLLER (object);
    priv = htoolkit_controller_get_instance_private (view);

    if (0 < priv->download_work_id)
    {
        g_source_remove (priv->download_work_id);
        priv->download_work_id = 0;
    }

    if (priv->download_packages)
    {
        g_queue_clear (priv->download_packages);
        g_queue_free (priv->download_packages);
        priv->download_packages = NULL;
    }

    g_mutex_clear (&thread_mutex);
    G_OBJECT_CLASS (htoolkit_controller_parent_class)->dispose (object);
}

static void
htoolkit_controller_finalize (GObject *object)
{
    G_OBJECT_CLASS (htoolkit_controller_parent_class)->finalize (object);
}

static void
htoolkit_controller_class_init (HToolkitControllerClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);
    object_class->dispose = htoolkit_controller_dispose;
    object_class->finalize = htoolkit_controller_finalize;
}

static void
htoolkit_controller_init (HToolkitController *self)
{
    HToolkitControllerPrivate *priv = htoolkit_controller_get_instance_private (self);
    priv->download_work_id = 0;
    priv->download_working = FALSE;
    priv->download_packages = g_queue_new ();

    g_mutex_init (&thread_mutex);
}

HToolkitController *
htoolkit_controller_new (void)
{
    HToolkitController *ctrl;
    ctrl = g_object_new (HTOOLKIT_TYPE_CONTROLLER, NULL);
    return ctrl;
}
