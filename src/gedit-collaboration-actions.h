/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

#ifndef __GEDIT_COLLABORATION_ACTIONS_H__
#define __GEDIT_COLLABORATION_ACTIONS_H__

#include "gedit-collaboration-window-helper.h"

G_BEGIN_DECLS

void on_action_file_new (GtkAction                      *action,
                         GeditCollaborationWindowHelper *helper);

void on_action_folder_new (GtkAction                      *action,
                           GeditCollaborationWindowHelper *helper);

void on_action_item_delete (GtkAction                      *action,
                            GeditCollaborationWindowHelper *helper);

void on_action_bookmark_new (GtkAction                      *action,
                             GeditCollaborationWindowHelper *helper);

void on_action_bookmark_edit (GtkAction                      *action,
                              GeditCollaborationWindowHelper *helper);

void on_action_session_disconnect (GtkAction                      *action,
                                   GeditCollaborationWindowHelper *helper);

G_END_DECLS

#endif /* __GEDIT_COLLABORATION_ACTIONS_H__ */

