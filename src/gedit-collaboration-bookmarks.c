/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

#include "gedit-collaboration-bookmarks.h"

#include <gedit/gedit-plugin.h>
#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>

#include <string.h>

#define GEDIT_COLLABORATION_BOOKMARKS_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE((object), GEDIT_COLLABORATION_TYPE_BOOKMARKS, GeditCollaborationBookmarksPrivate))

struct _GeditCollaborationBookmarksPrivate
{
	gchar *filename;
	GKeyFile *keyfile;

	GList *bookmarks;
	gulong idle_save_id;
};

/* Properties */
enum
{
	PROP_0,
	PROP_FILENAME
};

/* Signals */
enum
{
	ADDED,
	REMOVED,
	NUM_SIGNALS
};

static guint bookmarks_signals[NUM_SIGNALS] = {0,};
static GeditCollaborationBookmarks *bookmarks_default;

GEDIT_PLUGIN_DEFINE_TYPE (GeditCollaborationBookmarks, gedit_collaboration_bookmarks, \
                          G_TYPE_OBJECT)

static void
save_bookmark_property (xmlDocPtr    doc,
                        xmlNodePtr   parent,
                        gchar const *name,
                        gchar const *value)
{
	xmlNodePtr item = xmlNewDocNode (doc, NULL, (xmlChar *)name, NULL);
	xmlNodePtr text = xmlNewDocText (doc, (xmlChar *)value);

	xmlAddChild (item, text);
	xmlAddChild (parent, item);
}

static void
save_bookmark (xmlDocPtr                   doc,
               xmlNodePtr                  root,
               GeditCollaborationBookmark *bookmark)
{
	gchar *port;
	GeditCollaborationUser *user;
	gchar buffer[G_ASCII_DTOSTR_BUF_SIZE];

	xmlNodePtr bm = xmlNewDocNode (doc, NULL, (xmlChar *)"bookmark", NULL);
	xmlAddChild (root, bm);

	save_bookmark_property (doc, bm, "name", gedit_collaboration_bookmark_get_name (bookmark));
	save_bookmark_property (doc, bm, "host", gedit_collaboration_bookmark_get_host (bookmark));

	port = g_strdup_printf ("%d", gedit_collaboration_bookmark_get_port (bookmark));
	save_bookmark_property (doc, bm, "port", port);
	g_free (port);

	user = gedit_collaboration_bookmark_get_user (bookmark);
	save_bookmark_property (doc, bm, "username", gedit_collaboration_user_get_name (user));

	g_ascii_dtostr (buffer,
	                G_ASCII_DTOSTR_BUF_SIZE,
	                gedit_collaboration_user_get_hue (user));

	save_bookmark_property (doc, bm, "hue", buffer);
}

void
gedit_collaboration_bookmarks_save (GeditCollaborationBookmarks *bookmarks)
{
	xmlDocPtr doc;
	xmlNodePtr root;
	GList *item;
	xmlChar *mem;
	int size;
	gchar *dirn;

	g_return_if_fail (GEDIT_COLLABORATION_IS_BOOKMARKS (bookmarks));

	doc = xmlNewDoc ((xmlChar *)"1.0");
	root = xmlNewDocNode (doc, NULL, (xmlChar *)"infinote-bookmarks", NULL);

	xmlDocSetRootElement (doc, root);

	for (item = bookmarks->priv->bookmarks; item; item = g_list_next (item))
	{
		GeditCollaborationBookmark *bookmark = item->data;
		save_bookmark (doc, root, bookmark);
	}

	xmlIndentTreeOutput = 1;
	xmlDocDumpFormatMemoryEnc (doc,
	                           &mem,
	                           &size,
	                           xmlGetCharEncodingName (XML_CHAR_ENCODING_UTF8),
	                           1);

	dirn = g_path_get_dirname (bookmarks->priv->filename);
	g_mkdir_with_parents (dirn, 0755);
	g_free (dirn);

	g_file_set_contents (bookmarks->priv->filename,
	                     (gchar const *)mem,
	                     size,
	                     NULL);

	xmlFree (mem);
	xmlFreeDoc (doc);
}

static void
gedit_collaboration_bookmarks_finalize (GObject *object)
{
	GeditCollaborationBookmarks *bookmarks;

	bookmarks = GEDIT_COLLABORATION_BOOKMARKS (object);

	if (bookmarks->priv->filename)
	{
		gedit_collaboration_bookmarks_save (bookmarks);
	}

	if (bookmarks->priv->idle_save_id)
	{
		g_source_remove (bookmarks->priv->idle_save_id);
	}

	g_free (bookmarks->priv->filename);

	g_list_foreach (bookmarks->priv->bookmarks,
	                (GFunc)g_object_unref,
	                NULL);

	g_list_free (bookmarks->priv->bookmarks);

	G_OBJECT_CLASS (gedit_collaboration_bookmarks_parent_class)->finalize (object);
}

