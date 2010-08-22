/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

#ifndef __GEDIT_COLLABORATION_BOOKMARK_DIALOG_H__
#define __GEDIT_COLLABORATION_BOOKMARK_DIALOG_H__

#include <gtk/gtk.h>
#include "gedit-collaboration-bookmarks.h"

G_BEGIN_DECLS

#define GEDIT_COLLABORATION_TYPE_BOOKMARK_DIALOG		(gedit_collaboration_bookmark_dialog_get_type ())
#define GEDIT_COLLABORATION_BOOKMARK_DIALOG(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), GEDIT_COLLABORATION_TYPE_BOOKMARK_DIALOG, GeditCollaborationBookmarkDialog))
#define GEDIT_COLLABORATION_BOOKMARK_DIALOG_CONST(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), GEDIT_COLLABORATION_TYPE_BOOKMARK_DIALOG, GeditCollaborationBookmarkDialog const))
#define GEDIT_COLLABORATION_BOOKMARK_DIALOG_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), GEDIT_COLLABORATION_TYPE_BOOKMARK_DIALOG, GeditCollaborationBookmarkDialogClass))
#define GEDIT_COLLABORATION_IS_BOOKMARK_DIALOG(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), GEDIT_COLLABORATION_TYPE_BOOKMARK_DIALOG))
#define GEDIT_COLLABORATION_IS_BOOKMARK_DIALOG_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), GEDIT_COLLABORATION_TYPE_BOOKMARK_DIALOG))
#define GEDIT_COLLABORATION_BOOKMARK_DIALOG_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), GEDIT_COLLABORATION_TYPE_BOOKMARK_DIALOG, GeditCollaborationBookmarkDialogClass))

typedef struct _GeditCollaborationBookmarkDialog	GeditCollaborationBookmarkDialog;
typedef struct _GeditCollaborationBookmarkDialogClass	GeditCollaborationBookmarkDialogClass;
typedef struct _GeditCollaborationBookmarkDialogPrivate	GeditCollaborationBookmarkDialogPrivate;

struct _GeditCollaborationBookmarkDialog
{
	GtkDialog parent;

	GeditCollaborationBookmarkDialogPrivate *priv;
};

struct _GeditCollaborationBookmarkDialogClass
{
	GtkDialogClass parent_class;
};

GType gedit_collaboration_bookmark_dialog_get_type (void) G_GNUC_CONST;
void _gedit_collaboration_bookmark_dialog_register_type (GTypeModule *module);

GtkWidget	*gedit_collaboration_bookmark_dialog_new (const gchar                *data_dir,
                                                          GeditCollaborationBookmark *bookmark);

G_END_DECLS

#endif /* __GEDIT_COLLABORATION_BOOKMARK_DIALOG_H__ */
