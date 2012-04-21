/* tidy-texture-reflection.h: adds a reflection on a texture
 *
 * Copyright (C) 2007 OpenedHand
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see
 * <http://www.gnu.org/licenses/>.
 *
 * Author: Matthew Allum <mallum@o-hand.com>
 */

#ifndef __TIDY_TEXTURE_REFLECTION_H__
#define __TIDY_TEXTURE_REFLECTION_H__

#include "tidy-actor.h"
#include <clutter/clutter.h>
#include <cogl/cogl.h>

G_BEGIN_DECLS

#define TIDY_TYPE_TEXTURE_REFLECTION            (tidy_texture_reflection_get_type ())
#define TIDY_TEXTURE_REFLECTION(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), TIDY_TYPE_TEXTURE_REFLECTION, TidyTextureReflection))
#define TIDY_IS_TEXTURE_REFLECTION(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TIDY_TYPE_TEXTURE_REFLECTION))
#define TIDY_TEXTURE_REFLECTION_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), TIDY_TYPE_TEXTURE_REFLECTION, TidyTextureReflectionClass))
#define TIDY_IS_TEXTURE_REFLECTION_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), TIDY_TYPE_TEXTURE_REFLECTION))
#define TIDY_TEXTURE_REFLECTION_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), TIDY_TYPE_TEXTURE_REFLECTION, TidyTextureReflectionClass))

typedef struct _TidyTextureReflection           TidyTextureReflection;
typedef struct _TidyTextureReflectionPrivate    TidyTextureReflectionPrivate;
typedef struct _TidyTextureReflectionClass      TidyTextureReflectionClass;

struct _TidyTextureReflection
{
  ClutterClone parent_instance;

  TidyTextureReflectionPrivate *priv;
};

struct _TidyTextureReflectionClass
{
  ClutterCloneClass parent_class;
};

GType         tidy_texture_reflection_get_type              (void) G_GNUC_CONST;
ClutterActor *tidy_texture_reflection_new                   (ClutterTexture        *parent_texture);
void          tidy_texture_reflection_set_reflection_height (TidyTextureReflection *texture,
                                                             gint                   height);
gint          tidy_texture_reflection_get_reflection_height (TidyTextureReflection *texture);

G_END_DECLS

#endif /* __TIDY_TEXTURE_REFLECTION_H__ */
