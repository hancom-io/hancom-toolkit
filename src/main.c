/* main.c
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
#include <gio/gio.h>
#include <glib/gstdio.h>

#include "define.h"
#include "hancom-toolkit-config.h"
#include "htoolkit-application.h"

#include <locale.h>

static gboolean
check_live_installer ()
{
    gchar *installer_path;
    gboolean res;

    installer_path = g_find_program_in_path ("live-installer");
    res = (installer_path != NULL);
    g_free (installer_path);

    return res;
}

static gboolean
hancom_toolkit_init ()
{
    gboolean use_toolkit;

    FILE *fp;
    gchar line[1024], *lineptr;

    gchar *filename;
    filename = g_strdup (HTOOLKIT_PATH1);

    use_toolkit = FALSE;
    if ((fp = g_fopen (filename, "r")) != NULL)
    {
        while (fgets (line, sizeof (line), fp) != NULL)
        {
            lineptr = line + 11;
            if (g_str_has_prefix(lineptr, HTOOLKIT1))
            {
                use_toolkit = TRUE;
            }
            break;
        }
        fclose (fp);
    }

    g_free (filename);

    if (!use_toolkit)
        return FALSE;

    use_toolkit = FALSE;
    filename = g_strdup (HTOOLKIT_PATH2);
    if (!g_file_test (filename, G_FILE_TEST_EXISTS))
    {
        g_free (filename);
        return FALSE;
    }
    else
    {
        if ((fp = g_fopen (filename, "r")) != NULL)
        {
            while (fgets (line, sizeof (line), fp) != NULL)
            {
                if (g_str_has_prefix(line, "CODENAME"))
                {
                    lineptr = line + 9;
                    if (g_ascii_strncasecmp (lineptr, HTOOLKIT1, 7) == 0 ||
                        g_ascii_strncasecmp (lineptr, HTOOLKIT2, 6) == 0)
                    {
                        use_toolkit = TRUE;
                    }
                    break;
                }
            }
            fclose (fp);
        }
    }

    g_free (filename);
    return use_toolkit;
}
int
main (int   argc,
      char *argv[])
{
    if (check_live_installer ())
    {
        return 0;
    }

    if (!hancom_toolkit_init())
    {
        return 0;
    }

    g_autoptr(GtkApplication) app = NULL;

    setlocale (LC_ALL, "");

    /* Set up gettext translations */
    bindtextdomain (GETTEXT_PACKAGE, LOCALEDIR);
    bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
    textdomain (GETTEXT_PACKAGE);
    app = GTK_APPLICATION(htoolkit_application_new());

    return g_application_run (G_APPLICATION (app), argc, argv);
}
