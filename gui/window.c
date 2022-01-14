#include <stdlib.h>
#include <gtk/gtk.h>
#include "config.h"


/*
	definitions
*/
#define TIMER_START_DECISEC 0
#define TIME_MAXIMUM_DECISEC 36000

#define ADJUSTMNENT_DEFAULT_SECONDS ( 60.0 )
#define ADJUSTMNENT_MAXIMUM_SECONDS ( 3600.0 )

#define LABEL_MINIMUM_FONT_SIZE 90
#define LABEL_MAXIMUM_FONT_SIZE 180
#define LABEL_MAXIMUM_TEXT "3600.0"
#define LABEL_MAXIMUM_WIDTH_CHARS ( (int)sizeof( LABEL_MAXIMUM_TEXT ) - 1 )

enum button_state
{
	BUTTON_STATE_START,
	BUTTON_STATE_STOP,

	N_BUTTON_STATE
};


/*
	structures
*/
typedef struct _WindowPrivate WindowPrivate;
struct _WindowPrivate
{
	GtkSpinButton *spin;
	GtkLabel *label;
	GtkButton *button;
	enum button_state buttonstate;
	gint timer;
	size_t decisec;
	size_t maxdecisec;
};


/*
	function prototypes
*/
static G_DEFINE_QUARK( window-private, window_private )
static WindowPrivate* window_private_new( void );
static void window_private_free( WindowPrivate *priv );
static WindowPrivate* window_get_private( GtkWindow *window );

static GtkSpinButton* spin_button_new( void );

static G_DEFINE_QUARK( label-css-provider, label_css_provider )
static GtkLabel* label_new( void );
static void label_set_font_size( GtkLabel *label, gint font_size );

static GtkButton* button_new( GtkWindow *window );
static void on_button_clicked( GtkButton *button, gpointer user_data );

static gint timer_new( gpointer user_data );
static void timer_free( gint timer, gpointer user_data );
static gint timer_timeout_callback( gpointer user_data );

static gboolean on_key_pressed( GtkEventControllerKey *self, guint keyval, guint keycode, GdkModifierType state, gpointer user_data );


/*
	private
*/
static WindowPrivate*
window_private_new(
	void )
{
	return g_new( WindowPrivate, 1 );
}

static void
window_private_free(
	WindowPrivate *priv )
{
	g_free( priv );
}

static WindowPrivate*
window_get_private(
	GtkWindow *window )
{
	g_return_val_if_fail( GTK_IS_WINDOW( window ), NULL );

	return g_object_get_qdata( G_OBJECT( window ), window_private_quark() );
}


/*
	spin button
*/
static GtkSpinButton*
spin_button_new(
	void )
{
	GtkAdjustment *adj;
	GtkSpinButton *spin;

	adj = GTK_ADJUSTMENT( gtk_adjustment_new( ADJUSTMNENT_DEFAULT_SECONDS, 0.0, ADJUSTMNENT_MAXIMUM_SECONDS, 1.0, 1.0, 1.0 ) );
	spin = GTK_SPIN_BUTTON( gtk_spin_button_new( adj, 1.0, 0 ) );

	gtk_spin_button_set_update_policy( spin, GTK_UPDATE_IF_VALID );

	return spin;
}


/*
	label
*/
static GtkLabel*
label_new(
	void )
{
	GtkCssProvider *css_provider;
	GtkStyleContext *style_context;
	GtkLabel *label = GTK_LABEL( gtk_label_new( "0.0" ) );

	gtk_label_set_wrap( label, FALSE );
	gtk_label_set_width_chars( label, LABEL_MAXIMUM_WIDTH_CHARS );
	gtk_label_set_max_width_chars( label, LABEL_MAXIMUM_WIDTH_CHARS );
	gtk_label_set_single_line_mode( label, TRUE );

	css_provider = gtk_css_provider_new();
	style_context = gtk_widget_get_style_context( GTK_WIDGET( label ) );
	gtk_style_context_add_provider( style_context, GTK_STYLE_PROVIDER( css_provider ), GTK_STYLE_PROVIDER_PRIORITY_USER );

	g_object_set_qdata( G_OBJECT( label ), label_css_provider_quark(), css_provider );
	label_set_font_size( label, LABEL_MINIMUM_FONT_SIZE );

	return label;
}

static void
label_set_font_size(
	GtkLabel *label,
	gint font_size )
{
	const gchar *css_provider_fmt = "label { font-size: %dpx; }";
	gchar *css_provider_data;
	gint css_provider_data_len;
	GtkCssProvider *css_provider;

	g_return_if_fail( GTK_IS_LABEL( label ) );
	g_return_if_fail( font_size >= LABEL_MINIMUM_FONT_SIZE && font_size <= LABEL_MAXIMUM_FONT_SIZE );

	css_provider = GTK_CSS_PROVIDER( g_object_get_qdata( G_OBJECT( label ), label_css_provider_quark() ) );

	css_provider_data_len = g_snprintf( NULL, 0, css_provider_fmt, font_size );
	css_provider_data = g_new( gchar, css_provider_data_len + 1 );
	g_snprintf( css_provider_data, css_provider_data_len + 1, css_provider_fmt, font_size );
	gtk_css_provider_load_from_data( css_provider, css_provider_data, css_provider_data_len );
	g_free( css_provider_data );
}


