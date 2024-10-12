#include <gtk/gtk.h>
#include <glib.h>

GtkTextBuffer *buffer;
GtkWidget *statusbar;
gchar *current_file = NULL;
gboolean is_modified = FALSE;

// Function to update line and column numbers in the status bar
void updateStatusBar(GtkTextBuffer *buffer, GtkStatusbar *statusbar) {
    GtkTextIter iter;
    gtk_text_buffer_get_iter_at_mark(buffer, &iter, gtk_text_buffer_get_insert(buffer));

    int line = gtk_text_iter_get_line(&iter) + 1;   // Lines are zero-indexed
    int col = gtk_text_iter_get_line_offset(&iter) + 1;  // Columns are zero-indexed

    gchar *status = g_strdup_printf("Line: %d  Col: %d", line, col);
    gtk_statusbar_pop(statusbar, 0);
    gtk_statusbar_push(statusbar, 0, status);
    g_free(status);
}

// Function to mark buffer as modified
void markAsModified(GtkTextBuffer *buffer, gpointer data) {
    is_modified = TRUE;
}

// Function to save the current file
void saveFile(GtkWidget *widget) {
    if (!current_file) {
        // Use Save As functionality if no file is currently open
        saveFileAs(widget, NULL);
        return;
    }

    GtkTextIter start, end;
    gtk_text_buffer_get_start_iter(buffer, &start);
    gtk_text_buffer_get_end_iter(buffer, &end);
    gchar *content = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);
    g_file_set_contents(current_file, content, -1, NULL);
    g_free(content);
    is_modified = FALSE;
}

// Function to save the current file as
void saveFileAs(GtkWidget *widget, gpointer window) {
    GtkWidget *dialog = gtk_file_chooser_dialog_new("Save File As",
                                                    GTK_WINDOW(window),
                                                    GTK_FILE_CHOOSER_ACTION_SAVE,
                                                    "_Cancel", GTK_RESPONSE_CANCEL,
                                                    "_Save", GTK_RESPONSE_ACCEPT,
                                                    NULL);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        current_file = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));

        GtkTextIter start, end;
        gtk_text_buffer_get_start_iter(buffer, &start);
        gtk_text_buffer_get_end_iter(buffer, &end);
        gchar *content = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);
        g_file_set_contents(current_file, content, -1, NULL);
        g_free(content);
        is_modified = FALSE;
    }

    gtk_widget_destroy(dialog);
}

// Function to import a file
void importFile(GtkWidget *widget, gpointer window) {
    GtkWidget *dialog = gtk_file_chooser_dialog_new("Open File",
                                                    GTK_WINDOW(window),
                                                    GTK_FILE_CHOOSER_ACTION_OPEN,
                                                    "_Cancel", GTK_RESPONSE_CANCEL,
                                                    "_Open", GTK_RESPONSE_ACCEPT,
                                                    NULL);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        current_file = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));

        gchar *content;
        g_file_get_contents(current_file, &content, NULL, NULL);
        gtk_text_buffer_set_text(buffer, content, -1);
        g_free(content);
        is_modified = FALSE;
    }

    gtk_widget_destroy(dialog);
}

// Function to update word wrap settings
void toggleWordWrap(GtkCheckMenuItem *checkmenuitem, GtkTextView *textview) {
    gboolean is_active = gtk_check_menu_item_get_active(checkmenuitem);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(textview), is_active ? GTK_WRAP_WORD : GTK_WRAP_NONE);
}

// Function to display the about dialog
void showAboutDialog(GtkWidget *widget, gpointer window) {
    GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window),
                                               GTK_DIALOG_DESTROY_WITH_PARENT,
                                               GTK_MESSAGE_INFO,
                                               GTK_BUTTONS_OK,
                                               "Created by anonp4x, a free and open source software developer.");
    gtk_window_set_title(GTK_WINDOW(dialog), "About");
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

// Function to display help information
void showHelpDialog(GtkWidget *widget, gpointer window) {
    GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window),
                                               GTK_DIALOG_DESTROY_WITH_PARENT,
                                               GTK_MESSAGE_INFO,
                                               GTK_BUTTONS_OK,
                                               "This is a simple text editor. Use File menu to import, save, or save as.\nUse Settings to toggle word wrap.");
    gtk_window_set_title(GTK_WINDOW(dialog), "Help");
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

