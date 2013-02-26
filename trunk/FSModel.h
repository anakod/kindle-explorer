#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <unistd.h>
#include <string>
#include <gtk/gtk.h>
#include "../KindleLib/ShakeWindow.h"
#include "FileData.h"

enum
{
    COL_ICON = 0,
    COL_NAME,
    COL_AGE,
    COL_IS_DIR,
    NUM_COLS
};

class FilesModel
{
public:
    void Assign(GtkWidget *view, GtkWidget *pth)
    {
        tree = view;
        path = pth;
        GError *error = NULL;
        file = gdk_pixbuf_new_from_file(GetAppFile("res/file.png").c_str(), &error);
        if (error)
        {
            g_warning ("Could not load icon: %s\n", error->message);
            g_error_free(error);
            error = NULL;
        }
        folder = gdk_pixbuf_new_from_file(GetAppFile("res/folder.png").c_str(), &error);
        if (error)
        {
            g_warning ("Could not load icon: %s\n", error->message);
            g_error_free(error);
            error = NULL;
        }
        CreateColumns();
    }

    void CreateColumns()
    {
        GtkCellRenderer *renderer;
        /* --- Column --- */
        GtkTreeViewColumn *col = gtk_tree_view_column_new();
        gtk_tree_view_column_set_title(col, "Title");

        renderer = gtk_cell_renderer_pixbuf_new();
        gtk_tree_view_column_pack_start(col, renderer, FALSE);
        gtk_tree_view_column_set_attributes(col, renderer, "pixbuf", COL_ICON, NULL);

        renderer = gtk_cell_renderer_text_new();
        gtk_tree_view_column_pack_start(col, renderer, TRUE);
        gtk_tree_view_column_set_attributes(col, renderer, "text", COL_NAME, NULL);

        gtk_tree_view_append_column(GTK_TREE_VIEW(tree), col);

        /* --- Column --- */
        //renderer = gtk_cell_renderer_text_new();
        //gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(tree), -1, "Name", renderer, "text", COL_NAME, NULL);
        /* --- Column --- */
        //renderer = gtk_cell_renderer_text_new();
        //gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(tree), -1, "Size", renderer, "text", COL_AGE, NULL);
        UpdateList();
    }

    GtkTreeModel *GetModel()
    {
        return model;
    }

    void UpdateList()
    {
        model = CreateFilesModel();
        gtk_tree_view_set_model(GTK_TREE_VIEW(tree), model);
        g_object_unref(model);

        char cur[2048];
        GetCurrentDir(cur, sizeof(cur));
        gtk_entry_set_text(GTK_ENTRY(path), cur);
        gtk_entry_select_region(GTK_ENTRY(path), 0, 0);
        gtk_entry_set_position(GTK_ENTRY(path), strlen(cur) - 1);
    }

    bool IsItemSelected()
    {
        GtkTreeIter iter;
        GtkTreeModel *mdl;

        return gtk_tree_selection_get_selected(GTK_TREE_SELECTION(gtk_tree_view_get_selection(GTK_TREE_VIEW(tree))), &mdl, &iter);
    }

    FileItem GetSelectedItem()
    {
        GtkTreeIter iter;
        char *name;
        gboolean dir;
        FileItem result;
        GtkTreeModel *mdl;

        gtk_tree_selection_get_selected(GTK_TREE_SELECTION(gtk_tree_view_get_selection(GTK_TREE_VIEW(tree))), &mdl, &iter);
        {
            gtk_tree_model_get(mdl, &iter, COL_NAME, &name, COL_IS_DIR, &dir, -1);
            result.name = name;
            result.dir = dir;
            g_free(name);
        }
        return result;
    }

private:
    GtkTreeModel *CreateFilesModel()
    {
        GtkListStore *store;
        GtkTreeIter iter;
        store = gtk_list_store_new(NUM_COLS, GDK_TYPE_PIXBUF, G_TYPE_STRING, G_TYPE_UINT, G_TYPE_BOOLEAN);
        gtk_tree_sortable_set_default_sort_func (GTK_TREE_SORTABLE(store), NULL, NULL, NULL);
        gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE(store), GTK_TREE_SORTABLE_DEFAULT_SORT_COLUMN_ID, GTK_SORT_ASCENDING);
        vector<FileItem> files = GetFiles();
        for(vector<FileItem>::iterator it = files.begin(); it != files.end(); ++it)
        {
            GdkPixbuf* icon = it->dir ? folder : file;
            gtk_list_store_append(store, &iter);
            gtk_list_store_set(store, &iter, COL_ICON, icon, COL_NAME, it->name.c_str(), COL_AGE, 51, COL_IS_DIR, it->dir, -1);
        }

        return GTK_TREE_MODEL (store);
    }

private:
    GtkWidget *tree;
    GtkWidget *path;
    GtkTreeModel *model;
    GdkPixbuf* file;
    GdkPixbuf* folder;
};
