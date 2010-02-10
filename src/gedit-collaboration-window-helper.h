#ifndef __GEDIT_COLLABORATION_WINDOW_HELPER_H__
#define __GEDIT_COLLABORATION_WINDOW_HELPER_H__

#include <glib-object.h>
#include <gedit/gedit-window.h>

G_BEGIN_DECLS

#define GEDIT_TYPE_COLLABORATION_WINDOW_HELPER				(gedit_collaboration_window_helper_get_type ())
#define GEDIT_COLLABORATION_WINDOW_HELPER(obj)				(G_TYPE_CHECK_INSTANCE_CAST ((obj), GEDIT_TYPE_COLLABORATION_WINDOW_HELPER, GeditCollaborationWindowHelper))
#define GEDIT_COLLABORATION_WINDOW_HELPER_CONST(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), GEDIT_TYPE_COLLABORATION_WINDOW_HELPER, GeditCollaborationWindowHelper const))
#define GEDIT_COLLABORATION_WINDOW_HELPER_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST ((klass), GEDIT_TYPE_COLLABORATION_WINDOW_HELPER, GeditCollaborationWindowHelperClass))
#define GEDIT_IS_COLLABORATION_WINDOW_HELPER(obj)			(G_TYPE_CHECK_INSTANCE_TYPE ((obj), GEDIT_TYPE_COLLABORATION_WINDOW_HELPER))
#define GEDIT_IS_COLLABORATION_WINDOW_HELPER_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), GEDIT_TYPE_COLLABORATION_WINDOW_HELPER))
#define GEDIT_COLLABORATION_WINDOW_HELPER_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), GEDIT_TYPE_COLLABORATION_WINDOW_HELPER, GeditCollaborationWindowHelperClass))

typedef struct _GeditCollaborationWindowHelper			GeditCollaborationWindowHelper;
typedef struct _GeditCollaborationWindowHelperClass		GeditCollaborationWindowHelperClass;
typedef struct _GeditCollaborationWindowHelperPrivate	GeditCollaborationWindowHelperPrivate;

struct _GeditCollaborationWindowHelper {
	GObject parent;
	
	GeditCollaborationWindowHelperPrivate *priv;
};

struct _GeditCollaborationWindowHelperClass {
	GObjectClass parent_class;
};

GType gedit_collaboration_window_helper_get_type (void) G_GNUC_CONST;

GeditCollaborationWindowHelper *gedit_collaboration_window_helper_new (GeditWindow *window);
GType gedit_collaboration_window_helper_register_type (GTypeModule *module);

G_END_DECLS

#endif /* __GEDIT_COLLABORATION_WINDOW_HELPER_H__ */
