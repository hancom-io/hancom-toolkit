/**
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

#include <glib.h>
#include <gio/gio.h>
#include <glib/gstdio.h>

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
check_version_from_filename (const gchar* package, const gchar* filename)
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

gboolean
check_version (const gchar* package, const gchar* ver_name)
{
    g_autofree gchar *out;
    g_autofree gchar *command;

    gchar **version = NULL;

    gboolean res = FALSE;

    if (!ver_name)
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

        if (g_strcmp0 (ver, ver_name) != 0)
        {
            res = TRUE;
        }
    }

out:
    if (version)
    {
        g_strfreev (version);
    }

    return res;
}

static void
empty_directory (GFile *file)
{
    g_autoptr(GFileEnumerator) enumerator = NULL;
    g_autoptr (GFile) child = NULL;

    enumerator = g_file_enumerate_children (file,
                                            G_FILE_ATTRIBUTE_STANDARD_NAME,
                                            G_FILE_QUERY_INFO_NONE,
                                            NULL,
                                            NULL);

    g_file_enumerator_iterate (enumerator, NULL, &child, NULL, NULL);
    while (child != NULL)
    {
        gboolean res;
        res = g_file_delete (child, NULL, NULL);
        if (!res)
        {
            empty_directory (child);
            g_file_delete (child, NULL, NULL);
        }
        g_file_enumerator_iterate (enumerator, NULL, &child, NULL, NULL);
    }
}

void
remove_directory (gchar *path)
{
    if (!g_file_test (path, G_FILE_TEST_EXISTS))
        return;

    gint ret;
    ret = g_rmdir (path);

    if (ret != 0)
    {
        GFile *file = g_file_new_for_path (path);
        empty_directory (file);
        g_rmdir (path);
        g_object_unref (file);
    }
}
