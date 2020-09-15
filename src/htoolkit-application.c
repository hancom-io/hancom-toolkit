/* htoolkit-application.c
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

#include <gtk/gtk.h>
#include <gdk/gdkx.h>

#include "hancom-toolkit-config.h"
#include "htoolkit-window.h"
#include "htoolkit-application.h"

#define DEFAULT_WINDOW_WIDTH 340
#define DEFAULT_WINDOW_HEIGHT 390

struct _HToolkitApplicationPrivate
{
    GtkWindow *window;
    GtkCssProvider *provider;
};

G_DEFINE_TYPE_WITH_PRIVATE (HToolkitApplication, htoolkit_application, GTK_TYPE_APPLICATION)

static void
htoolkit_application_shutdown_cb (GtkWindow *win, gpointer data)
{
    g_print ("%s\n", __func__);
    g_application_quit (G_APPLICATION (data));
}

static void
htoolkit_application_activate (GApplication *app)
{
    GFile *file;
    HToolkitApplicationPrivate *priv;
    priv = htoolkit_application_get_instance_private (HTOOLKIT_APPLICATION(app));

    /* Get the current window or create one if necessary. */
    priv->window = gtk_application_get_active_window (GTK_APPLICATION(app));
    if (priv->window == NULL)
        priv->window = g_object_new (HTOOLKIT_TYPE_WINDOW,
                               "application", app,
                               "default-width", DEFAULT_WINDOW_WIDTH,
                               "default-height", DEFAULT_WINDOW_HEIGHT,
                               NULL);

    gtk_window_set_position (GTK_WINDOW (priv->window), GTK_WIN_POS_NONE);
    gtk_window_set_decorated (GTK_WINDOW (priv->window), FALSE);
    gtk_window_set_skip_taskbar_hint (GTK_WINDOW (priv->window), TRUE);
    gtk_window_set_skip_pager_hint (GTK_WINDOW (priv->window), TRUE);
    gtk_window_set_resizable (GTK_WINDOW (priv->window), FALSE);
    gtk_widget_set_app_paintable (GTK_WIDGET (priv->window), TRUE);
    GdkScreen *screen = gtk_window_get_screen (GTK_WINDOW (priv->window));

    if(gdk_screen_is_composited(screen))
    {
        GdkVisual *visual = gdk_screen_get_rgba_visual (screen);

        if (visual == NULL)
            visual = gdk_screen_get_system_visual (screen);

        gtk_widget_set_visual (GTK_WIDGET(priv->window), visual);
    }

    int width = gdk_screen_width() - DEFAULT_WINDOW_WIDTH - 10;
    gtk_window_move (GTK_WINDOW (priv->window), width, 0);
    /* Ask the window manager/compositor to present the window. */
    gtk_window_present (GTK_WINDOW(priv->window));

    priv->provider = gtk_css_provider_new ();
    file = g_file_new_for_uri ("resource:///kr/hancom/hancom-toolkit/style.css");
    gtk_css_provider_load_from_file (priv->provider, file, NULL);
    gtk_style_context_add_provider_for_screen (gdk_screen_get_default(),
                                               GTK_STYLE_PROVIDER (priv->provider),
                                               GTK_STYLE_PROVIDER_PRIORITY_APPLICATION + 1);
    g_object_unref (file);
    g_signal_connect (priv->window,
                      "shutdown",
                      G_CALLBACK (htoolkit_application_shutdown_cb),
                      app);
}

static void
htoolkit_application_dispose (GObject *object)
{
    HToolkitApplication *app = HTOOLKIT_APPLICATION(object);
    HToolkitApplicationPrivate *priv = app->priv;

    if (priv && priv->window != NULL)
    {
        gtk_widget_destroy (GTK_WIDGET(priv->window));
        priv->window = NULL;
    }

    if (priv && priv->provider != NULL)
    {
        g_clear_object (&priv->provider);
        priv->provider = NULL;
    }
    G_OBJECT_CLASS (htoolkit_application_parent_class)->dispose (object);
}

static void
htoolkit_application_finalize (GObject *object)
{
    G_OBJECT_CLASS (htoolkit_application_parent_class)->finalize (object);
}

static void
htoolkit_application_init (HToolkitApplication *application)
{
    HToolkitApplicationPrivate *priv;
    priv = htoolkit_application_get_instance_private (application);
    priv->window = NULL;
}

static void
htoolkit_application_class_init (HToolkitApplicationClass *class)
{
    G_OBJECT_CLASS (class)->dispose = htoolkit_application_dispose;
    G_OBJECT_CLASS (class)->finalize = htoolkit_application_finalize;
    G_APPLICATION_CLASS (class)->activate = htoolkit_application_activate;
}

HToolkitApplication*
htoolkit_application_new (void)
{
  return g_object_new (HTOOLKIT_TYPE_APPLICATION,
                       "application-id", "kr.hancom.hancom-toolkit",
                       "flags", G_APPLICATION_HANDLES_OPEN,
                       NULL);
}