// Function to prompt save before closing
gboolean onDeleteEvent(GtkWidget *widget, GdkEvent *event, gpointer window) {
    if (!is_modified) {
        return FALSE;  // Allow window to close if no changes
    }

    GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window),
                                               GTK_DIALOG_MODAL,
                                               GTK_MESSAGE_WARNING,
                                               GTK_BUTTONS_NONE,
                                               "You have unsaved changes. Do you want to save before quitting?");
    gtk_dialog_add_buttons(GTK_DIALOG(dialog),
                           "_Cancel", GTK_RESPONSE_CANCEL,
                           "_Discard", GTK_RESPONSE_REJECT,
                           "_Save", GTK_RESPONSE_ACCEPT,
                           NULL);

    gint response = gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);

    if (response == GTK_RESPONSE_ACCEPT) {
        saveFile(NULL);
        return FALSE;  // Proceed with closing
    } else if (response == GTK_RESPONSE_CANCEL) {
        return TRUE;  // Cancel closing
    }

    return FALSE;  // Discard changes and close
}

// Function to create the text editor window
void createTextEditorWindow() {
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Text Editor");
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    // Create menu bar
    GtkWidget *menubar = gtk_menu_bar_new();
    gtk_box_pack_start(GTK_BOX(vbox), menubar, FALSE, FALSE, 0);

    // File menu
    GtkWidget *fileMenu = gtk_menu_new();
    GtkWidget *fileItem = gtk_menu_item_new_with_label("File");
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(fileItem), fileMenu);
    gtk_menu_shell_append(GTK_MENU_SHELL(menubar), fileItem);

    GtkWidget *importItem = gtk_menu_item_new_with_label("Import");
    g_signal_connect(importItem, "activate", G_CALLBACK(importFile), window);
    gtk_menu_shell_append(GTK_MENU_SHELL(fileMenu), importItem);

    GtkWidget *saveItem = gtk_menu_item_new_with_label("Save");
    g_signal_connect(saveItem, "activate", G_CALLBACK(saveFile), NULL);
    gtk_menu_shell_append(GTK_MENU_SHELL(fileMenu), saveItem);

    GtkWidget *saveAsItem = gtk_menu_item_new_with_label("Save As");
    g_signal_connect(saveAsItem, "activate", G_CALLBACK(saveFileAs), window);
    gtk_menu_shell_append(GTK_MENU_SHELL(fileMenu), saveAsItem);

    // Settings menu
    GtkWidget *settingsMenu = gtk_menu_new();
    GtkWidget *settingsItem = gtk_menu_item_new_with_label("Settings");
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(settingsItem), settingsMenu);
    gtk_menu_shell_append(GTK_MENU_SHELL(menubar), settingsItem);

    GtkWidget *wordWrapItem = gtk_check_menu_item_new_with_label("Word Wrap");
    g_signal_connect(wordWrapItem, "toggled", G_CALLBACK(toggleWordWrap), NULL);
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(wordWrapItem), TRUE); // Enable word wrap by default
    gtk_menu_shell_append(GTK_MENU_SHELL(settingsMenu), wordWrapItem);

    // Help menu
    GtkWidget *helpMenu = gtk_menu_new();
    GtkWidget *helpItem = gtk_menu_item_new_with_label("Help");
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(helpItem), helpMenu);
    gtk_menu_shell_append(GTK_MENU_SHELL(menubar), helpItem);

    GtkWidget *helpInfoItem = gtk_menu_item_new_with_label("Help");
    g_signal_connect(helpInfoItem, "activate", G_CALLBACK(showHelpDialog), window);
    gtk_menu_shell_append(GTK_MENU_SHELL(helpMenu), helpInfoItem);

    // About menu
    GtkWidget *aboutItem = gtk_menu_item_new_with_label("About");
    g_signal_connect(aboutItem, "activate", G_CALLBACK(showAboutDialog), window);
    gtk_menu_shell_append(GTK_MENU_SHELL(menubar), aboutItem);

    // Create text view and buffer with scrolling
    buffer = gtk_text_buffer_new(NULL);
    GtkWidget *textView = gtk_text_view_new_with_buffer(buffer);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(textView), GTK_WRAP_WORD);

    // Create a scrolled window and add the text view to it
    GtkWidget *scrolledWindow = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolledWindow), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(scrolledWindow), textView);
    gtk_box_pack_start(GTK_BOX(vbox), scrolledWindow, TRUE, TRUE, 0);

    // Add status bar for line/column display
    statusbar = gtk_statusbar_new();
    gtk_box_pack_start(GTK_BOX(vbox), statusbar, FALSE, FALSE, 0);

    // Update status bar on buffer changes
    g_signal_connect(buffer, "changed", G_CALLBACK(updateStatusBar), statusbar);
    g_signal_connect(buffer, "mark-set", G_CALLBACK(updateStatusBar), statusbar);

    // Mark buffer as modified on changes
    g_signal_connect(buffer, "changed", G_CALLBACK(markAsModified), NULL);

    // Connect the delete event for the window close confirmation dialog
    g_signal_connect(window, "delete-event", G_CALLBACK(onDeleteEvent), window);

    gtk_widget_show_all(window);
}

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);
    createTextEditorWindow();
    gtk_main();
    return 0;
}
