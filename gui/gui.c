#include <gtk/gtk.h>
#include "config.h"
#include "window.h"

static void
on_app_activate(
	GtkApplication *app,
	gpointer user_data )
{
	GtkWindow *window = window_new( app );

	g_return_if_fail( GTK_IS_WINDOW( window ) );

	gtk_widget_show( GTK_WIDGET( window ) );
}

int
gui_start(
	int argc,
	char *argv[] )
{
	GtkApplication *app;
	int status = EXIT_FAILURE;

	if( g_application_id_is_valid( APP_ID ) )
	{
		app = gtk_application_new( APP_ID, G_APPLICATION_FLAGS_NONE );
		if( app != NULL )
		{
			g_signal_connect( G_OBJECT( app ), "activate", G_CALLBACK( on_app_activate ), NULL );
			status = g_application_run( G_APPLICATION( app ), argc, argv );
			g_object_unref( app );
		}
	}

	return status;
}