/*
	button
*/
static GtkButton*
button_new(
	GtkWindow *window )
{
	GtkButton *button;

	g_return_val_if_fail( GTK_IS_WINDOW( window ), NULL );

	button = GTK_BUTTON( gtk_button_new_with_label( "Start" ) );
	g_signal_connect( button, "clicked", G_CALLBACK( on_button_clicked ), window );

	return button;
}

static void
on_button_clicked(
	GtkButton *button,
	gpointer user_data )
{
	GtkWindow *window = GTK_WINDOW( user_data );
	WindowPrivate *priv = window_get_private( window );

	switch( priv->buttonstate )
	{
		case BUTTON_STATE_START:
			priv->timer = timer_new( user_data );
			break;

		case BUTTON_STATE_STOP:
			timer_free( priv->timer, user_data );
			break;

		default:
			break;
	}
}

/*
	timer
*/
static gint
timer_new(
	gpointer user_data )
{
	GtkWindow *window = GTK_WINDOW( user_data );
	WindowPrivate *priv = window_get_private( window );
	
	priv->decisec = 1;
	priv->maxdecisec = 10 * gtk_spin_button_get_value_as_int( priv->spin );
	priv->buttonstate = BUTTON_STATE_STOP;

	gtk_widget_set_sensitive( GTK_WIDGET( priv->spin ), FALSE );
	gtk_button_set_label( priv->button, "Stop" );
	gtk_label_set_text( priv->label, "0.0" );

	return g_timeout_add( 100, timer_timeout_callback, user_data );
}

static void
timer_free(
	gint timer,
	gpointer user_data )
{
	GtkWindow *window = GTK_WINDOW( user_data );
	WindowPrivate *priv = window_get_private( window );

	priv->decisec = 1;
	priv->buttonstate = BUTTON_STATE_START;

	gtk_widget_set_sensitive( GTK_WIDGET( priv->spin ), TRUE );
	gtk_button_set_label( priv->button, "Start" );

	g_source_remove( timer );
}

static gint
timer_timeout_callback(
	gpointer user_data )
{
	GtkWindow *window = GTK_WINDOW( user_data );
	WindowPrivate *priv = window_get_private( window );
	char text[1024];
	size_t dec, denom;

	if( priv->decisec <= priv->maxdecisec )
	{
		dec = priv->decisec / 10;
		denom = priv->decisec - 10 * dec;
		snprintf( text, 1024, "%zu.%zu", dec, denom );
		gtk_label_set_text( priv->label, text );
		priv->decisec++;
		return TRUE;
	}

	timer_free( priv->timer, user_data );
	return FALSE;
}


/*
	window
*/
static gboolean
on_key_pressed(
	GtkEventControllerKey *self,
	guint keyval,
	guint keycode,
	GdkModifierType state,
	gpointer user_data )
{
	GtkWindow *window = GTK_WINDOW( user_data );

	if( keyval == GDK_KEY_q && ( state & GDK_CONTROL_MASK ) )
	{
		gtk_window_destroy( window );
		return FALSE;
	}

	return FALSE;
}


/*
	public
*/
GtkWindow*
window_new(
	GtkApplication *app )
{
	WindowPrivate *priv;
	GtkWindow *window;
	GtkBox *vbox;
	GtkSpinButton *spin;
	GtkLabel *label;
	GtkButton *button;
	GtkEventControllerKey *key;

	/* create window */
	window = GTK_WINDOW( gtk_application_window_new( app ) );
	gtk_window_set_title( window, PROGRAM_NAME );
	gtk_window_set_resizable( window, FALSE );
	key = GTK_EVENT_CONTROLLER_KEY( gtk_event_controller_key_new() );
	gtk_widget_add_controller( GTK_WIDGET( window ), GTK_EVENT_CONTROLLER( key ) );
	g_signal_connect( G_OBJECT( key ), "key-pressed", G_CALLBACK( on_key_pressed ), window );

	/* create widgets */
	vbox = GTK_BOX( gtk_box_new( GTK_ORIENTATION_VERTICAL, 1 ) );
	spin = spin_button_new();
	label = label_new();
	button = button_new( window );

	/* layout widgets */
	gtk_window_set_child( window, GTK_WIDGET( vbox ) );
	gtk_box_append( vbox, GTK_WIDGET( spin ) );
	gtk_box_append( vbox, GTK_WIDGET( label ) );
	gtk_box_append( vbox, GTK_WIDGET( button ) );

	/* collect private */
	priv = window_private_new();
	priv->spin = spin;
	priv->label = label;
	priv->button = button;
	priv->buttonstate = BUTTON_STATE_START;
	priv->timer = 0;
	priv->decisec = TIMER_START_DECISEC;
	priv->maxdecisec = TIME_MAXIMUM_DECISEC;
	g_object_set_qdata_full( G_OBJECT( window ), window_private_quark(), priv, (GDestroyNotify)window_private_free ); 

	return window;
}

