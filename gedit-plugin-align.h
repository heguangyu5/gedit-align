#ifndef __GEDIT_PLUGIN_ALIGN_H__
#define __GEDIT_PLUGIN_ALIGN_H__

#include <gtk/gtk.h>
#include <libpeas/peas.h>

#define GEDIT_PLUGIN_TYPE_ALIGN gedit_plugin_align_get_type()
G_DECLARE_FINAL_TYPE (GeditPluginAlign, gedit_plugin_align, GEDIT_PLUGIN, ALIGN, PeasExtensionBase)

void peas_register_types(PeasObjectModule *module);

#endif
