/* htoolkit-app-row.c
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

#include "hancom-toolkit-config.h"
#include "htoolkit-app.h"
#include "htoolkit-app-row.h"
#include "define.h"

typedef struct
{
    HToolkitApp		*app;
    HToolkitController *ctrl;

    GtkWidget       *box;
    GtkWidget       *app_image;
    GtkWidget       *app_label;
    GtkWidget       *error_label;
    GtkWidget       *status_label;
    GtkWidget       *install_label;
    GtkWidget       *install_button;
    GtkWidget       *close_button;

    GtkProgressBar  *progress;

    guint           state;
    guint           status_progress;

} HToolkitAppRowPrivate;

enum {
    PROP_STATE = 1,
    PROP_PROGGRESS = 1,
    PROP_LAST
};

G_DEFINE_TYPE_WITH_PRIVATE (HToolkitAppRow, htoolkit_app_row, GTK_TYPE_LIST_BOX_ROW)

void
htoolkit_app_row_app_init (HToolkitAppRow *app_row)
{
    HToolkitAppRowPrivate *priv = htoolkit_app_row_get_instance_private (app_row);
    HToolkitApp *app = priv->app;

    GdkPixbuf *pixbuf;
    gchar* app_image = NULL;
    app_image = htoolkit_app_get_image_from_file (app);

    if (app_image && strlen (app_image) != 0)
    {
        pixbuf = gdk_pixbuf_new_from_file (app_image, NULL);
    }
    else
    {
        app_image = htoolkit_app_get_image_from_resource (app);
        pixbuf = gdk_pixbuf_new_from_resource (app_image, NULL);
    }

    g_free (app_image);

    gtk_image_set_from_pixbuf (GTK_IMAGE (priv->app_image), pixbuf);

    g_autofree gchar* app_name;
    app_name = htoolkit_app_get_name (app);
    gtk_label_set_text (GTK_LABEL (priv->app_label), app_name);

    if (htoolkit_app_get_update (app))
    {
        gtk_label_set_text (GTK_LABEL (priv->error_label), _("Update version"));
        gtk_widget_show (priv->error_label);
        gtk_label_set_text (GTK_LABEL (priv->install_label), _("Update"));
    }
}

HToolkitApp*
htoolkit_app_row_get_app (HToolkitAppRow *app_row)
{
    g_return_val_if_fail (HTOOLKIT_APP_ROW (app_row), NULL);

    HToolkitAppRowPrivate *priv = htoolkit_app_row_get_instance_private (app_row);
    return priv->app;
}

static gboolean 
htoolkit_app_row_refresh_state_idle (gpointer user_data)
{
    HToolkitAppRowPrivate *priv = htoolkit_app_row_get_instance_private (user_data);

    guint state;
    state = priv->state;

    if (state == STATE_NORMAL)
    {
        gtk_widget_hide (priv->status_label);
        gtk_widget_hide (priv->error_label);
        gtk_widget_show (GTK_WIDGET (priv->install_button));
        gtk_widget_show (GTK_WIDGET (priv->close_button));
    }
    else
    {
        gtk_widget_hide (GTK_WIDGET (priv->install_button));
        gtk_widget_hide (GTK_WIDGET (priv->close_button));
    }

    switch (state)
    {
    case STATE_READY:
      {
        gtk_label_set_text (GTK_LABEL (priv->status_label), _("Waiting"));
        gtk_widget_show (priv->status_label);
        gtk_widget_hide (priv->error_label);

        htoolkit_controller_add_package (priv->ctrl, priv->app);
        break;
      }
    case STATE_DOWNLOADING:
      {
        gtk_label_set_text (GTK_LABEL (priv->status_label), _("Downloading"));
        gtk_widget_show (priv->status_label);
        gtk_widget_show (GTK_WIDGET (priv->progress));
         break;
      }
    case STATE_DOWNLOADED:
      {
        gtk_label_set_text (GTK_LABEL (priv->status_label), _("Downloaded"));
        gtk_widget_show (priv->status_label);
        break;
      }
    case STATE_INSTALLING:
      {
        gtk_label_set_text (GTK_LABEL (priv->status_label), _("Installing"));
        gtk_widget_show (priv->status_label);

        g_autofree gchar *msg;
        msg = htoolkit_app_get_install_message (priv->app);

        if (msg && strlen (msg)!= 0)
        {
            gtk_widget_hide (GTK_WIDGET (priv->progress));

            gtk_label_set_text (GTK_LABEL (priv->error_label), msg);
            gtk_widget_show (priv->error_label);
        }
        break;
      }
    case STATE_INSTALLED :
      {
        #if 0
        gtk_label_set_text (GTK_LABEL (priv->status_label), _("Installed"));
        gtk_widget_show (priv->status_label);
        #endif
        break;
      }
    case STATE_ERROR:
      {
        g_autofree gchar *msg;
        msg = htoolkit_app_get_error_msg (priv->app);
        gtk_label_set_text (GTK_LABEL (priv->error_label), msg);
        gtk_widget_show (priv->error_label);
        gtk_widget_show (GTK_WIDGET (priv->close_button));
        gtk_widget_show (GTK_WIDGET (priv->install_button));

        gtk_widget_hide (priv->status_label);
        gtk_widget_hide (GTK_WIDGET (priv->progress));

        GtkStyleContext *context = gtk_widget_get_style_context (priv->error_label);
        gtk_style_context_add_class (context, "error_label");
        break;
      }
    default :
        break;
    }

    return FALSE;
}

static gboolean
htoolkit_app_row_refresh_progress_idle (gpointer user_data)
{
    gdouble val;
    HToolkitAppRowPrivate *priv;

    priv = htoolkit_app_row_get_instance_private (user_data);
    val = GPOINTER_TO_UINT (priv->status_progress) / 100.f;

    gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (priv->progress), val);
    return FALSE;
}

static void
htoolkit_app_row_app_state_notify_cb (HToolkitApp *app, GParamSpec *pspec, gpointer user_data)
{
    guint state;
    g_object_get (G_OBJECT (app), "state", &state, NULL);

    HToolkitAppRow  *app_row = HTOOLKIT_APP_ROW (user_data);
    HToolkitAppRowPrivate *priv = htoolkit_app_row_get_instance_private (app_row);

    if (state != priv->state)
    {
        g_object_set (G_OBJECT (app_row), "state", state, NULL);
    }
}

static void
htoolkit_app_row_app_progress_notify_cb (HToolkitApp *app, GParamSpec *pspec, gpointer user_data)
{
    guint progress;
    g_object_get (G_OBJECT (app), "progress", &progress, NULL);

    HToolkitAppRow  *app_row = HTOOLKIT_APP_ROW (user_data);
    HToolkitAppRowPrivate *priv = htoolkit_app_row_get_instance_private (app_row);

    if (progress != priv->status_progress)
    {
        priv->status_progress = progress;
        g_idle_add (htoolkit_app_row_refresh_progress_idle, app_row);
    }
}

static void 
htoolkit_app_row_button_install_clicked (GtkWidget *button, HToolkitAppRow *app_row)
{
    g_return_if_fail (HTOOLKIT_APP_ROW (app_row));

    g_object_set (G_OBJECT (app_row), "state", STATE_READY, NULL);
}

static void 
htoolkit_app_row_button_close_clicked (GtkWidget *button, HToolkitAppRow *app_row)
{
    g_return_if_fail (HTOOLKIT_APP_ROW (app_row));

    g_object_set (G_OBJECT (app_row), "state", STATE_CLOSE, NULL);
}

static void
htoolkit_app_row_set_controller (HToolkitAppRow *app_row, HToolkitController *ctrl)
{
    HToolkitAppRowPrivate *priv = htoolkit_app_row_get_instance_private (app_row);
    priv->ctrl = ctrl;
}

static void
htoolkit_app_row_set_app (HToolkitAppRow *app_row, HToolkitApp *app)
{
    HToolkitAppRowPrivate *priv = htoolkit_app_row_get_instance_private (app_row);
    priv->app = app;

    g_signal_connect_object (app, "notify::state",
                             G_CALLBACK (htoolkit_app_row_app_state_notify_cb),
                             app_row, 0);

    g_signal_connect_object (app, "notify::progress",
                             G_CALLBACK (htoolkit_app_row_app_progress_notify_cb),
                             app_row, 0);
}

static void
htoolkit_app_row_dispose (GObject *object)
{
    HToolkitAppRow *app_row = HTOOLKIT_APP_ROW (object);
    HToolkitAppRowPrivate *priv = htoolkit_app_row_get_instance_private (app_row);

    if (priv->app)
    {
        g_clear_object (&priv->app);
    }

    G_OBJECT_CLASS (htoolkit_app_row_parent_class)->dispose (object);
}

static void
htoolkit_app_row_set_property (GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
    HToolkitAppRow *app_row = HTOOLKIT_APP_ROW (object);
    HToolkitAppRowPrivate *priv = htoolkit_app_row_get_instance_private (app_row);

    if (!priv->app)
        return;

    guint val;
    switch (prop_id)
    {
    case PROP_STATE:
      {
        val = g_value_get_uint (value);
        priv->state = val;

        if (val == STATE_CLOSE)
            return; 

        g_idle_add (htoolkit_app_row_refresh_state_idle, app_row);
        break;
      }
    default:
        break;
    }
}

static void
htoolkit_app_row_get_property (GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
    HToolkitAppRow *app_row = HTOOLKIT_APP_ROW (object);
    HToolkitAppRowPrivate *priv = htoolkit_app_row_get_instance_private (app_row);

    switch (prop_id)
    {
    case PROP_STATE:
        g_value_set_uint (value, priv->state);
        break;
    default:
        break;
    }
}

static void
htoolkit_app_row_class_init (HToolkitAppRowClass *klass)
{
    GParamSpec *pspec;
    GObjectClass *object_class = G_OBJECT_CLASS (klass);
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

    object_class->set_property = htoolkit_app_row_set_property;
    object_class->get_property = htoolkit_app_row_get_property;
    object_class->dispose = htoolkit_app_row_dispose;

    pspec = g_param_spec_uint ("state", NULL, NULL, 
                               STATE_NORMAL,
                               STATE_ERROR,
                               STATE_NORMAL, G_PARAM_READWRITE | G_PARAM_CONSTRUCT);
    g_object_class_install_property (object_class, PROP_STATE, pspec);

    gtk_widget_class_set_template_from_resource (widget_class,
                                                "/kr/hancom/hancom-toolkit/hancom-toolkit-view-row.ui");
    gtk_widget_class_bind_template_child_private (widget_class, HToolkitAppRow, box);
    gtk_widget_class_bind_template_child_private (widget_class, HToolkitAppRow, app_image);
    gtk_widget_class_bind_template_child_private (widget_class, HToolkitAppRow, app_label);
    gtk_widget_class_bind_template_child_private (widget_class, HToolkitAppRow, error_label);
    gtk_widget_class_bind_template_child_private (widget_class, HToolkitAppRow, status_label);
    gtk_widget_class_bind_template_child_private (widget_class, HToolkitAppRow, install_label);
    gtk_widget_class_bind_template_child_private (widget_class, HToolkitAppRow, progress);
    gtk_widget_class_bind_template_child_private (widget_class, HToolkitAppRow, install_button);
    gtk_widget_class_bind_template_child_private (widget_class, HToolkitAppRow, close_button);
}

static void
htoolkit_app_row_init (HToolkitAppRow *app_row)
{
    gtk_widget_init_template (GTK_WIDGET (app_row));
    gtk_widget_set_has_window (GTK_WIDGET (app_row), FALSE);

    HToolkitAppRowPrivate *priv = htoolkit_app_row_get_instance_private (app_row);

    priv->state = STATE_NORMAL;
    priv->status_progress = 0;
    priv->app = NULL;

    gtk_widget_hide (GTK_WIDGET (priv->error_label));
    gtk_widget_hide (GTK_WIDGET (priv->status_label));
    gtk_widget_hide (GTK_WIDGET (priv->progress));
    gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (priv->progress), 0);

    g_signal_connect (priv->install_button, "clicked",
                      G_CALLBACK (htoolkit_app_row_button_install_clicked), app_row);

    g_signal_connect (priv->close_button, "clicked",
                      G_CALLBACK (htoolkit_app_row_button_close_clicked), app_row);
}

GtkWidget *
htoolkit_app_row_new (HToolkitApp *app, HToolkitController *ctrl)
{
    GtkWidget *app_row;

    g_return_val_if_fail (HTOOLKIT_IS_APP (app), NULL);

    app_row = g_object_new (HTOOLKIT_TYPE_APP_ROW, NULL);

    htoolkit_app_row_set_app (HTOOLKIT_APP_ROW (app_row), app);
    htoolkit_app_row_set_controller (HTOOLKIT_APP_ROW (app_row), ctrl);
    htoolkit_app_row_app_init (HTOOLKIT_APP_ROW (app_row));

    return app_row;
}
