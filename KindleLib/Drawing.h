#include <gdk/gdk.h>
#include <glib.h>
#include <gtk/gtk.h>

#ifndef DRAWING_H
#define DRAWING_H

#define RED_PIX          0
#define GREEN_PIX        1
#define BLUE_PIX         2
#define ALPHA_PIX        3

using namespace std;

class ButtonImage
{
public:
    ButtonImage() { Enabled = NULL; Disabled = NULL; }
    ~ButtonImage() { Free(); }

public:
    ButtonImage(GdkPixbuf *e, GdkPixbuf *d)
    {
        Enabled = NULL; Disabled = NULL;
        Initialize(e, d);
    }
    GtkWidget *GetImage(bool enabled)
    {
        if (enabled)
            return Enabled;
        else
            return Disabled;
    }
    void Resize(int w, int h)
    {
        GdkPixbuf* pixe = gtk_image_get_pixbuf(GTK_IMAGE(Enabled));
        GdkPixbuf* pixd = gtk_image_get_pixbuf(GTK_IMAGE(Disabled));
        pixe = gdk_pixbuf_scale_simple(pixe, w, h, GDK_INTERP_BILINEAR);
        pixd = gdk_pixbuf_scale_simple(pixd, w, h, GDK_INTERP_BILINEAR);
        Initialize(pixe, pixd);
    }

private:
    void Free()
    {
        if (Enabled)
            g_object_unref(Enabled);
        if (Disabled)
            g_object_unref(Disabled);
        Enabled = NULL;
        Disabled = NULL;
    }
    void Initialize(GdkPixbuf *e, GdkPixbuf *d)
    {
        Free();
        Enabled = gtk_image_new_from_pixbuf(e);
        Disabled = gtk_image_new_from_pixbuf(d);
        Width = gdk_pixbuf_get_width(e);
        Height = gdk_pixbuf_get_height(e);
        g_object_ref(Enabled);
        g_object_ref(Disabled);
    }

public:
    int Width;
    int Height;

private:
    GtkWidget *Enabled;
    GtkWidget *Disabled;
};

GdkPixbuf* MakeDisabled(GdkPixbuf* src)
{
    int width = gdk_pixbuf_get_width(src);
	int height = gdk_pixbuf_get_height(src);
    int rowstride = gdk_pixbuf_get_rowstride(src);
    guchar* orig_pixels = gdk_pixbuf_get_pixels(src);
    guchar* pixels = new guchar[rowstride * height];

    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width * 4; j += 4)
        {
            int id = rowstride * i + j;

            pixels[id] = orig_pixels[id];
            pixels[id + 1] = orig_pixels[id + 1];
            pixels[id + 2] = orig_pixels[id + 2];
            pixels[id + 3] = orig_pixels[id + 3] / 4;
        }
    }

    GdkPixbuf* pixbuf = gdk_pixbuf_new_from_data(pixels, gdk_pixbuf_get_colorspace(src), true, 8, width, height, rowstride, NULL, NULL);

   return pixbuf;
}

#endif // END
