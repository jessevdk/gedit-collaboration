/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

#ifndef __GEDIT_COLLABORATION_BOOKMARK_H__
#define __GEDIT_COLLABORATION_BOOKMARK_H__

#include <glib-object.h>
#include "gedit-collaboration-user.h"

G_BEGIN_DECLS

#define GEDIT_COLLABORATION_TYPE_BOOKMARK		(gedit_collaboration_bookmark_get_type ())
#define GEDIT_COLLABORATION_BOOKMARK(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), GEDIT_COLLABORATION_TYPE_BOOKMARK, GeditCollaborationBookmark))
#define GEDIT_COLLABORATION_BOOKMARK_CONST(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), GEDIT_COLLABORATION_TYPE_BOOKMARK, GeditCollaborationBookmark const))
#define GEDIT_COLLABORATION_BOOKMARK_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), GEDIT_COLLABORATION_TYPE_BOOKMARK, GeditCollaborationBookmarkClass))
#define GEDIT_COLLABORATION_IS_BOOKMARK(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), GEDIT_COLLABORATION_TYPE_BOOKMARK))
#define GEDIT_COLLABORATION_IS_BOOKMARK_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), GEDIT_COLLABORATION_TYPE_BOOKMARK))
#define GEDIT_COLLABORATION_BOOKMARK_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), GEDIT_COLLABORATION_TYPE_BOOKMARK, GeditCollaborationBookmarkClass))

typedef struct _GeditCollaborationBookmark		GeditCollaborationBookmark;
typedef struct _GeditCollaborationBookmarkClass		GeditCollaborationBookmarkClass;
typedef struct _GeditCollaborationBookmarkPrivate	GeditCollaborationBookmarkPrivate;

struct _GeditCollaborationBookmark
{
	GObject parent;

	GeditCollaborationBookmarkPrivate *priv;
};

struct _GeditCollaborationBookmarkClass
{
	GObjectClass parent_class;
};

GType gedit_collaboration_bookmark_get_type (void) G_GNUC_CONST;
void _gedit_collaboration_bookmark_register_type (GTypeModule *module);

GeditCollaborationBookmark *gedit_collaboration_bookmark_new (void);

const gchar *gedit_collaboration_bookmark_get_name (GeditCollaborationBookmark *bookmark);
void gedit_collaboration_bookmark_set_name (GeditCollaborationBookmark *bookmark,
                                            const gchar                *name);

const gchar *gedit_collaboration_bookmark_get_host (GeditCollaborationBookmark *bookmark);
void gedit_collaboration_bookmark_set_host (GeditCollaborationBookmark *bookmark,
                                            const gchar                *host);

const gint gedit_collaboration_bookmark_get_port (GeditCollaborationBookmark *bookmark);
void gedit_collaboration_bookmark_set_port (GeditCollaborationBookmark *bookmark,
                                            gint                        port);

GeditCollaborationUser *gedit_collaboration_bookmark_get_user (GeditCollaborationBookmark *bookmark);

G_END_DECLS

#endif /* __GEDIT_COLLABORATION_BOOKMARK_H__ */
