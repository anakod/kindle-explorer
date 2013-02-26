#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <unistd.h>
#include <string>
#include <gtk/gtk.h>
#include "../KindleLib/ShakeWindow.h"
#include "FSModel.h"
#include "timer.h"

#define EDITOR "/mnt/us/extensions/leafpad/bin/leafpad"
#define TERMINAL "/mnt/us/extensions/kterm/bin/kterm"

using namespace std;

ShakeWindow *win;
GtkWidget *lstFiles, *txtPath;
FilesModel *fs;
Timer timer; string lastSel;
string copied;
const char* imageFormats = "bmp;png;gif;ico;jpg;jpeg;wmf;tga";

void UpdateButtons()
{
    bool sel = fs->IsItemSelected();

    win->Enable("btnView", sel);
    win->Enable("btnEdit", sel);
    win->Enable("btnExecute", sel);

    win->Enable("btnCopy", sel);
    win->Enable("btnRename", sel);
    win->Enable("btnDelete", sel);

    win->Enable("btnPaste", copied.length() > 0);
}

void DirUp(GtkWidget *widget, gpointer data)
{
    chdir("..");
    fs->UpdateList();
}

void ExecuteFile(GtkWidget *widget, gpointer data)
{
    if (!fs->IsItemSelected())
        return;
    FileItem sel = fs->GetSelectedItem();
    if (sel.dir)
        return;

    Execute(sel.name);
}

void EditFile(GtkWidget *widget, gpointer data)
{
    if (!fs->IsItemSelected())
        return;
    FileItem sel = fs->GetSelectedItem();
    if (sel.dir)
        return;

    if (!FileExist(EDITOR))
    {
        win->MessageBox(string("Can't find editor: ") + EDITOR);
        return;
    }

    Execute(string(EDITOR) + " " + GetFullLocalFilePath(sel.name));
}

void RunTerminal(GtkWidget *widget, gpointer data)
{
    if (!FileExist(TERMINAL))
    {
        win->MessageBox(string("Can't find terminal: ") + TERMINAL);
        return;
    }

    Execute(string(TERMINAL), GetFullLocalFilePath(""));
}

void RenameFile(GtkWidget *widget, gpointer data)
{
    if (!fs->IsItemSelected())
        return;
    FileItem sel = fs->GetSelectedItem();

    OpenKeyboard();
    InputDialogBox* input = new InputDialogBox(win, "Enter new name:", "", sel.name);
    if (input->ShowDialog() == GTK_RESPONSE_OK)
    {
        string to = GetFullLocalFilePath(input->GetValue());
        rename(GetFullLocalFilePath(sel.name).c_str(), to.c_str());
        fs->UpdateList();
    }
    CloseKeyboard();
    delete input;
}

void DeleteFile(GtkWidget *widget, gpointer data)
{
    if (!fs->IsItemSelected())
        return;
    FileItem sel = fs->GetSelectedItem();

    string msg = string("Remove \"") + sel.name + "\"?";
    if (sel.dir)
        msg = "Do you really want to remove \"" + sel.name + "\" and all its subdirectories?";

    gint res = win->MessageBox(msg, GTK_BUTTONS_YES_NO);
    if (res == GTK_RESPONSE_YES)
    {
        WaitWindow* wait = new WaitWindow(win);
        wait->Show();
        RemovePath(GetFullLocalFilePath(sel.name));
        fs->UpdateList();
        delete wait;
    }
}

void CopyFile(GtkWidget *widget, gpointer data)
{
    if (!fs->IsItemSelected())
        return;
    FileItem sel = fs->GetSelectedItem();
    copied = GetFullLocalFilePath(sel.name);
    //win->MessageBox("Clipboard item: " + copied);
    fs->UpdateList();
}

void PasteFile(GtkWidget *widget, gpointer data)
{
    if (copied.length() == 0)
        return;

    string to = GetFullLocalFilePath(GetFileName(copied));
    if (to == copied)
        to += "_copy";
    else if (to.find(copied) != string::npos)
    {
        win->MessageBox("You can't copy " + copied + " to " + to + ". This is recursion! :(");
        return;
    }

    WaitWindow* wait = new WaitWindow(win);
    wait->Show();

    CopyPath(copied, to);
    copied = "";
    fs->UpdateList();

    delete wait;
}

void ViewFile(GtkWidget *widget, gpointer data)
{
    if (!fs->IsItemSelected())
        return;
    FileItem sel = fs->GetSelectedItem();
    if (sel.dir)
    {
        chdir(sel.name.c_str());
        fs->UpdateList();
        return;
    }

    bool isImage = CheckExtension(imageFormats, sel.name);
    string layout = "";

    if (isImage)
        layout = "ViewImage.glade";
    else
        layout = "ViewText.glade";

    ShakeWindow *viewer = new ShakeWindow();
    viewer->Load(GetResFile(layout), false);
    viewer->SetCloseButton("btnClose", true);
    viewer->ApplyImage("btnClose", GetResFile("back.png"));
    viewer->SetModal(win);
    viewer->SetText("lblName", sel.name, "Tahoma 14");

    if (isImage)
    {
        GtkWidget *image = gtk_image_new_from_file(sel.name.c_str());
        gtk_container_add(GTK_CONTAINER(viewer->GetWidget("viewportMain")), image);
    }
    else
    {
        string txt = get_file_contents(sel.name.c_str());
        GtkTextBuffer *buffer = gtk_text_buffer_new(NULL);
        gtk_text_buffer_set_text(buffer, txt.c_str(), -1);
        gtk_text_view_set_buffer((GtkTextView*)viewer->GetWidget("txtData"), buffer);
        viewer->SetFont("txtData", "Tahoma 7");
    }

    viewer->Show();
}

