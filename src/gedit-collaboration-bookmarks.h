/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

#ifndef __GEDIT_COLLABORATION_BOOKMARKS_H__
#define __GEDIT_COLLABORATION_BOOKMARKS_H__

#include <glib-object.h>
#include "gedit-collaboration-bookmark.h"

G_BEGIN_DECLS

#define GEDIT_COLLABORATION_TYPE_BOOKMARKS				(gedit_collaboration_bookmarks_get_type ())
#define GEDIT_COLLABORATION_BOOKMARKS(obj)				(G_TYPE_CHECK_INSTANCE_CAST ((obj), GEDIT_COLLABORATION_TYPE_BOOKMARKS, GeditCollaborationBookmarks))
#define GEDIT_COLLABORATION_BOOKMARKS_CONST(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), GEDIT_COLLABORATION_TYPE_BOOKMARKS, GeditCollaborationBookmarks const))
#define GEDIT_COLLABORATION_BOOKMARKS_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST ((klass), GEDIT_COLLABORATION_TYPE_BOOKMARKS, GeditCollaborationBookmarksClass))
#define GEDIT_COLLABORATION_IS_BOOKMARKS(obj)			(G_TYPE_CHECK_INSTANCE_TYPE ((obj), GEDIT_COLLABORATION_TYPE_BOOKMARKS))
#define GEDIT_COLLABORATION_IS_BOOKMARKS_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), GEDIT_COLLABORATION_TYPE_BOOKMARKS))
#define GEDIT_COLLABORATION_BOOKMARKS_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), GEDIT_COLLABORATION_TYPE_BOOKMARKS, GeditCollaborationBookmarksClass))

typedef struct _GeditCollaborationBookmarks			GeditCollaborationBookmarks;
typedef struct _GeditCollaborationBookmarksClass	GeditCollaborationBookmarksClass;
typedef struct _GeditCollaborationBookmarksPrivate	GeditCollaborationBookmarksPrivate;

struct _GeditCollaborationBookmarks {
	GObject parent;

	GeditCollaborationBookmarksPrivate *priv;
};

struct _GeditCollaborationBookmarksClass {
	GObjectClass parent_class;
};

GType gedit_collaboration_bookmarks_register_type (GTypeModule *module);
GType gedit_collaboration_bookmarks_get_type (void) G_GNUC_CONST;

GeditCollaborationBookmarks *gedit_collaboration_bookmarks_initialize (gchar const *filename);
GeditCollaborationBookmarks *gedit_collaboration_bookmarks_get_default (void);

GList *gedit_collaboration_bookmarks_get_bookmarks (GeditCollaborationBookmarks *bookmarks);
void gedit_collaboration_bookmarks_save (GeditCollaborationBookmarks *bookmarks);

void gedit_collaboration_bookmarks_remove (GeditCollaborationBookmarks *bookmarks,
                                           GeditCollaborationBookmark  *bookmark);

void gedit_collaboration_bookmarks_add (GeditCollaborationBookmarks *bookmarks,
                                        GeditCollaborationBookmark  *bookmark);

G_END_DECLS

#endif /* __GEDIT_COLLABORATION_BOOKMARKS_H__ */
