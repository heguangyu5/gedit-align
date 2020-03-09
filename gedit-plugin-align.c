#include "gedit-plugin-align.h"
#include "gedit-plugin-align-configurable.h"
#include <gedit/gedit-view.h>
#include <gedit/gedit-view-activatable.h>
#include <libpeas-gtk/peas-gtk.h>

static void gedit_view_activatable_iface_init(GeditViewActivatableInterface *iface);

struct _GeditPluginAlign {
    PeasExtensionBase parent;
    GtkWidget *view;
    gulong populate_popup_handler_id;
};

G_DEFINE_DYNAMIC_TYPE_EXTENDED (GeditPluginAlign,
                                gedit_plugin_align,
                                PEAS_TYPE_EXTENSION_BASE,
                                0,
                                G_IMPLEMENT_INTERFACE_DYNAMIC (GEDIT_TYPE_VIEW_ACTIVATABLE,
                                                               gedit_view_activatable_iface_init))

enum {
    PROP_VIEW = 1
};

typedef struct {
    gint  line_number;
    gchar **cols;
    gint  cols_len;
    gint  *cols_utf8_len;
} Line;

static void gedit_plugin_align_set_property(GObject      *object,
                                            guint        prop_id,
                                            const GValue *value,
                                            GParamSpec   *pspec)
{
    //g_print("%s\n", G_STRFUNC);

    GeditPluginAlign *plugin = GEDIT_PLUGIN_ALIGN(object);

    switch(prop_id) {
    case PROP_VIEW:
        plugin->view = g_value_dup_object(value);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
}

static void gedit_plugin_align_get_property(GObject    *object,
                                            guint      prop_id,
                                            GValue     *value,
                                            GParamSpec *pspec)
{
    //g_print("%s\n", G_STRFUNC);

    GeditPluginAlign *plugin = GEDIT_PLUGIN_ALIGN(object);

    switch(prop_id) {
    case PROP_VIEW:
        g_value_set_object(value, plugin->view);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
}

static void gedit_plugin_align_finalize(GObject *object)
{
    //g_print("%s\n", G_STRFUNC);

    GeditPluginAlign *plugin = GEDIT_PLUGIN_ALIGN(object);

    g_object_unref(plugin->view);

    G_OBJECT_CLASS(gedit_plugin_align_parent_class)->finalize(object);
}

static void gedit_plugin_align_class_init(GeditPluginAlignClass *class)
{
    //g_print("%s\n", G_STRFUNC);

    GObjectClass *object_class = G_OBJECT_CLASS(class);

    object_class->set_property = gedit_plugin_align_set_property;
    object_class->get_property = gedit_plugin_align_get_property;
    object_class->finalize     = gedit_plugin_align_finalize;

    g_object_class_override_property(object_class, PROP_VIEW, "view");
}

static void gedit_plugin_align_init(GeditPluginAlign *plugin)
{
    //g_print("%s\n", G_STRFUNC);
}

static void gedit_plugin_align_class_finalize(GeditPluginAlignClass *class)
{
    //g_print("%s\n", G_STRFUNC);
}
/*
static void print_line(Line *line)
{
    g_print("line_number = % 4d, cols_len = % 2d, cols: ", line->line_number + 1, line->cols_len);
    for (gint i = 0; i < line->cols_len; i++) {
        g_print("@%s@", line->cols[i]);
        if (line->cols_utf8_len) {
            g_print("% 2d", line->cols_utf8_len[i]);
        }
        g_print("\t");
    }
    g_print("\n");
}

static void print_lines(Line *lines, gint lines_count)
{
    for (gint i = 0; i < lines_count; i++) {
        print_line(lines + i);
    }
}

static void print_int_arr(gint *arr, gint len, gchar *desc)
{
    g_print("%s\n", desc);
    for (gint i = 0; i < len; i++) {
        g_print("%d", arr[i]);
        if (i < len - 1) {
            g_print(", ");
        }
    }
}
*/
static void free_lines(Line *lines, gint lines_count)
{
    for (gint i = 0; i < lines_count; i++) {
        Line *line = lines + i;
        if (line->cols) {
            g_strfreev(line->cols);
            g_free(line->cols_utf8_len);
        }
    }
    g_free(lines);
}

static void gedit_plugin_align_menu_item_activate(GtkMenuItem *menu_item, GeditPluginAlign *plugin)
{
    //g_print("%s\n", G_STRFUNC);

    gchar *sp = g_object_get_data(G_OBJECT(menu_item), "sp");
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(plugin->view));

    GtkTextIter start, end;
    if (!gtk_text_buffer_get_selection_bounds(buffer, &start, &end)) {
        goto out;
    }

    gchar *sp_tmp   = g_strdup(sp);
    gchar *splitter = g_strstrip(sp_tmp);
    if (splitter[0] == 0) {
        splitter = " ";
    }

    gint start_line  = gtk_text_iter_get_line(&start);
    gint end_line    = gtk_text_iter_get_line(&end);
    gint lines_count = end_line - start_line + 1;
    Line *lines      = g_new0(Line, lines_count);

    // split line to cols
    gint i;
    gint max_cols = 0;
    for (i = start_line; i <= end_line; i++) {
        Line *line = lines + (i - start_line);
        line->line_number = i;
        gtk_text_buffer_get_iter_at_line(buffer, &start, i);
        end = start;
        gtk_text_iter_forward_to_line_end(&end);
        gchar *text = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);
        if (!g_strstr_len(text, -1, splitter)) {
            g_free(text);
            continue;
        }
        line->cols = g_strsplit(text, splitter, -1);
        line->cols_len = g_strv_length(line->cols);
        if (line->cols_len > max_cols) {
            max_cols = line->cols_len;
        }
        line->cols_utf8_len = g_new0(gint, line->cols_len);
        g_strchomp(line->cols[0]);
        line->cols_utf8_len[0] = g_utf8_strlen(line->cols[0], -1);
        for (gint j = 1; j < line->cols_len; j++) {
            gchar *p = g_strstrip(line->cols[j]);
            if (p != line->cols[j]) {
                p = g_strdup(p);
                g_free(line->cols[j]);
                line->cols[j] = p;
            }
            line->cols_utf8_len[j] = g_utf8_strlen(line->cols[j], -1);
        }
    }
    //print_lines(lines, lines_count);
    // calc col max width
    gint *widths = g_new0(gint, max_cols);
    for (i = 0; i < lines_count; i++) {
        Line *line = lines + i;
        for (gint j = 0; j < line->cols_len; j++) {
            gint col_width = line->cols_utf8_len[j];
            if (col_width > widths[j]) {
                widths[j] = col_width;
            }
        }
    }
    //print_int_arr(widths, max_cols, "col max width:");

    // update buffer
    gtk_text_buffer_begin_user_action(buffer);

    for (i = 0; i < lines_count; i++) {
        Line *line = lines + i;
        if (line->cols_len == 0) {
            continue;
        }
        GString *s = g_string_new(NULL);
        for (gint j = 0; j < line->cols_len; j++) {
            g_string_append(s, line->cols[j]);
            if (j < line->cols_len - 1) {
                gint diff = widths[j] - line->cols_utf8_len[j];
                if (diff > 0) {
                    g_string_append_printf(s, "%-*s", diff, "");
                }
                g_string_append(s, sp);
            }
        }
        gtk_text_buffer_get_iter_at_line(buffer, &start, line->line_number);
        end = start;
        gtk_text_iter_forward_to_line_end(&end);
        gtk_text_buffer_delete(buffer, &start, &end);
        gtk_text_buffer_insert(buffer, &start, s->str, s->len);
        g_string_free(s, TRUE);
    }

    gtk_text_buffer_end_user_action(buffer);

out:
    g_free(sp);
    g_free(sp_tmp);
    free_lines(lines, lines_count);
    g_free(widths);
}