static void
gedit_collaboration_bookmarks_set_property (GObject      *object,
                                            guint         prop_id,
                                            const GValue *value,
                                            GParamSpec   *pspec)
{
	GeditCollaborationBookmarks *self = GEDIT_COLLABORATION_BOOKMARKS (object);

	switch (prop_id)
	{
		case PROP_FILENAME:
			g_free (self->priv->filename);
			self->priv->filename = g_value_dup_string (value);
		break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
gedit_collaboration_bookmarks_get_property (GObject    *object,
                                            guint       prop_id,
                                            GValue     *value,
                                            GParamSpec *pspec)
{
	GeditCollaborationBookmarks *self = GEDIT_COLLABORATION_BOOKMARKS (object);

	switch (prop_id)
	{
		case PROP_FILENAME:
			g_value_set_string (value, self->priv->filename);
		break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static const gchar *
get_xml_content (xmlNodePtr node)
{
	const gchar *ret = NULL;

	if (node->children && node->children->type == XML_TEXT_NODE)
	{
		ret = (const gchar *)node->children->content;
	}

	return ret != NULL ? ret : "";
}

static GeditCollaborationBookmark *
parse_bookmark (GeditCollaborationBookmarks *bookmarks,
                xmlNodePtr                   node)
{
	xmlNodePtr child = node->children;
	GeditCollaborationBookmark *bookmark = gedit_collaboration_bookmark_new ();
	GeditCollaborationUser *user = gedit_collaboration_bookmark_get_user (bookmark);

	while (child)
	{
		if (child->type != XML_ELEMENT_NODE)
		{
			child = child->next;
			continue;
		}

		if (strcmp ((gchar const *)child->name, "name") == 0)
		{
			gedit_collaboration_bookmark_set_name (bookmark,
			                                       get_xml_content (child));
		}
		else if (strcmp ((gchar const *)child->name, "host") == 0)
		{
			gedit_collaboration_bookmark_set_host (bookmark,
			                                       get_xml_content (child));
		}
		else if (strcmp ((gchar const *)child->name, "port") == 0)
		{
			const gchar *content = get_xml_content (child);
			gedit_collaboration_bookmark_set_port (bookmark,
			                                       content && *content ? atoi (content) : 0);
		}
		else if (strcmp ((gchar const *)child->name, "username") == 0)
		{
			gedit_collaboration_user_set_name (user,
			                                   get_xml_content (child));
		}
		else if (strcmp ((gchar const *)child->name, "hue") == 0)
		{
			gedit_collaboration_user_set_hue (user,
			                                  g_ascii_strtod (get_xml_content (child), NULL));
		}

		child = child->next;
	}

	if (gedit_collaboration_bookmark_get_name (bookmark) == NULL ||
	    gedit_collaboration_bookmark_get_host (bookmark) == NULL)
	{
		g_object_unref (bookmark);
		return NULL;
	}

	return bookmark;
}

static gboolean
bookmarks_idle_save (GeditCollaborationBookmarks *bookmarks)
{
	bookmarks->priv->idle_save_id = 0;
	gedit_collaboration_bookmarks_save (bookmarks);

	return FALSE;
}

static void
on_bookmark_changed (GeditCollaborationBookmark  *bookmark,
                     GParamSpec                  *spec,
                     GeditCollaborationBookmarks *bookmarks)
{
	if (bookmarks->priv->idle_save_id == 0)
	{
		bookmarks->priv->idle_save_id = g_idle_add ((GSourceFunc)bookmarks_idle_save,
		                                            bookmarks);
	}
}

static void
load_bookmarks (GeditCollaborationBookmarks *bookmarks)
{
	xmlDocPtr doc;
	xmlNodePtr root;
	xmlXPathContextPtr ctx;
	xmlXPathObjectPtr obj;
	int i;

	doc = xmlReadFile (bookmarks->priv->filename, NULL, XML_PARSE_NOWARNING);

	if (!doc)
	{
		return;
	}

	root = xmlDocGetRootElement (doc);

	ctx = xmlXPathNewContext (doc);
	ctx->node = root;

	if (!ctx)
	{
		xmlFreeDoc (doc);
		return;
	}

	obj = xmlXPathEvalExpression ((xmlChar *)"/infinote-bookmarks/bookmark", ctx);

	if (!obj)
	{
		xmlXPathFreeContext (ctx);
		xmlFreeDoc (doc);
		return;
	}

	for (i = 0; i < obj->nodesetval->nodeNr; ++i)
	{
		xmlNodePtr node = obj->nodesetval->nodeTab[i];
		GeditCollaborationBookmark *bookmark;

		bookmark = parse_bookmark (bookmarks, node);

		if (bookmark != NULL)
		{
			bookmarks->priv->bookmarks = g_list_prepend (bookmarks->priv->bookmarks,
			                                             bookmark);

			g_signal_connect (bookmark,
			                  "notify",
			                  G_CALLBACK (on_bookmark_changed),
			                  bookmarks);
		}
	}

	bookmarks->priv->bookmarks = g_list_reverse (bookmarks->priv->bookmarks);

	xmlFreeDoc (doc);
	xmlXPathFreeObject (obj);
	xmlXPathFreeContext (ctx);
}

static void
gedit_collaboration_bookmarks_constructed (GObject *object)
{
	GeditCollaborationBookmarks *bookmarks;

	bookmarks = GEDIT_COLLABORATION_BOOKMARKS (object);

	if (!bookmarks->priv->filename)
	{
		return;
	}

	load_bookmarks (bookmarks);
}

static void
gedit_collaboration_bookmarks_class_init (GeditCollaborationBookmarksClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->finalize = gedit_collaboration_bookmarks_finalize;
	object_class->set_property = gedit_collaboration_bookmarks_set_property;
	object_class->get_property = gedit_collaboration_bookmarks_get_property;
	object_class->constructed = gedit_collaboration_bookmarks_constructed;

	g_object_class_install_property (object_class,
	                                 PROP_FILENAME,
	                                 g_param_spec_string ("filename",
	                                                      "Filename",
	                                                      "Filename",
	                                                      NULL,
	                                                      G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));

	bookmarks_signals[ADDED] =
		g_signal_new ("added",
		              G_TYPE_FROM_CLASS (klass),
		              G_SIGNAL_RUN_LAST,
		              0,
		              NULL,
		              NULL,
		              g_cclosure_marshal_VOID__OBJECT,
		              G_TYPE_NONE,
		              1,
		              G_TYPE_OBJECT);

	bookmarks_signals[REMOVED] =
		g_signal_new ("removed",
		              G_TYPE_FROM_CLASS (klass),
		              G_SIGNAL_RUN_LAST,
		              0,
		              NULL,
		              NULL,
		              g_cclosure_marshal_VOID__OBJECT,
		              G_TYPE_NONE,
		              1,
		              G_TYPE_OBJECT);

	g_type_class_add_private (object_class, sizeof(GeditCollaborationBookmarksPrivate));
}

static void
gedit_collaboration_bookmarks_init (GeditCollaborationBookmarks *self)
{
	self->priv = GEDIT_COLLABORATION_BOOKMARKS_GET_PRIVATE (self);
}

GList *
gedit_collaboration_bookmarks_get_bookmarks (GeditCollaborationBookmarks *bookmarks)
{
	g_return_val_if_fail (GEDIT_COLLABORATION_IS_BOOKMARKS (bookmarks), NULL);

	return bookmarks->priv->bookmarks;
}

GeditCollaborationBookmarks *
gedit_collaboration_bookmarks_initialize (const gchar *filename)
{
	if (bookmarks_default == NULL)
	{
		bookmarks_default = g_object_new (GEDIT_COLLABORATION_TYPE_BOOKMARKS,
		                                  "filename", filename,
		                                  NULL);
	}

	return bookmarks_default;
}

GeditCollaborationBookmarks *
gedit_collaboration_bookmarks_get_default ()
{
	g_return_val_if_fail (bookmarks_default != NULL, NULL);
	return bookmarks_default;
}

void
gedit_collaboration_bookmarks_add (GeditCollaborationBookmarks *bookmarks,
                                   GeditCollaborationBookmark  *bookmark)
{
	g_return_if_fail (GEDIT_COLLABORATION_IS_BOOKMARKS (bookmarks));

	bookmarks->priv->bookmarks = g_list_append (bookmarks->priv->bookmarks,
	                                            g_object_ref (bookmark));

	gedit_collaboration_bookmarks_save (bookmarks);
	g_signal_emit (bookmarks, bookmarks_signals[ADDED], 0, bookmark);

	g_signal_connect (bookmark,
	                  "notify",
	                  G_CALLBACK (on_bookmark_changed),
	                  bookmarks);
}

void
gedit_collaboration_bookmarks_remove (GeditCollaborationBookmarks *bookmarks,
                                      GeditCollaborationBookmark  *bookmark)
{
	GList *item;

	g_return_if_fail (GEDIT_COLLABORATION_IS_BOOKMARKS (bookmarks));

	item = g_list_find (bookmarks->priv->bookmarks, bookmark);

	if (item != NULL)
	{
		bookmarks->priv->bookmarks = g_list_delete_link (bookmarks->priv->bookmarks,
		                                                 item);

		gedit_collaboration_bookmarks_save (bookmarks);

		g_signal_handlers_disconnect_by_func (bookmark,
		                                      G_CALLBACK (on_bookmark_changed),
		                                      bookmarks);

		g_signal_emit (bookmarks, bookmarks_signals[REMOVED], 0, bookmark);
		g_object_unref (bookmark);
	}
}
