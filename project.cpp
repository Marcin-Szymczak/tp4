#include "project.h"

#include <vector>
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <cmath>

#define PI 3.14159265358

namespace project
{

    const char* FILENAME = "outputRotateB01.log";
    const int TIMESTEP = 0.04; // 25 Hz - 0.04 s
    const int GROUPS = 4;
    const int GROUPSIZE = 3;
    const int GROUP = 0;

    const int SURFACES = 6;
    static cairo_surface_t* surface[SURFACES];

    int TIME = 0;
    float DRAW_SCALE = 1;

    GtkWidget* drawing[SURFACES];
    int SKIP_SAMPLES = 0;

    struct Data
    {
        std::vector< double > values;
        double minimum;
        double maximum;
        double constant;
        void calculate()
        {
            double sum;
            minimum = 99999;
            maximum = -99999;
            for( auto it = values.begin(); it != values.end(); it++ ){
                sum += *it;
                if( *it > maximum )
                    maximum = *it;
                if( *it < minimum )
                    minimum = *it;
            }
            sum /= values.size();
            constant = sum;
            std::cout<<"C: "<<constant<<" Min: "<< minimum << " Max: "<< maximum << "\n";
        }
    };


    Data data[3];

    struct SignalDrawingData
    {
        size_t surface_id;
    };

    void load( const char* filename )
    {

        for( int i=0; i<SURFACES; i++ ){
            data[i].values.clear();
        }

        std::ifstream f( filename );
        int nline=1;

        f.seekg( 0, f.end );
        size_t filesize = f.tellg();
        f.seekg( 0, f.beg );

        double temp;

        for( int i=0; i<GROUPS*GROUPSIZE*SKIP_SAMPLES; i++ ){
            f>>temp;
        }

        while( f && f.tellg() < filesize )
        {

            int i=0;
            for( ; i<GROUP*GROUPSIZE; i++ ){
                f>>temp;
            }

            for( ; i<(GROUP+1)*GROUPSIZE; i++ ){
                f>>temp;
                data[i-GROUP*GROUPSIZE].values.push_back( temp );
            }

            for( ; i<GROUPS*GROUPSIZE; i++ ){
                f>>temp;
            }
            f.ignore( 1, '\n' );
        }

        std::cout<<"File "<< filename << " loaded!" << "\n";
    }

    void refresh_all()
    {
        for( int i=0; i<SURFACES; i++ ){
            gtk_widget_queue_draw( drawing[i] );
        }
    }

    void clear_surface( int id )
    {
        cairo_t* cr;
        cr = cairo_create( surface[id] );
        cairo_set_source_rgb( cr, 1, 1, 1 );
        cairo_paint( cr );
        cairo_destroy( cr );
    }

    void plot( GtkWidget* widget, cairo_surface_t* surface, Data& data )
    {
        cairo_t* cr;
        cr = cairo_create( surface );
        int offset = 100;

        cairo_set_source_rgb( cr, .5, 0, 0 );
        cairo_move_to( cr, 0, offset );
        cairo_line_to( cr, 99999, offset );
        cairo_stroke( cr );

        cairo_set_source_rgb( cr, 0, 0, 0 );
        if( data.values.size() > 0 ){
            size_t i = 0;
            float SCALE = DRAW_SCALE*100/(data.maximum - data.minimum);
            cairo_move_to( cr, i, ( data.values[0] - data.constant )*SCALE+100 );
            i++;
            for( ; i<data.values.size(); i++ ){
                cairo_line_to( cr, i, ( data.values[i] - data.constant )*SCALE+100 );
            }
            cairo_set_line_width( cr, 1 );
            cairo_set_line_cap( cr, CAIRO_LINE_CAP_ROUND );
            cairo_stroke( cr );
        }

        cairo_set_source_rgb( cr, 0, 0, 1 );
        cairo_move_to( cr, TIME, 0 );
        cairo_line_to( cr, TIME, 1000 );
        cairo_stroke( cr );

        cairo_destroy( cr );

        gtk_widget_queue_draw( widget );
    }