static void gedit_plugin_align_populate_popup(GtkTextView *text_view, GtkWidget *popup, gpointer user_data)
{
    //g_print("%s\n", G_STRFUNC);

    GtkMenuShell *menu;
    GtkWidget *menu_item;

    if (!GTK_IS_MENU_SHELL(popup)) {
        return;
    }

    //g_print("popup append align menu items\n");

    menu = GTK_MENU_SHELL(popup);

    menu_item = gtk_separator_menu_item_new();
    gtk_menu_shell_append(menu, menu_item);
    gtk_widget_show(menu_item);

    gboolean can_align = gtk_text_view_get_editable(text_view)
                         && gtk_text_buffer_get_has_selection(gtk_text_view_get_buffer(text_view));

    GSettings *settings = g_settings_new("org.gnome.gedit.plugins.align");
    gchar **separators = g_settings_get_strv(settings, "separators");
    gchar **sp = separators;
    gchar *label;
    while (*sp) {
        label = g_strdup_printf("_Align by '%s'", *sp);
        menu_item = gtk_menu_item_new_with_mnemonic(label);
        g_object_set_data(G_OBJECT(menu_item), "sp", g_strdup(*sp));
        g_signal_connect(menu_item, "activate", G_CALLBACK(gedit_plugin_align_menu_item_activate), user_data);
        gtk_menu_shell_append(menu, menu_item);
        gtk_widget_set_sensitive (menu_item, can_align);
        gtk_widget_show(menu_item);
        sp++;
        g_free(label);
    }
    g_strfreev(separators);
    g_object_unref(settings);
}

static void gedit_plugin_align_activate(GeditViewActivatable *activatable)
{
    //g_print("%s\n", G_STRFUNC);

    GeditPluginAlign *plugin = GEDIT_PLUGIN_ALIGN(activatable);

    plugin->populate_popup_handler_id = g_signal_connect(plugin->view, "populate-popup", G_CALLBACK(gedit_plugin_align_populate_popup), plugin);
}

static void gedit_plugin_align_deactivate(GeditViewActivatable *activatable)
{
    //g_print("%s\n", G_STRFUNC);

    GeditPluginAlign *plugin = GEDIT_PLUGIN_ALIGN(activatable);

    g_signal_handler_disconnect(plugin->view, plugin->populate_popup_handler_id);
}

static void gedit_view_activatable_iface_init(GeditViewActivatableInterface *iface)
{
    //g_print("%s\n", G_STRFUNC);

    iface->activate   = gedit_plugin_align_activate;
    iface->deactivate = gedit_plugin_align_deactivate;
}

void peas_register_types(PeasObjectModule *module)
{
    //g_print("%s\n", G_STRFUNC);

    gedit_plugin_align_register_type(G_TYPE_MODULE(module));
    gedit_plugin_align_configurable_register(G_TYPE_MODULE(module));

    peas_object_module_register_extension_type(module,
                                               GEDIT_TYPE_VIEW_ACTIVATABLE,
                                               GEDIT_PLUGIN_TYPE_ALIGN);
    peas_object_module_register_extension_type(module,
                                               PEAS_GTK_TYPE_CONFIGURABLE,
                                               GEDIT_PLUGIN_TYPE_ALIGN_CONFIGURABLE);
}
