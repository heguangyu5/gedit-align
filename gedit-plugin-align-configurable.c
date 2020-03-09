#include "gedit-plugin-align-configurable.h"
#include <libpeas-gtk/peas-gtk.h>
#include <gtksourceview/gtksource.h>

static void peas_gtk_configurable_iface_init(PeasGtkConfigurableInterface *iface);

struct _GeditPluginAlignConfigurable {
    PeasExtensionBase parent;
};

G_DEFINE_DYNAMIC_TYPE_EXTENDED (GeditPluginAlignConfigurable,
                                gedit_plugin_align_configurable,
                                PEAS_TYPE_EXTENSION_BASE,
                                0,
                                G_IMPLEMENT_INTERFACE_DYNAMIC (PEAS_GTK_TYPE_CONFIGURABLE,
                                                               peas_gtk_configurable_iface_init))

static void gedit_plugin_align_configurable_class_init(GeditPluginAlignConfigurableClass *class)
{
    //g_print("%s\n", G_STRFUNC);
}

static void gedit_plugin_align_configurable_init(GeditPluginAlignConfigurable *plugin)
{
    //g_print("%s\n", G_STRFUNC);
}

static void gedit_plugin_align_configurable_class_finalize(GeditPluginAlignConfigurableClass *class)
{
    //g_print("%s\n", G_STRFUNC);
}

static void gedit_plugin_align_configurable_close(GtkTextView *view, GtkTextBuffer *buffer)
{
    GtkTextIter start, end;
    gtk_text_buffer_get_bounds(buffer, &start, &end);
    gchar *text = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);
    gchar **lines = g_strsplit(text, "\n", -1);
    g_free(text);

    GPtrArray *separators = g_ptr_array_new();
    gchar **l = lines;
    while (*l) {
        if (**l != 0) {
            g_ptr_array_add(separators, *l);
        }
        l++;
    }
    g_ptr_array_add(separators, NULL);

/*
    gchar *tmp = g_strjoinv("\n", (gchar**)separators->pdata);
    g_print("final separators: @%s@\n", tmp);
    g_free(tmp);
*/
    GSettings *settings = g_object_get_data(G_OBJECT(buffer), "gsettings");
    g_settings_set_strv(settings, "separators", (const gchar **)separators->pdata);
    g_settings_sync();

    g_object_unref(settings);
    g_ptr_array_free(separators, TRUE);
    g_strfreev(lines);
}

static GtkWidget *gedit_plugin_align_configurable_create_configure_widget(PeasGtkConfigurable *configurable)
{
    //g_print("%s\n", G_STRFUNC);

    GtkWidget *view =  g_object_new(GTK_SOURCE_TYPE_VIEW,
                                    "width-request", 200,
                                    "height-request", 150,
                                    "monospace", TRUE,
                                    "highlight-current-line", TRUE,
                                    NULL);
    GtkSourceSpaceDrawer *drawer = gtk_source_view_get_space_drawer(GTK_SOURCE_VIEW(view));
    gtk_source_space_drawer_set_types_for_locations(drawer,
                                                    GTK_SOURCE_SPACE_LOCATION_ALL,
                                                    GTK_SOURCE_SPACE_TYPE_ALL & ~GTK_SOURCE_SPACE_TYPE_NEWLINE);
    gtk_source_space_drawer_set_enable_matrix(drawer, TRUE);

    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(view));
    GSettings *settings = g_settings_new("org.gnome.gedit.plugins.align");
    gchar **separators = g_settings_get_strv(settings, "separators");
    gchar *text = g_strjoinv("\n", separators);
    g_strfreev(separators);
    gtk_text_buffer_set_text(GTK_TEXT_BUFFER(buffer), text, -1);
    g_free(text);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    GtkWidget *label  = gtk_label_new("Separators");
    gtk_label_set_xalign(GTK_LABEL(label), 0.0);
    gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), view, TRUE, TRUE, 0);

    g_object_set_data(G_OBJECT(buffer), "gsettings", settings);
    g_signal_connect(view, "destroy", G_CALLBACK(gedit_plugin_align_configurable_close), buffer);

    return vbox;
}

static void peas_gtk_configurable_iface_init(PeasGtkConfigurableInterface *iface)
{
    //g_print("%s\n", G_STRFUNC);

    iface->create_configure_widget = gedit_plugin_align_configurable_create_configure_widget;
}

void gedit_plugin_align_configurable_register(GTypeModule *module)
{
    //g_print("%s\n", G_STRFUNC);

    gedit_plugin_align_configurable_register_type(module);
}
