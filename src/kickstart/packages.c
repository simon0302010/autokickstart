// Source - https://stackoverflow.com/a/77619798
// Posted by Holger
// Retrieved 2026-02-18, License - CC BY-SA 4.0

  /*
   * The sole purpose of this program is to test some approaches to using
   * ListView with GTK4, C, and VFL.It may contain errors and does not
   * claim to be considered "best practice".
   */

#include <gtk/gtk.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "../globals.h"
#include "../utils/utils.h"
#include "gio/gio.h"
#include "glib-object.h"

GtkWidget *window;

static void create_model() {
    g_list_store_append(options.packages.packages, gtk_string_object_new("Test 1"));
    g_list_store_append(options.packages.packages, gtk_string_object_new("Test 2"));
    g_list_store_append(options.packages.packages, gtk_string_object_new("Test 3"));
    g_list_store_append(options.packages.packages, gtk_string_object_new("Test 4"));
    g_list_store_append(options.packages.packages, gtk_string_object_new("Test 5"));
}

/*
 * Create a class derived from Widget that contains the controls 
 * and implements the Layout Manager.
 */

#define LISTVFL_TYPE_LAYOUT (listvfl_layout_get_type())

G_DECLARE_FINAL_TYPE (ListvflLayout, listvfl_layout, LISTVFL, LAYOUT, 
GtkWidget)

struct _ListvflLayout
{
    GtkWidget parent_instance;
 
    GtkWidget *main_grid;
    GtkWidget *scrolledwindow;
    GtkWidget *listview;
    GtkWidget *label;
    GtkWidget *btndelete;
    GtkWidget *btnadd;
    GtkWidget *entry;
    GtkSingleSelection *selection;
    guint position;
  };

G_DEFINE_TYPE (ListvflLayout, listvfl_layout, GTK_TYPE_WIDGET)

static bool pkg_exists(const char *pkg) {
    unsigned long command_len = strlen("dnf info ") + strlen(pkg) + 1;
    char *command = malloc(command_len);
    if (!command) return false;
    strcpy(command, "dnf info ");
    strcat(command, pkg);
    bool exists = (system(command) == 0);
    free(command);
    return exists;
}

static void setup_list_item_cb (GtkListItemFactory *factory, GtkListItem *list_item) {
    GtkWidget*label = gtk_label_new (NULL);
    gtk_list_item_set_child (GTK_LIST_ITEM(list_item), label);
}

static void bind_list_item_cb (GtkListItemFactory *factory, GtkListItem *list_item,  gpointer listvfllayout) {
    GtkWidget *label = gtk_list_item_get_child(list_item);
    GtkStringObject *str = gtk_list_item_get_item(list_item);
    
    if (!GTK_IS_LABEL(label) || !GTK_IS_STRING_OBJECT(str))
        return;

    const char *string = gtk_string_object_get_string(str);
    gtk_label_set_text(GTK_LABEL(label), string);

    if (gtk_list_item_get_selected(list_item))
    {
        ListvflLayout  *widget = (ListvflLayout*)listvfllayout;
        GtkWidget *label1 = GTK_WIDGET(widget->label);
        gtk_label_set_text(GTK_LABEL(label1), string);
        widget->position = gtk_list_item_get_position(list_item);
    }
}

static void selection_changed(GObject *object, GParamSpec *pspec, GtkWidget *listvfllayout) {
    GtkListItem *list_item=gtk_single_selection_get_selected_item(GTK_SINGLE_SELECTION(object));
    if (list_item == NULL) return;
    
    guint pos = gtk_single_selection_get_selected(GTK_SINGLE_SELECTION(object));
    const char *string = gtk_string_object_get_string(GTK_STRING_OBJECT(list_item));
    ListvflLayout  *widget = (ListvflLayout*)listvfllayout;
    gtk_label_set_label(GTK_LABEL(widget->label),string);
}

