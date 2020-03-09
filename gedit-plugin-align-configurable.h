#ifndef __GEDIT_PLUGIN_ALIGN_CONFIGURABLE_H__
#define __GEDIT_PLUGIN_ALIGN_CONFIGURABLE_H__

#include <gtk/gtk.h>
#include <libpeas/peas.h>

#define GEDIT_PLUGIN_TYPE_ALIGN_CONFIGURABLE gedit_plugin_align_configurable_get_type()
G_DECLARE_FINAL_TYPE (GeditPluginAlignConfigurable, gedit_plugin_align_configurable, GEDIT_PLUGIN, ALIGN_CONFIGURABLE, PeasExtensionBase)

void gedit_plugin_align_configurable_register(GTypeModule *module);

#endif