    void draw_horizon( GtkWidget* widget, cairo_surface_t* surface, Data& samples )
    {
        cairo_t* cr;
        cr = cairo_create( surface );
        int offset = 100;
        int center = 100; //w definicji widgetu horyzontu
        cairo_set_source_rgb( cr, 0, 0, 0 );
        if( TIME >= 0 && TIME < samples.values.size() ){
            double rad = samples.values[TIME]/180*PI;
            double vx = cos( rad );
            double vy = sin( rad );

            cairo_move_to( cr, center - vx*200, offset - vy*200 );
            cairo_line_to( cr, center + vx*200, offset + vy*200 );
            cairo_stroke( cr );
        }
        cairo_destroy( cr );
        gtk_widget_queue_draw( widget );

    }

    void createSurfaceForWidget( GtkWidget* widget, int id ){
        if( surface[id] )
            cairo_surface_destroy( surface[id] );
        surface[id] = gdk_window_create_similar_surface( gtk_widget_get_window( widget ),
                                                    CAIRO_CONTENT_COLOR,
                                                    gtk_widget_get_allocated_width( widget ),
                                                    gtk_widget_get_allocated_height( widget ) );
        clear_surface( id );

    }

    gboolean drawing_configure( GtkWidget* widget, GdkEventConfigure* event, SignalDrawingData* data )
    {
        createSurfaceForWidget( widget, data->surface_id );
        return TRUE;
    }

    gboolean drawing_draw( GtkWidget* widget, cairo_t *cr, SignalDrawingData* ptr )
    {
        clear_surface( ptr->surface_id );
        plot( widget, surface[ptr->surface_id], data[ ptr->surface_id ] );
        cairo_set_source_surface( cr, surface[ptr->surface_id], 0, 0 );
        cairo_paint( cr );

        return FALSE;
    }

    gboolean drawing_draw_horizon( GtkWidget* widget, cairo_t *cr, SignalDrawingData* ptr )
    {
        clear_surface( ptr->surface_id );
        draw_horizon( widget, surface[ptr->surface_id], data[ ptr->surface_id - 3 ] );

        cairo_set_source_surface( cr, surface[ptr->surface_id], 0, 0 );
        cairo_paint( cr );

        return FALSE;
    }

    void button_load( GtkWidget* widget, GdkEventButton* event, gpointer user_data )
    {
        if( event->type != GDK_BUTTON_PRESS ) return;

        load( FILENAME );
        for( int i=0; i<3; i++ )
            data[i].calculate();
    }

    gboolean button_press_zoom_in( GtkWidget* widget, GdkEventButton* event, gpointer user_data )
    {
        DRAW_SCALE += 0.5;
    }

    gboolean button_press_zoom_out( GtkWidget* widget, GdkEventButton* event, gpointer user_data )
    {
        DRAW_SCALE -= 0.5;
    }

    gboolean button_press_sample_next( GtkWidget* widget, GdkEventButton* event, gpointer user_data )
    {
        TIME++;
    }

    gboolean button_press_sample_prev( GtkWidget* widget, GdkEventButton* event, gpointer user_data )
    {
        if( TIME > 0 )
            TIME--;
    }

    gboolean button_press_sample_next_fast( GtkWidget* widget, GdkEventButton* event, gpointer user_data )
    {
        TIME+=10;
    }

    gboolean button_press_sample_prev_fast( GtkWidget* widget, GdkEventButton* event, gpointer user_data )
    {
        if( TIME >= 10 )
            TIME-=10;
    }

    gboolean spinbutton_skipsamples_value_changed( GtkSpinButton* widget, gpointer user_data )
    {
        int amount = gtk_spin_button_get_value( widget );
        SKIP_SAMPLES = amount;
    }