void ShowAbout(GtkWidget *widget, gpointer data)
{
  GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file(GetResFile("program.png").c_str(), NULL);

  GtkWidget *dialog = gtk_about_dialog_new();
  gtk_about_dialog_set_name(GTK_ABOUT_DIALOG(dialog), "KindleExplorer");
  gtk_about_dialog_set_version(GTK_ABOUT_DIALOG(dialog), "1.0");
  gtk_about_dialog_set_copyright(GTK_ABOUT_DIALOG(dialog), "Developer: anakod");
  gtk_about_dialog_set_comments(GTK_ABOUT_DIALOG(dialog), "Fast and simple file explorer.\n\nhttp://66bit.ru");
  //gtk_about_dialog_set_website(GTK_ABOUT_DIALOG(dialog), "");
  gtk_about_dialog_set_logo(GTK_ABOUT_DIALOG(dialog), pixbuf);
  g_object_unref(pixbuf), pixbuf = NULL;
  gtk_window_set_title(GTK_WINDOW(dialog), "L:A_N:Explorer:About");
  gtk_window_set_position (GTK_WINDOW(dialog), GTK_WIN_POS_CENTER);
  gtk_dialog_run(GTK_DIALOG (dialog));
  gtk_widget_destroy(dialog);
}

void on_changed(GtkWidget *widget, gpointer label)
{
    UpdateButtons();
}

gboolean ListCkicked(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
    if (!fs->IsItemSelected())
        return FALSE;

    FileItem sel = fs->GetSelectedItem();

    if (!timer.IsStarted() || timer.GetDuration() > 0.7 || lastSel != sel.name)
    {
        timer.Start();
        lastSel = sel.name;
        return FALSE;
    }
    timer.Start();

    if (sel.dir)
    {
        chdir(sel.name.c_str());
        fs->UpdateList();
    }

    return TRUE;
}

void AdaptSize()
{
    gint screenWidth = gdk_screen_get_width(gdk_screen_get_default());
    gint size = screenWidth / 12;
    if (size < 62) // for small screens downsize buttons
    {
        vector<GtkWidget*> buttons = win->FindWidgetsByType("GtkButton");
        for(vector<GtkWidget*>::iterator it = buttons.begin(); it != buttons.end(); it++)
            win->ResizeWidget(*it, size, size);
    }
}

int main (int argc, char **argv)
{
    ShakeWindow::Initialize();
    ShakeWindow::SetDefaultTitle("L:A_N:application_FileExplorer:Window");
    win = new ShakeWindow();
    win->Load(GetAppFile("res/MainWindow.glade"), true);
    win->SetCloseButton("btnClose");
    lstFiles = win->GetWidget("lstFiles");
    txtPath = win->GetWidget("txtPath");
    fs = new FilesModel();
    fs->Assign(lstFiles, txtPath);

    win->OnClick("btnDirUp", DirUp);
    win->OnClick("btnView", ViewFile);
    win->OnClick("btnExecute", ExecuteFile);
    win->OnClick("btnEdit", EditFile);
    win->OnClick("btnTerminal", RunTerminal);

    win->OnClick("btnRename", RenameFile);
    win->OnClick("btnCopy", CopyFile);
    win->OnClick("btnPaste", PasteFile);
    win->OnClick("btnDelete", DeleteFile);
    win->OnClick("btnAbout", ShowAbout);

    win->ApplyImage("btnDirUp", GetResFile("up.png"));
    win->ApplyImage("btnView", GetResFile("view.png"));
    win->ApplyImage("btnExecute", GetResFile("execute.png"));
    win->ApplyImage("btnEdit", GetResFile("edit.png"));
    win->ApplyImage("btnTerminal", GetResFile("terminal.png"));
    win->ApplyImage("btnRename", GetResFile("rename.png"));
    win->ApplyImage("btnCopy", GetResFile("copy.png"));
    win->ApplyImage("btnPaste", GetResFile("paste.png"));
    win->ApplyImage("btnDelete", GetResFile("delete.png"));
    win->ApplyImage("btnAbout", GetResFile("about.png"));
    win->ApplyImage("btnClose", GetResFile("close.png"));

    AdaptSize();

    g_signal_connect(gtk_tree_view_get_selection(GTK_TREE_VIEW(lstFiles)), "changed", G_CALLBACK(on_changed), NULL);
    g_signal_connect(G_OBJECT(lstFiles), "button_release_event", (GtkSignalFunc)ListCkicked, NULL);

    UpdateButtons();
    win->Show();

    gtk_main();

    delete fs;
    return 0;
}
