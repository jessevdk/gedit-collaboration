#include "gedit-collaboration-window-helper.h"

#include <config.h>
#include <glib/gi18n-lib.h>
#include <gedit/gedit-plugin.h>

#include <libinfgtk/inf-gtk-browser-view.h>
#include <libinfgtk/inf-gtk-browser-store.h>
#include <libinfgtk/inf-gtk-io.h>
#include <libinfinity/common/inf-xmpp-manager.h>
#include <libinfinity/inf-config.h>

#ifdef LIBINFINITY_HAVE_AVAHI
#include <libinfinity/common/inf-discovery-avahi.h>
#endif

#define GEDIT_COLLABORATION_WINDOW_HELPER_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE((object), GEDIT_TYPE_COLLABORATION_WINDOW_HELPER, GeditCollaborationWindowHelperPrivate))

/* Properties */
enum
{
	PROP_0,
	PROP_WINDOW
};

struct _GeditCollaborationWindowHelperPrivate
{
	GeditWindow *window;

	InfGtkBrowserStore *browser_store;
	GtkWidget *browser_view;

	Gsasl *sasl_ctx;
};

GEDIT_PLUGIN_DEFINE_TYPE (GeditCollaborationWindowHelper, gedit_collaboration_window_helper,
                          G_TYPE_OBJECT)

static void
gedit_collaboration_window_helper_finalize (GObject *object)
{
	G_OBJECT_CLASS (gedit_collaboration_window_helper_parent_class)->finalize (object);
}

static void
gedit_collaboration_window_helper_set_property (GObject      *object,
                                                guint         prop_id,
                                                const GValue *value,
                                                GParamSpec   *pspec)
{
	GeditCollaborationWindowHelper *self = GEDIT_COLLABORATION_WINDOW_HELPER (object);
	
	switch (prop_id)
	{
		case PROP_WINDOW:
			self->priv->window = GEDIT_WINDOW (g_value_get_object (value));
		break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
gedit_collaboration_window_helper_get_property (GObject    *object,
                                                guint       prop_id,
                                                GValue     *value,
                                                GParamSpec *pspec)
{
	GeditCollaborationWindowHelper *self = GEDIT_COLLABORATION_WINDOW_HELPER (object);
	
	switch (prop_id)
	{
		case PROP_WINDOW:
			g_value_set_object (value, self->priv->window);
		break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static int
sasl_callback (Gsasl          *ctx,
               Gsasl_session  *sctx,
               Gsasl_property  prop)
{
	int rc = GSASL_NO_CALLBACK;

	switch (prop)
	{
		case GSASL_PASSWORD:
			/* TODO */
		break;
		case GSASL_AUTHID:
		case GSASL_ANONYMOUS_TOKEN:
			/* TODO */
		break;
	}

	return rc;
}

static void
on_set_browser (InfGtkBrowserModel *model,
                GtkTreePath        *path,
                GtkTreeIter        *iter,
                InfcBrowser        *browser,
                gpointer            user_data)
{
	if (browser != NULL)
	{
		/* TODO:
		infc_browser_add_plugin (browser, &INF_TEST_GTK_BROWSER_TEXT_PLUGIN); */
	}
}

static void
init_infinity (GeditCollaborationWindowHelper *helper)
{
	InfGtkIo *io;
	InfCommunicationManager *communication_manager;
	InfXmppManager *xmpp_manager;
	InfCertificateCredentials *certificate_credentials;

	io = inf_gtk_io_new ();
	communication_manager = inf_communication_manager_new ();
	xmpp_manager = inf_xmpp_manager_new ();
	certificate_credentials = inf_certificate_credentials_new ();

	gsasl_init (&helper->priv->sasl_ctx);
	gsasl_callback_set (helper->priv->sasl_ctx, sasl_callback);

	helper->priv->browser_store = inf_gtk_browser_store_new (INF_IO (io),
                                                             communication_manager);

	helper->priv->browser_view =
			inf_gtk_browser_view_new_with_model (INF_GTK_BROWSER_MODEL (helper->priv->browser_store));

	gtk_widget_show (helper->priv->browser_view);

	g_signal_connect_after (helper->priv->browser_store,
	                        "set-browser",
	                        G_CALLBACK (on_set_browser),
	                        NULL);

#ifdef LIBINFINITY_HAVE_AVAHI
	InfDiscoveryAvahi *discovery = inf_discovery_avahi_new (INF_IO (io),
	                                                        xmpp_manager,
	                                                        certificate_credentials,
	                                                        helper->priv->sasl_ctx,
	                                                        "ANONYMOUS PLAIN");

	inf_gtk_browser_store_add_discovery (helper->priv->browser_store,
	                                     INF_DISCOVERY (discovery));
#endif

	g_object_unref (io);
	g_object_unref (communication_manager);
	g_object_unref (xmpp_manager);
	inf_certificate_credentials_unref (certificate_credentials);
}

static void
build_ui (GeditCollaborationWindowHelper *helper)
{
	GeditPanel *panel;
	GtkWidget *vbox;
	GtkWidget *sw;

	panel = gedit_window_get_side_panel (helper->priv->window);

	vbox = gtk_vbox_new (FALSE, 3);
	gtk_widget_show (vbox);

	sw = gtk_scrolled_window_new (NULL, NULL);
	gtk_widget_show (sw);

	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
	                                GTK_POLICY_AUTOMATIC,
	                                GTK_POLICY_AUTOMATIC);

	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (sw),
	                                     GTK_SHADOW_ETCHED_IN);

	init_infinity (helper);

	gtk_container_add (GTK_CONTAINER (sw), helper->priv->browser_view);

	gtk_box_pack_start (GTK_BOX (vbox), sw, TRUE, TRUE, 0);
	gedit_panel_add_item (panel, vbox, _("Collaboration"), NULL);
}

static void
gedit_collaboration_window_helper_constructed (GObject *object)
{
	build_ui (GEDIT_COLLABORATION_WINDOW_HELPER (object));
}

static void
gedit_collaboration_window_helper_class_init (GeditCollaborationWindowHelperClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	
	object_class->finalize = gedit_collaboration_window_helper_finalize;
	object_class->set_property = gedit_collaboration_window_helper_set_property;
	object_class->get_property = gedit_collaboration_window_helper_get_property;
	object_class->constructed = gedit_collaboration_window_helper_constructed;

	g_object_class_install_property (object_class,
	                                 PROP_WINDOW,
	                                 g_param_spec_object ("window",
	                                                      "Window",
	                                                      "The gedit window",
	                                                      GEDIT_TYPE_WINDOW,
	                                                      G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));

	g_type_class_add_private (object_class, sizeof(GeditCollaborationWindowHelperPrivate));
}

static void
gedit_collaboration_window_helper_init (GeditCollaborationWindowHelper *self)
{
	self->priv = GEDIT_COLLABORATION_WINDOW_HELPER_GET_PRIVATE (self);
}

GeditCollaborationWindowHelper *
gedit_collaboration_window_helper_new (GeditWindow *window)
{
	return g_object_new (GEDIT_TYPE_COLLABORATION_WINDOW_HELPER,
	                     "window", window,
	                     NULL);
}
