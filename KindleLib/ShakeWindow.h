#include <map>
#include <string>
#include <cstring>
#include "Drawing.h"

#ifndef _SHAKE_WINDOW_
#define _SHAKE_WINDOW_

using namespace std;

typedef void (*ButtonClickEvent)(GtkWidget *widget, gpointer data);

class ShakeWindow
{
public:
    ShakeWindow()
    {
        window = NULL;
        builder = NULL;
    }
    ~ShakeWindow()
    {
        Close();
    }

public:
    static void Initialize()
    {
        gtk_init(0, NULL);
    }
    static void DoEvents()
    {
        while (gtk_events_pending())
            gtk_main_iteration();
    }

    bool Load(string fileName, bool isMain)
    {
        /* Create new GtkBuilder object */
        builder = gtk_builder_new();
        GError     *error = NULL;
        if(!gtk_builder_add_from_file(builder, fileName.c_str(), &error))
        {
            g_warning("err = %s", error->message);
            g_free(error );
            return 0;
        }

        GdkScreen *screen = gdk_screen_get_default();
        gint screen_height = gdk_screen_get_height(screen);
        gint screen_width = gdk_screen_get_width(screen);

        InitializeWidgetsList();
        gtk_builder_connect_signals(builder, NULL);
        gtk_window_set_title(GTK_WINDOW(window), DefaultShakeWindowTitle.c_str());
        gtk_window_set_default_size(GTK_WINDOW(window), screen_width, screen_height);

        if (isMain)
            g_signal_connect_swapped(G_OBJECT(window), "destroy", G_CALLBACK(gtk_main_quit), NULL);

        return 1;
    }

    bool Create(string title = "")
    {
        if (title.length() == 0)
            title = DefaultShakeWindowTitle;

        window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
        gtk_container_set_border_width(GTK_CONTAINER(window), 8);
        gtk_window_set_title (GTK_WINDOW(window), title.c_str());
        gtk_window_set_position (GTK_WINDOW(window), GTK_WIN_POS_CENTER);
        gtk_widget_realize(window);

        return true;
    }

    GtkWidget* Handle()
    {
        return window;
    }

    gint MessageBox(string text, GtkButtonsType buttons = GTK_BUTTONS_CLOSE)
    {
        gint result;
        GtkWidget *dialog = NULL;

        dialog = gtk_message_dialog_new(GTK_WINDOW(window), GTK_DIALOG_MODAL, GTK_MESSAGE_INFO, buttons, text.c_str(), "");
        gtk_window_set_title(GTK_WINDOW(dialog), "L:A_N:msgbox:show");
        gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER);
        result = gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        return result;
    }

    static void SetDefaultTitle(string title)
    {
        DefaultShakeWindowTitle = title;
    }

    void Show()
    {
        gtk_widget_show_all(window);
    }
    gint ShowDialog()
    {
        gtk_widget_show_all(window);
        return gtk_dialog_run(GTK_DIALOG(window));
    }

    void SetModal(ShakeWindow* parent)
    {
        gtk_window_set_modal(GTK_WINDOW(window), TRUE);
        gtk_window_set_transient_for(GTK_WINDOW(window), GTK_WINDOW(parent->Handle()));
    }

    GtkWidget* GetWidget(string name)
    {
        return controls[name];//(GtkWidget*)gtk_builder_get_object(builder, name);
    }

    GtkWidget* operator[](const char* name)
    {
        return GetWidget(name);
    }

    void OnClick(GtkWidget* button, ButtonClickEvent method, void* userData = NULL)
    {
        g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(method), userData);
    }
    void OnClick(const char* button, ButtonClickEvent method, void* userData = NULL)
    {
        OnClick(GetWidget(button), method, userData);
    }

    void ApplyImage(GtkWidget* button, string imagePath, bool imgOnly = true)
    {
        GdkPixbuf* src = gdk_pixbuf_new_from_file(imagePath.c_str(), NULL);
        GdkPixbuf* dst = MakeDisabled(src);
        btnImages[button] = new ButtonImage(src, dst);
        gtk_button_set_image((GtkButton*)button, btnImages[button]->GetImage(GTK_WIDGET_SENSITIVE(button)));
        if (imgOnly)
            gtk_button_set_label((GtkButton*)button, "");
    }
    void ApplyImage(string button, string imagePath, bool imgOnly = true)
    {
        ApplyImage(GetWidget(button), imagePath, imgOnly);
    }

    void SetCloseButton(const char* name, bool freeOnClose = true)
    {
        if (freeOnClose)
            g_signal_connect(window, "destroy", G_CALLBACK(DestroyShakeWindow), this);
        OnClick(name, (ButtonClickEvent)CloseShakeWindow, this);
    }

    void Close()
    {
        for (map<GtkWidget*, ButtonImage*>::iterator it = btnImages.begin(); it != btnImages.end(); it++)
            delete it->second;
        btnImages.clear();

        GtkWidget* window = this->window;
        GtkBuilder *builder = this->builder;

        this->builder = NULL;
        this->window = NULL;

        // You can not do anything here!!! It's true!!!
        if (builder != NULL)
            g_object_unref(G_OBJECT(builder));
        if (window != NULL)
            gtk_widget_destroy(GTK_WIDGET(window));
    }

    void SetText(const char* widget, string text, string font = "")
    {
        SetText(GetWidget(widget), text, font);
    }
    void SetText(GtkWidget *widget, string text, string font = "")
    {
        if (GTK_IS_LABEL(widget))
            gtk_label_set_text(GTK_LABEL(widget), text.c_str());

        if (font.length() != 0)
            SetFont(widget, font);
    }

    void SetFont(const char* widget, string font)
    {
        SetFont(GetWidget(widget), font);
    }
    void SetFont(GtkWidget *widget, string font)
    {
        gtk_widget_modify_font(widget, pango_font_description_from_string(font.c_str()));
    }

    void Enable(const char* widget, bool enabled)
    {
        Enable(GetWidget(widget), enabled);
    }
    void Enable(GtkWidget* widget, bool enabled)
    {
        map<GtkWidget*, ButtonImage*>::iterator it = btnImages.find(widget);
        if (it != btnImages.end())
            gtk_button_set_image((GtkButton*)widget, it->second->GetImage(enabled));
        gtk_widget_set_sensitive(widget, enabled);
    }

    vector<GtkWidget*> FindWidgetsByType(string type)
    {
        vector<GtkWidget*> result;
        GSList *list = gtk_builder_get_objects(builder);
        for(; list; list=list->next)
        {
            GtkWidget* widget = (GtkWidget*)list->data;
            const gchar *name = gtk_widget_get_name((GtkWidget*)widget);
            if (string(name) == type)
                result.push_back(widget);
        }
        if (list) g_slist_free(list);
        return result;
    }

    void ResizeWidget(GtkWidget* widget, int w, int h)
    {
        gint cw, ch;
        gtk_widget_get_size_request(widget, &cw, &ch);
        gtk_widget_set_size_request(widget, w, h);
        map<GtkWidget*, ButtonImage*>::iterator it = btnImages.find(widget);
        if (it != btnImages.end())
        {
            it->second->Resize(it->second->Width * w / cw, it->second->Height * h / ch);
            gtk_button_set_image((GtkButton*)widget, it->second->GetImage(GTK_WIDGET_SENSITIVE(widget)));
        }
    }

