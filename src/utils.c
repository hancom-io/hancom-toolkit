/**
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

#include <glib.h>

#include "define.h"
#include "hancom-toolkit-config.h"

gboolean
check_package (const gchar* package)
{
    g_autofree gchar *out;
    g_autofree gchar *command;

    if (!package)
        return FALSE;

    command = g_strdup_printf ("%s/%s/%s %s",LIBDIR, GETTEXT_PACKAGE, HTOOLKIT_CHECK, package);
    g_spawn_command_line_sync (command, &out, NULL, NULL, NULL);

    if (g_str_has_prefix (out, "Version"))
        return TRUE;

    return FALSE;
}

gboolean
check_version (const gchar* package, const gchar* filename)
{
    g_autofree gchar *out;
    g_autofree gchar *command;

    gchar **version = NULL;
    gchar **file_version = NULL;

    gboolean res = FALSE;

    if (!filename)
        goto out;

    command = g_strdup_printf ("%s/%s/%s %s",LIBDIR, GETTEXT_PACKAGE, HTOOLKIT_CHECK, package);
    g_spawn_command_line_sync (command, &out, NULL, NULL, NULL);

    if (g_str_has_prefix (out, "Version"))
    {
        version = g_strsplit (out, " ", -1);
        if (!version[1])
            goto out;

        gchar *ver;
        ver = g_strchomp (version[1]);

        file_version = g_strsplit (filename, "_", -1);
        if (file_version && file_version[1])
        {
            if (g_strcmp0 (file_version[1], ver) != 0)
            {
                res = TRUE;
            }
        }
    }

out:
    if (version)
    {
        g_strfreev (version);
    }

    if (file_version)
    {
        g_strfreev (file_version);
    }

    return res;
}