static void delete(GtkWidget *btndelete, gpointer listvfllayout) {
    ListvflLayout *widget = LISTVFL_LAYOUT(listvfllayout);
    GListModel *store = gtk_single_selection_get_model(widget->selection);
    guint pos = gtk_single_selection_get_selected(GTK_SINGLE_SELECTION(widget->selection));
    
    if (pos != GTK_INVALID_LIST_POSITION && g_list_model_get_n_items(store) > 0) {
        g_list_store_remove(G_LIST_STORE(store), pos);
        gtk_label_set_label(GTK_LABEL(widget->label), "No package selected");
    }
}

static void add(GtkWidget *btnadd, gpointer listvfllayout) {
    ListvflLayout *widget = LISTVFL_LAYOUT(listvfllayout);
    GtkEntryBuffer *buffer;
    const char *text;
    buffer = gtk_entry_get_buffer(GTK_ENTRY(widget->entry));
    text = gtk_entry_buffer_get_text(buffer);
    if (strlen(text) > 0) {
        if (!is_fedora() || pkg_exists(text) || text[0] == '@') {
            GListModel *store = gtk_single_selection_get_model(widget->selection);
            g_list_store_append(G_LIST_STORE(store), gtk_string_object_new(text));
        } else {
            const char *warning_prefix = "Could not locate package: ";
            char *warning_msg = malloc(strlen(warning_prefix) + strlen(text) + 1);
            strcpy(warning_msg, warning_prefix);
            strcat(warning_msg, text);
            GtkRoot *root = gtk_widget_get_root(GTK_WIDGET(widget));
            show_alert(GTK_WIDGET(root), warning_msg);
            free(warning_msg);
        }
    }
    gtk_entry_buffer_delete_text(buffer,0,-1);
}

static void listvfl_layout_dispose (GObject *object) {
    ListvflLayout *self = LISTVFL_LAYOUT (object);
    g_clear_pointer (&self->scrolledwindow, gtk_widget_unparent);
    g_clear_pointer (&self->label, gtk_widget_unparent);
    g_clear_pointer (&self->btndelete, gtk_widget_unparent);
    g_clear_pointer (&self->btnadd, gtk_widget_unparent);
    g_clear_pointer (&self->entry, gtk_widget_unparent);
    g_clear_pointer(&self->main_grid, gtk_widget_unparent);
    G_OBJECT_CLASS (listvfl_layout_parent_class)->dispose (object);
}

static void listvfl_layout_class_init (ListvflLayoutClass *class) {
    GObjectClass *object_class = G_OBJECT_CLASS(class);
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (class);
    object_class->dispose = listvfl_layout_dispose;
    // Layout manager
    gtk_widget_class_set_layout_manager_type (widget_class,GTK_TYPE_BIN_LAYOUT);
}

