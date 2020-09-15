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


#include <glib/gi18n.h>

#include "define.h"
#include "hancom-toolkit-config.h"

#include "htoolkit-app.h"
#include "htoolkit-app-row.h"
#include "htoolkit-app-list.h"
#include "htoolkit-controller.h"

typedef struct
{
    gboolean quit;
    HToolkitController *ctrl;
} HToolkitAppListPrivate;

enum {
    PROP_QUIT = 1,
    PROP_LAST
};

G_DEFINE_TYPE_WITH_PRIVATE (HToolkitAppList, htoolkit_app_list, GTK_TYPE_LIST_BOX)

static void
htoolkit_controller_network_changed (GNetworkMonitor *monitor,
                                     gboolean network_available,
                                     gpointer user_data)
{
    GList *list, *p;
    HToolkitAppList *app_list = HTOOLKIT_APP_LIST (user_data);

    if (network_available)
        return;

     list = gtk_container_get_children (GTK_CONTAINER (app_list));

    for (p = list; p != NULL; p = list->next)
    {
        gint state;
        HToolkitApp *app = HTOOLKIT_APP (p->data);

        state = htoolkit_app_get_state (app);
        if (STATE_INSTALLING <= state)
            continue;

        gchar *error = g_strdup (_("Network is not active"));
        htoolkit_app_set_error_msg (app, error);
        g_object_set (G_OBJECT (app), "status", STATE_ERROR, NULL);
        g_free (error);
    }
}

static void
htoolkit_app_list_row_state_notify_cb (HToolkitAppRow *app_row, GParamSpec *pspec, gpointer user_data)
{
    gint length;
    guint state;
    GList *list;

    g_object_get (G_OBJECT (app_row), "state", &state, NULL);

    HToolkitAppList *app_list = HTOOLKIT_APP_LIST (user_data);

    switch (state)
    {
    case STATE_INSTALLED:
    case STATE_CLOSE:
        {
            gtk_widget_hide (GTK_WIDGET(app_row));
            gtk_container_remove (GTK_CONTAINER (app_list), GTK_WIDGET(app_row));
            break;
        }
      default:
        break;
    }

    list = gtk_container_get_children (GTK_CONTAINER (app_list));
    length = g_list_length (list);

    if (length == 0)
    {
        g_signal_emit_by_name (app_list, "shutdown");
    }
}

void
htoolkit_app_list_add_app (HToolkitAppList *app_list, HToolkitApp *app)
{
    GtkWidget *app_row;
    HToolkitAppListPrivate *priv;
    priv  = htoolkit_app_list_get_instance_private (app_list);

    app_row = htoolkit_app_row_new (app, priv->ctrl);

    gtk_container_add (GTK_CONTAINER (app_list), app_row);

    g_signal_connect_object (app_row, "notify::state",
                             G_CALLBACK (htoolkit_app_list_row_state_notify_cb),
                             app_list, 0);
    gtk_widget_show (app_row);
}

static void
htoolkit_app_list_set_property (GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
    HToolkitAppList *app_list = HTOOLKIT_APP_LIST (object);
    HToolkitAppListPrivate *priv = htoolkit_app_list_get_instance_private (app_list);

    if (prop_id == PROP_QUIT)
    {
        priv->quit = g_value_get_boolean (value);
    }
}

static void
htoolkit_app_list_get_property (GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
    HToolkitAppList *app_list = HTOOLKIT_APP_LIST (object);
    HToolkitAppListPrivate *priv = htoolkit_app_list_get_instance_private (app_list);

    if (prop_id == PROP_QUIT)
    {
        g_value_set_boolean (value, priv->quit);
    }
}
static void
htoolkit_app_list_dispose (GObject *object)
{
    HToolkitAppList *app_list = HTOOLKIT_APP_LIST (object);
    HToolkitAppListPrivate *priv = htoolkit_app_list_get_instance_private (app_list);
    if (priv->ctrl)
    {
        g_clear_object (&priv->ctrl);
    }
    G_OBJECT_CLASS (htoolkit_app_list_parent_class)->dispose (object);
}

static void
htoolkit_app_list_init (HToolkitAppList *list)
{
    HToolkitAppListPrivate *priv = htoolkit_app_list_get_instance_private (list);
    priv->quit = FALSE;
    priv->ctrl  = htoolkit_controller_new ();

    GNetworkMonitor *monitor = g_network_monitor_get_default();
    g_signal_connect (monitor, "network-changed", G_CALLBACK (htoolkit_controller_network_changed), list);
}

static void
htoolkit_app_list_class_init (HToolkitAppListClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);
    object_class->dispose = htoolkit_app_list_dispose;
    object_class->set_property = htoolkit_app_list_set_property;
    object_class->get_property = htoolkit_app_list_get_property;

    g_signal_new( "shutdown",
                 G_TYPE_FROM_CLASS(klass),
                 G_SIGNAL_RUN_FIRST,
                 0,
                 NULL,
                 NULL,
                 g_cclosure_marshal_VOID__VOID,
                 G_TYPE_NONE,
                 0 );
}

GtkWidget *
htoolkit_app_list_new (void)
{
    HToolkitAppList *update_list;
    update_list = g_object_new (HTOOLKIT_TYPE_APP_LIST, NULL);
    return GTK_WIDGET (update_list);
}
