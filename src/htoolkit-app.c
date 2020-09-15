/* htoolkit-app.c
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

#include "hancom-toolkit-config.h"
#include "htoolkit-app.h"
#include "define.h"

typedef struct
{
    GObject       parent_instance;

    gchar        *error;

    gchar        *package;
    gchar        *app_name;
    gchar        *app_image_resource;
    gchar        *app_image_file;

    gchar        *dw_uri;
    gchar        *dw_referer;
    gchar        *dw_dest;
    gchar        *dw_md5;
    gchar        *dw_sha256;

    gchar        *out_path;

    guint         state;
    guint         progress;

    gboolean      valid;
    gboolean      installed;

    GThread       *download_thread;
} HToolkitAppPrivate;

enum {
    PROP_PACKAGE = 1,
    PROP_NAME,
    PROP_IMAGE_FILE,
    PROP_IMAGE_RESOURCE,
    PROP_URI,
    PROP_REFERER,
    PROP_DEST,
    PROP_MD5,
    PROP_SHA256,
    PROP_STATE,
    PROP_PROGRESS,
    PROP_INSTALLED,
    PROP_LAST
};
G_DEFINE_TYPE_WITH_PRIVATE (HToolkitApp, htoolkit_app, G_TYPE_OBJECT)

void
htoolkit_app_set_download_path (HToolkitApp *app, const gchar* path)
{
    HToolkitAppPrivate *priv = htoolkit_app_get_instance_private (app);
    g_return_if_fail (HTOOLKIT_IS_APP (app));

    priv->out_path = g_strdup (path);
}

void
htoolkit_app_set_error_msg (HToolkitApp *app, const gchar* msg)
{
    HToolkitAppPrivate *priv = htoolkit_app_get_instance_private (app);
    g_return_if_fail (HTOOLKIT_IS_APP (app));

    priv->error = g_strdup (msg);
}

void
htoolkit_app_set_package (HToolkitApp *app, const gchar* package)
{
    HToolkitAppPrivate *priv = htoolkit_app_get_instance_private (app);
    g_return_if_fail (HTOOLKIT_IS_APP (app));

    priv->package = g_strdup (package);
}

void
htoolkit_app_set_name (HToolkitApp *app, const gchar* name)
{
    HToolkitAppPrivate *priv = htoolkit_app_get_instance_private (app);
    g_return_if_fail (HTOOLKIT_IS_APP (app));

    priv->app_name = g_strdup (name);
}

void
htoolkit_app_set_image_from_file (HToolkitApp *app, const gchar* image)
{
    HToolkitAppPrivate *priv = htoolkit_app_get_instance_private (app);
    g_return_if_fail (HTOOLKIT_IS_APP (app));

    priv->app_image_file = g_strdup (image);
}

void
htoolkit_app_set_image_from_resource (HToolkitApp *app, const gchar* image)
{
    HToolkitAppPrivate *priv = htoolkit_app_get_instance_private (app);
    g_return_if_fail (HTOOLKIT_IS_APP (app));

    priv->app_image_resource = g_strdup (image);
}

void
htoolkit_app_set_uri (HToolkitApp *app, const gchar* uri)
{
    HToolkitAppPrivate *priv = htoolkit_app_get_instance_private (app);
    g_return_if_fail (HTOOLKIT_IS_APP (app));

    priv->dw_uri = g_strdup (uri);
}

void
htoolkit_app_set_referer (HToolkitApp *app, const gchar* uri)
{
    HToolkitAppPrivate *priv = htoolkit_app_get_instance_private (app);
    g_return_if_fail (HTOOLKIT_IS_APP (app));

    priv->dw_referer = g_strdup (uri);
}

void
htoolkit_app_set_dest (HToolkitApp *app, const gchar* dest)
{
    HToolkitAppPrivate *priv = htoolkit_app_get_instance_private (app);
    g_return_if_fail (HTOOLKIT_IS_APP (app));

    priv->dw_dest = g_strdup (dest);
}

void
htoolkit_app_set_md5 (HToolkitApp *app, const gchar* md5)
{
    HToolkitAppPrivate *priv = htoolkit_app_get_instance_private (app);
    g_return_if_fail (HTOOLKIT_IS_APP (app));

    priv->dw_md5 = g_strdup (md5);
}

void
htoolkit_app_set_sha256 (HToolkitApp *app, const gchar* sha256)
{
    HToolkitAppPrivate *priv = htoolkit_app_get_instance_private (app);
    g_return_if_fail (HTOOLKIT_IS_APP (app));

    priv->dw_sha256 = g_strdup (sha256);
}

void
htoolkit_app_set_state (HToolkitApp *app, guint state)
{
    HToolkitAppPrivate *priv = htoolkit_app_get_instance_private (app);
    //g_return_if_fail (HTOOLKIT_IS_APP (app));
    priv->state = state;
}

void
htoolkit_app_set_progress  (HToolkitApp *app, guint progress)
{
    HToolkitAppPrivate *priv = htoolkit_app_get_instance_private (app);
    g_return_if_fail (HTOOLKIT_IS_APP (app));

    priv->progress = progress;
}

void
htoolkit_app_set_installed (HToolkitApp *app, gboolean installed)
{
    HToolkitAppPrivate *priv = htoolkit_app_get_instance_private (app);
    g_return_if_fail (HTOOLKIT_IS_APP (app));

    priv->installed = installed;
}

void
htoolkit_app_set_valid (HToolkitApp *app, gboolean valid)
{
    HToolkitAppPrivate *priv = htoolkit_app_get_instance_private (app);
    g_return_if_fail (HTOOLKIT_IS_APP (app));

    priv->valid = valid;
}

void
htoolkit_app_set_thread (HToolkitApp *app, GThread *thread)
{
    HToolkitAppPrivate *priv = htoolkit_app_get_instance_private (app);
    g_return_if_fail (HTOOLKIT_IS_APP (app));

    priv->download_thread = thread;
}

gchar*
htoolkit_app_get_download_path (HToolkitApp *app)
{
    g_return_val_if_fail (HTOOLKIT_IS_APP (app), NULL);
    HToolkitAppPrivate *priv = htoolkit_app_get_instance_private (app);

    return g_strdup (priv->out_path);
}

gchar*
htoolkit_app_get_error_msg (HToolkitApp *app)
{
    g_return_val_if_fail (HTOOLKIT_IS_APP (app), NULL);
    HToolkitAppPrivate *priv = htoolkit_app_get_instance_private (app);

    return g_strdup (priv->error);
}

gchar*
htoolkit_app_get_package (HToolkitApp *app)
{
    g_return_val_if_fail (HTOOLKIT_IS_APP (app), NULL);
    HToolkitAppPrivate *priv = htoolkit_app_get_instance_private (app);

    return g_strdup (priv->package);
}

gchar*
htoolkit_app_get_name (HToolkitApp *app)
{
    g_return_val_if_fail (HTOOLKIT_IS_APP (app), NULL);
    HToolkitAppPrivate *priv = htoolkit_app_get_instance_private (app);

    return g_strdup (priv->app_name);
}

gchar*
htoolkit_app_get_image_from_file (HToolkitApp *app)
{
    g_return_val_if_fail (HTOOLKIT_IS_APP (app), NULL);
    HToolkitAppPrivate *priv = htoolkit_app_get_instance_private (app);

    return g_strdup (priv->app_image_file);
}

gchar*
htoolkit_app_get_image_from_resource (HToolkitApp *app)
{
    g_return_val_if_fail (HTOOLKIT_IS_APP (app), NULL);
    HToolkitAppPrivate *priv = htoolkit_app_get_instance_private (app);

    return g_strdup (priv->app_image_resource);
}

gchar*
htoolkit_app_get_uri (HToolkitApp *app)
{
    g_return_val_if_fail (HTOOLKIT_IS_APP (app), NULL);
    HToolkitAppPrivate *priv = htoolkit_app_get_instance_private (app);

    return g_strdup (priv->dw_uri);
}

gchar*
htoolkit_app_get_referer (HToolkitApp *app)
{
    g_return_val_if_fail (HTOOLKIT_IS_APP (app), NULL);
    HToolkitAppPrivate *priv = htoolkit_app_get_instance_private (app);

    return g_strdup (priv->dw_referer);
}

gchar*
htoolkit_app_get_dest (HToolkitApp *app)
{
    g_return_val_if_fail (HTOOLKIT_IS_APP (app), NULL);
    HToolkitAppPrivate *priv = htoolkit_app_get_instance_private (app);

    return g_strdup (priv->dw_dest);
}

gchar*
htoolkit_app_get_md5 (HToolkitApp *app)
{
    g_return_val_if_fail (HTOOLKIT_IS_APP (app), NULL);
    HToolkitAppPrivate *priv = htoolkit_app_get_instance_private (app);

    return g_strdup (priv->dw_md5);
}

gchar*
htoolkit_app_get_sha256 (HToolkitApp *app)
{
    g_return_val_if_fail (HTOOLKIT_IS_APP (app), NULL);
    HToolkitAppPrivate *priv = htoolkit_app_get_instance_private (app);

    return g_strdup (priv->dw_sha256);
}

guint
htoolkit_app_get_state (HToolkitApp *app)
{
    g_return_val_if_fail (HTOOLKIT_IS_APP (app), STATE_NORMAL);
    HToolkitAppPrivate *priv = htoolkit_app_get_instance_private (app);

    return priv->state;
}

guint
htoolkit_app_get_progress (HToolkitApp *app)
{
    g_return_val_if_fail (HTOOLKIT_IS_APP (app), 0);
    HToolkitAppPrivate *priv = htoolkit_app_get_instance_private (app);

    return priv->progress;
}

gboolean
htoolkit_app_get_installed (HToolkitApp *app)
{
    g_return_val_if_fail (HTOOLKIT_IS_APP (app), FALSE);
    HToolkitAppPrivate *priv = htoolkit_app_get_instance_private (app);

    return priv->installed;
}

gboolean
htoolkit_app_get_valid (HToolkitApp *app)
{
    g_return_val_if_fail (HTOOLKIT_IS_APP (app), FALSE);
    HToolkitAppPrivate *priv = htoolkit_app_get_instance_private (app);

    return priv->valid;
}

GThread*
htoolkit_app_get_thread (HToolkitApp *app)
{
    g_return_val_if_fail (HTOOLKIT_IS_APP (app), FALSE);
    HToolkitAppPrivate *priv = htoolkit_app_get_instance_private (app);

    return priv->download_thread;
}

static void
htoolkit_app_get_property (GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
    HToolkitApp *app = HTOOLKIT_APP (object);
    HToolkitAppPrivate *priv = htoolkit_app_get_instance_private (app);

    switch (prop_id)
    {
    case PROP_PACKAGE:
        g_value_set_string (value, priv->package);
        break;
    case PROP_NAME:
        g_value_set_string (value, priv->app_name);
        break;
    case PROP_IMAGE_FILE:
        g_value_set_string (value, priv->app_image_file);
        break;
    case PROP_IMAGE_RESOURCE:
        g_value_set_string (value, priv->app_image_resource);
        break;
    case PROP_URI:
        g_value_set_string (value, priv->dw_uri);
        break;
    case PROP_REFERER:
        g_value_set_string (value, priv->dw_referer);
        break;
    case PROP_MD5:
        g_value_set_string (value, priv->dw_md5);
        break;
    case PROP_SHA256:
        g_value_set_string (value, priv->dw_sha256);
        break;
    case PROP_STATE:
        g_value_set_uint(value, priv->state);
        break;
    case PROP_PROGRESS:
        g_value_set_uint(value, priv->progress);
        break;
    case PROP_INSTALLED:
        g_value_set_boolean (value, priv->installed);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void
htoolkit_app_set_property (GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
    HToolkitApp *app = HTOOLKIT_APP (object);
    HToolkitAppPrivate *priv = htoolkit_app_get_instance_private (app);

    switch (prop_id)
    {
    case PROP_PACKAGE:
        priv->package = g_strdup (g_value_get_string (value));
        break;
    case PROP_NAME:
        priv->app_name = g_strdup (g_value_get_string (value));
        break;
    case PROP_IMAGE_FILE:
        priv->app_image_file = g_strdup (g_value_get_string (value));
        break;
    case PROP_IMAGE_RESOURCE:
        priv->app_image_resource = g_strdup (g_value_get_string (value));
        break;
    case PROP_URI:
        priv->dw_uri = g_strdup (g_value_get_string (value));
        break;
    case PROP_REFERER:
        priv->dw_referer = g_strdup (g_value_get_string (value));
        break;
    case PROP_MD5:
        priv->dw_md5 = g_strdup (g_value_get_string (value));
        break;
    case PROP_SHA256:
        priv->dw_sha256 = g_strdup (g_value_get_string (value));
        break;
    case PROP_STATE:
        priv->state = g_value_get_uint (value);
        break;
    case PROP_PROGRESS:
        priv->progress = g_value_get_uint (value);
        break;
    case PROP_INSTALLED:
        priv->installed = g_value_get_boolean (value);
        break;
    default:
        //G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void
htoolkit_app_dispose (GObject *object)
{
    G_OBJECT_CLASS (htoolkit_app_parent_class)->dispose (object);
}

static void
htoolkit_app_finalize (GObject *object)
{
    HToolkitApp *app = HTOOLKIT_APP (object);
    HToolkitAppPrivate *priv = htoolkit_app_get_instance_private (app);

    g_print ("%s\n", __func__);

    g_free (priv->error);

    g_free (priv->package);
    g_free (priv->app_name);
    g_free (priv->app_image_file);
    g_free (priv->app_image_resource);

    g_free (priv->dw_uri);
    g_free (priv->dw_referer);
    g_free (priv->dw_dest);
    g_free (priv->dw_md5);
    g_free (priv->dw_sha256);

    if (priv->out_path)
    {
        unlink (priv->out_path);
        g_free (priv->out_path);
    }

    if (priv->download_thread)
    {
        g_thread_unref (priv->download_thread);
        priv->download_thread = NULL;
    }

    G_OBJECT_CLASS (htoolkit_app_parent_class)->finalize (object);
}

static void
htoolkit_app_class_init (HToolkitAppClass *klass)
{
    GParamSpec *pspec;
    GObjectClass *object_class = G_OBJECT_CLASS (klass);
    object_class->dispose = htoolkit_app_dispose;
    object_class->finalize = htoolkit_app_finalize;
    object_class->get_property = htoolkit_app_get_property;
    object_class->set_property = htoolkit_app_set_property;

    pspec = g_param_spec_string ("package", NULL, NULL,
                     NULL,
                     G_PARAM_READWRITE | G_PARAM_CONSTRUCT);
    g_object_class_install_property (object_class, PROP_PACKAGE, pspec);

    pspec = g_param_spec_string ("app_name", NULL, NULL,
                     NULL,
                     G_PARAM_READWRITE | G_PARAM_CONSTRUCT);
    g_object_class_install_property (object_class, PROP_NAME, pspec);

    pspec = g_param_spec_string ("app_image_file", NULL, NULL,
                     NULL,
                     G_PARAM_READWRITE | G_PARAM_CONSTRUCT);
    g_object_class_install_property (object_class, PROP_IMAGE_FILE, pspec);

    pspec = g_param_spec_string ("app_image_resource", NULL, NULL,
                     NULL,
                     G_PARAM_READWRITE | G_PARAM_CONSTRUCT);
    g_object_class_install_property (object_class, PROP_IMAGE_RESOURCE, pspec);

    pspec = g_param_spec_string ("dw_uri", NULL, NULL,
                     NULL,
                     G_PARAM_READWRITE | G_PARAM_CONSTRUCT);
    g_object_class_install_property (object_class, PROP_URI, pspec);

    pspec = g_param_spec_string ("dw_referer", NULL, NULL,
                     NULL,
                     G_PARAM_READWRITE | G_PARAM_CONSTRUCT);
    g_object_class_install_property (object_class, PROP_REFERER, pspec);

    pspec = g_param_spec_string ("dw_dest", NULL, NULL,
                     NULL,
                     G_PARAM_READWRITE | G_PARAM_CONSTRUCT);
    g_object_class_install_property (object_class, PROP_DEST, pspec);

    pspec = g_param_spec_string ("dw_md5", NULL, NULL,
                     NULL,
                     G_PARAM_READWRITE | G_PARAM_CONSTRUCT);
    g_object_class_install_property (object_class, PROP_MD5, pspec);

    pspec = g_param_spec_string ("dw_sha256", NULL, NULL,
                     NULL,
                     G_PARAM_READWRITE | G_PARAM_CONSTRUCT);
    g_object_class_install_property (object_class, PROP_SHA256, pspec);

    pspec = g_param_spec_uint ("state", NULL, NULL,
                     STATE_NORMAL,
                     STATE_ERROR,
                     STATE_NORMAL,
                     G_PARAM_READWRITE | G_PARAM_CONSTRUCT);
    g_object_class_install_property (object_class, PROP_STATE, pspec);

    pspec = g_param_spec_uint ("progress", NULL, NULL,
                     0, 100, 0,
                     G_PARAM_READWRITE | G_PARAM_CONSTRUCT);
    g_object_class_install_property (object_class, PROP_PROGRESS, pspec);

    pspec = g_param_spec_boolean ("installed", NULL, NULL,
                     FALSE,
                     G_PARAM_READWRITE | G_PARAM_CONSTRUCT);
    g_object_class_install_property (object_class, PROP_INSTALLED, pspec);
}

static void
htoolkit_app_init (HToolkitApp *app)
{
    HToolkitAppPrivate *priv = htoolkit_app_get_instance_private (app);

    priv->error = NULL;

    priv->package = NULL;
    priv->app_name = NULL;
    priv->app_image_file = NULL;
    priv->app_image_resource = NULL;
    priv->dw_uri = NULL;
    priv->dw_referer = NULL;
    priv->dw_dest = NULL;
    priv->dw_md5 = NULL;
    priv->dw_sha256 = NULL;
    
    priv->out_path = NULL;

    priv->state = STATE_NORMAL;
    priv->progress = 0;

    priv->installed = FALSE;
    priv->valid = FALSE;

    priv->download_thread = NULL;
}

/*
 * @id: an application ID, or %NULL, e.g. "kr.hancom.viewer.desktop"
*/
HToolkitApp*
htoolkit_app_new (const gchar *package)
{
    HToolkitApp *app;
    app = g_object_new (HTOOLKIT_TYPE_APP, "package", package, NULL);
    return HTOOLKIT_APP (app);
}