private:
    void InitializeWidgetsList()
    {
        GSList *list = gtk_builder_get_objects(builder);
        for(; list; list=list->next)
        {
            GtkWidget* widget = (GtkWidget*)list->data;
            const gchar *name = gtk_buildable_get_name((GtkBuildable*)widget);
            controls[name] = widget;
            if (GTK_IS_WINDOW(widget))
                window = widget;
        }
        if (list) g_slist_free(list);
    }
    static void CloseShakeWindow(GtkWidget *widget, gpointer data)
    {
        ((ShakeWindow*)data)->Close();
    }
    static void DestroyShakeWindow(GtkWidget *widget, gpointer data)
    {
        delete ((ShakeWindow*)data);
    }

protected:
    static string DefaultShakeWindowTitle;
    GtkWidget* window;
    GtkBuilder *builder;
    map<string, GtkWidget*> controls;
    map<GtkWidget*, ButtonImage*> btnImages;
};

class InputDialogBox : public ShakeWindow
{
public:
    InputDialogBox(ShakeWindow* parent, string question, string title = "", string value = "")
    {
        if (title.length() == 0)
            title = "L:A_N:Dialog:Input";

        window = gtk_dialog_new_with_buttons(title.c_str(), GTK_WINDOW(parent->Handle()), GTK_DIALOG_MODAL, NULL);
        btnCancel = gtk_dialog_add_button((GtkDialog*)window, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL);
        btnOK = gtk_dialog_add_button((GtkDialog*)window, GTK_STOCK_OK, GTK_RESPONSE_OK);
        OnClick(btnOK, OnClose, this);
        OnClick(btnCancel, OnClose, this);

        gtk_dialog_set_default_response(GTK_DIALOG(window), GTK_RESPONSE_REJECT);
        GtkBox* area = GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(window)));
        GtkWidget* label = gtk_label_new((question + "\n").c_str());
        gtk_box_pack_start(area, label, TRUE, TRUE, 1);
        input = gtk_entry_new();
        gtk_entry_set_text(GTK_ENTRY(input), value.c_str());
        gtk_box_pack_start(area, input, FALSE, FALSE, 1);
        SetModal(parent);
    }
    string GetValue()
    {
        return value;
    }

private:
    static void OnClose(GtkWidget *widget, gpointer data)
    {
        ((InputDialogBox*)data)->value = gtk_entry_get_text(GTK_ENTRY(((InputDialogBox*)data)->input));
        ((InputDialogBox*)data)->Close();
    }

private:
    GtkWidget* input;
    GtkWidget* btnOK;
    GtkWidget* btnCancel;
    string value;
};

class WaitWindow : public ShakeWindow
{
public:
    WaitWindow(ShakeWindow* parent, string msg = "Please, wait...", string title = "")
    {
        if (title.length() == 0)
            title = "L:A_N:Dialog:Wait";

        Create(title);
        SetModal(parent);
        gtk_window_set_position (GTK_WINDOW(window), GTK_WIN_POS_CENTER);
        gtk_window_set_default_size(GTK_WINDOW(window), 400, 200);
        GtkWidget* label = gtk_label_new(msg.c_str());
        gtk_container_add(GTK_CONTAINER(Handle()), label);
        gtk_window_set_type_hint(GTK_WINDOW(Handle()), GDK_WINDOW_TYPE_HINT_DIALOG);
    }
};

string ShakeWindow::DefaultShakeWindowTitle;

#endif // _SHAKE_WINDOW_
