/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

#ifndef __GEDIT_COLLABORATION_USER_H__
#define __GEDIT_COLLABORATION_USER_H__

#include <glib-object.h>

G_BEGIN_DECLS

#define GEDIT_COLLABORATION_TYPE_USER				(gedit_collaboration_user_get_type ())
#define GEDIT_COLLABORATION_USER(obj)				(G_TYPE_CHECK_INSTANCE_CAST ((obj), GEDIT_COLLABORATION_TYPE_USER, GeditCollaborationUser))
#define GEDIT_COLLABORATION_USER_CONST(obj)			(G_TYPE_CHECK_INSTANCE_CAST ((obj), GEDIT_COLLABORATION_TYPE_USER, GeditCollaborationUser const))
#define GEDIT_COLLABORATION_USER_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST ((klass), GEDIT_COLLABORATION_TYPE_USER, GeditCollaborationUserClass))
#define GEDIT_COLLABORATION_IS_USER(obj)			(G_TYPE_CHECK_INSTANCE_TYPE ((obj), GEDIT_COLLABORATION_TYPE_USER))
#define GEDIT_COLLABORATION_IS_USER_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), GEDIT_COLLABORATION_TYPE_USER))
#define GEDIT_COLLABORATION_USER_GET_CLASS(obj)		(G_TYPE_INSTANCE_GET_CLASS ((obj), GEDIT_COLLABORATION_TYPE_USER, GeditCollaborationUserClass))

typedef struct _GeditCollaborationUser			GeditCollaborationUser;
typedef struct _GeditCollaborationUserClass		GeditCollaborationUserClass;
typedef struct _GeditCollaborationUserPrivate	GeditCollaborationUserPrivate;

struct _GeditCollaborationUser {
	GObject parent;

	GeditCollaborationUserPrivate *priv;
};

struct _GeditCollaborationUserClass {
	GObjectClass parent_class;
};

GType gedit_collaboration_user_get_type (void) G_GNUC_CONST;
GeditCollaborationUser *gedit_collaboration_user_new (const gchar *name);

GeditCollaborationUser	*gedit_collaboration_user_get_default (void);

const gchar 		*gedit_collaboration_user_get_name (GeditCollaborationUser *user);
void			 gedit_collaboration_user_set_name (GeditCollaborationUser *user,
                                                            const gchar            *name);

gdouble			 gedit_collaboration_user_get_hue (GeditCollaborationUser *user);
void			 gedit_collaboration_user_set_hue (GeditCollaborationUser *user,
                                                           gdouble                 hue);

G_END_DECLS

#endif /* __GEDIT_COLLABORATION_USER_H__ */