    void connect_signals( GtkBuilder* builder )
    {
        g_signal_connect( GTK_WIDGET( gtk_builder_get_object( builder, "button1" ) ), "button-press-event", G_CALLBACK( button_load ), NULL );

        SignalDrawingData* drawingdata = new SignalDrawingData;
        drawingdata->surface_id = 0;
        g_signal_connect( drawing[0], "configure-event", G_CALLBACK( drawing_configure ), drawingdata );
        g_signal_connect( drawing[0], "draw", G_CALLBACK( drawing_draw ), drawingdata );

        drawingdata = new SignalDrawingData;
        drawingdata->surface_id = 1;
        g_signal_connect( drawing[1], "configure-event", G_CALLBACK( drawing_configure ), drawingdata );
        g_signal_connect( drawing[1], "draw", G_CALLBACK( drawing_draw ), drawingdata );

        drawingdata = new SignalDrawingData;
        drawingdata->surface_id = 2;
        g_signal_connect( drawing[2], "configure-event", G_CALLBACK( drawing_configure ), drawingdata );
        g_signal_connect( drawing[2], "draw", G_CALLBACK( drawing_draw ), drawingdata );

        drawingdata = new SignalDrawingData;
        drawingdata->surface_id = 3;
        g_signal_connect( drawing[3], "configure-event", G_CALLBACK( drawing_configure ), drawingdata );
        g_signal_connect( drawing[3], "draw", G_CALLBACK( drawing_draw_horizon ), drawingdata );

        drawingdata = new SignalDrawingData;
        drawingdata->surface_id = 4;
        g_signal_connect( drawing[4], "configure-event", G_CALLBACK( drawing_configure ), drawingdata );
        g_signal_connect( drawing[4], "draw", G_CALLBACK( drawing_draw_horizon ), drawingdata );

        drawingdata = new SignalDrawingData;
        drawingdata->surface_id = 5;
        g_signal_connect( drawing[5], "configure-event", G_CALLBACK( drawing_configure ), drawingdata );
        g_signal_connect( drawing[5], "draw", G_CALLBACK( drawing_draw_horizon ), drawingdata );

        g_signal_connect( GTK_WIDGET( gtk_builder_get_object( builder, "button_zoom_in" ) ), "button-press-event", G_CALLBACK( button_press_zoom_in ), NULL );
        g_signal_connect( GTK_WIDGET( gtk_builder_get_object( builder, "button_zoom_out" ) ), "button-press-event", G_CALLBACK( button_press_zoom_out ), NULL );

        g_signal_connect(GTK_WIDGET(gtk_builder_get_object(builder, "button_sample_next")), "button-press-event", G_CALLBACK( button_press_sample_next ), NULL );
        g_signal_connect(GTK_WIDGET(gtk_builder_get_object(builder, "button_sample_prev")), "button-press-event", G_CALLBACK( button_press_sample_prev ), NULL );

        g_signal_connect(GTK_WIDGET(gtk_builder_get_object(builder, "button_sample_next_fast")), "button-press-event", G_CALLBACK( button_press_sample_next_fast ), NULL );
        g_signal_connect(GTK_WIDGET(gtk_builder_get_object(builder, "button_sample_prev_fast")), "button-press-event", G_CALLBACK( button_press_sample_prev_fast ), NULL );

        g_signal_connect(GTK_WIDGET(gtk_builder_get_object(builder, "skipsamples")), "value-changed", G_CALLBACK( spinbutton_skipsamples_value_changed ), NULL );
    }

    void init( GtkBuilder* builder )
    {
        drawing[0] = GTK_WIDGET( gtk_builder_get_object( builder, "drawing1" ) );
        drawing[1] = GTK_WIDGET( gtk_builder_get_object( builder, "drawing2" ) );
        drawing[2] = GTK_WIDGET( gtk_builder_get_object( builder, "drawing3" ) );
        drawing[3] = GTK_WIDGET( gtk_builder_get_object( builder, "drawing_horizon1" ) );
        drawing[4] = GTK_WIDGET( gtk_builder_get_object( builder, "drawing_horizon2" ) );
        drawing[5] = GTK_WIDGET( gtk_builder_get_object( builder, "drawing_horizon3" ) );

        connect_signals( builder );
    }
}