// Initializing the class
static void listvfl_layout_init (ListvflLayout *self) {
    GtkWidget *main_grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(main_grid), 8);
    gtk_grid_set_column_spacing(GTK_GRID(main_grid), 10);
    gtk_widget_set_margin_start(main_grid, 15);
    gtk_widget_set_margin_end(main_grid, 15);
    gtk_widget_set_margin_top(main_grid, 15);
    gtk_widget_set_margin_bottom(main_grid, 15);

    GtkWidget *right_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);

    GtkListItemFactory *factory;
    GListModel *model;
    self->position = 0;

    GtkWidget *widget = GTK_WIDGET (self);

    self->label = gtk_label_new(NULL);

    self->btndelete = gtk_button_new_with_label("Delete");
    gtk_widget_add_css_class(self->btndelete, "destructive-action");
    g_signal_connect(self->btndelete,"clicked",G_CALLBACK(delete),self);

    self->listview = gtk_list_view_new(NULL,NULL);        
    self->selection = gtk_single_selection_new(G_LIST_MODEL(G_LIST_MODEL (options.packages.packages)));
    gtk_single_selection_set_autoselect(self->selection, TRUE);
    gtk_list_view_set_model(GTK_LIST_VIEW(self->listview),GTK_SELECTION_MODEL(self->selection));
    g_signal_connect (self->selection,"notify::selected", G_CALLBACK(selection_changed),self);

    factory = gtk_signal_list_item_factory_new();
    g_signal_connect (factory, "setup", G_CALLBACK (setup_list_item_cb), NULL);
    g_signal_connect (factory, "bind",G_CALLBACK (bind_list_item_cb),self);
    gtk_list_view_set_factory (GTK_LIST_VIEW (self->listview),factory);

    self->scrolledwindow = gtk_scrolled_window_new();
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(self->scrolledwindow),self->listview);

    self->entry = gtk_entry_new();
    g_signal_connect(self->entry,"activate",G_CALLBACK(add),self);

    self->btnadd = gtk_button_new_with_label("Add");
    gtk_widget_add_css_class(self->btnadd, "suggested-action");
    g_signal_connect(self->btnadd,"clicked",G_CALLBACK(add),self);

    self->main_grid = main_grid;

    GtkWidget *vspacer = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

    GtkWidget *close_btn = gtk_button_new_with_label("Finish");
    g_signal_connect_swapped(close_btn, "clicked", G_CALLBACK(gtk_window_close), window);

    gtk_widget_set_vexpand(vspacer, TRUE);
    gtk_widget_set_hexpand(self->scrolledwindow, TRUE);
    gtk_widget_set_vexpand(self->scrolledwindow, TRUE);

    gtk_grid_attach(GTK_GRID(main_grid), self->label, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(main_grid), self->scrolledwindow, 0, 1, 1, 1);

    gtk_box_append(GTK_BOX(right_box), self->btndelete);
    gtk_box_append(GTK_BOX(right_box), self->entry);
    gtk_box_append(GTK_BOX(right_box), self->btnadd);
    gtk_box_append(GTK_BOX(right_box), options.packages.multilib);
    gtk_box_append(GTK_BOX(right_box), options.packages.nocore);
    gtk_box_append(GTK_BOX(right_box), vspacer);
    gtk_box_append(GTK_BOX(right_box), close_btn);

    gtk_grid_attach(GTK_GRID(main_grid), right_box, 1, 1, 1, 1);
    
    gtk_widget_set_parent(GTK_WIDGET(main_grid), GTK_WIDGET(self));
}

void open_package_management(GtkWidget *open_management_button, gpointer user_data) 
{
    GtkWidget *box, *listvfllayout;

    window = gtk_window_new();
    gtk_window_set_title (GTK_WINDOW (window), "Package Management");
    gtk_widget_set_size_request (window, 400,500);
    g_object_add_weak_pointer (G_OBJECT (window),(gpointer *)&window);
    box = gtk_box_new(GTK_ORIENTATION_VERTICAL,12);
    gtk_window_set_child(GTK_WINDOW (window), box);
    listvfllayout = g_object_new(listvfl_layout_get_type(), NULL);
    gtk_widget_set_hexpand(listvfllayout, TRUE);
    gtk_widget_set_vexpand(listvfllayout, TRUE);
    gtk_box_append (GTK_BOX (box), listvfllayout);
    gtk_window_present (GTK_WINDOW(window));
}

// sets up all widgets in options that are used in packages popup
void setup_packages_window_items() {
    options.packages.packages = g_list_store_new(GTK_TYPE_STRING_OBJECT);
    g_object_ref(options.packages.packages);

    options.packages.multilib = gtk_check_button_new_with_label("Use Multilib");
    gtk_widget_set_margin_top(options.packages.multilib, 30);
    gtk_widget_set_tooltip_text(options.packages.multilib, "Configure the installed system for multilib packages (that is, to allow installing 32-bit packages on a 64-bit system) and install packages specified in this section as such.");
    g_object_ref(options.packages.multilib);

    options.packages.nocore = gtk_check_button_new_with_label("No Core");
    gtk_widget_set_tooltip_text(options.packages.nocore, "Do not install the @Core group.");
    g_object_ref(options.packages.nocore);
}