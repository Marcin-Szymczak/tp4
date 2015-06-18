#include <gtk/gtk.h>

#include <iostream>

#include "project.h"

static cairo_surface_t* surface = NULL;

static void clear_surface()
{
    cairo_t *cr;
    cr= cairo_create( surface );
    cairo_set_source_rgb( cr, 1, 1, 1 );
    cairo_paint( cr );
    cairo_destroy( cr );
}

gboolean button_test( GtkWidget* widget, GdkEvent* event, gpointer user_data )
{
    static int i=0;
    std::cout<<"Hello! "<<i++<<std::endl;
    return false;
}

int main( int argc, char *argv[])
{
    GtkWidget* window;
    GtkBuilder* builder;

    gtk_init(&argc, &argv);

    builder = gtk_builder_new_from_file("tp4.glade");

    window = GTK_WIDGET( gtk_builder_get_object( builder, "mainwindow" ) );

    gtk_builder_connect_signals( builder, NULL );

    project::init( builder );

    g_object_unref( G_OBJECT( builder ) );

    gtk_widget_show(window);

    gtk_main();

    return 0;
}
