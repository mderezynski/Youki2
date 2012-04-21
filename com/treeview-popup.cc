#include "mpx/com/treeview-popup.hh"
#include "mpx/util-ui.hh"

using namespace Gtk;

MPX::TreeViewPopup::TreeViewPopup (GtkTreeView* cobj)
: TreeView(cobj)
{
    m_ui_manager = Gtk::UIManager::create();
    m_actions = Gtk::ActionGroup::create(typeid(this).name());
}

MPX::TreeViewPopup::~TreeViewPopup ()
{
}

bool
MPX::TreeViewPopup::on_event(GdkEvent * ev)
{
    if( ev->type == GDK_BUTTON_PRESS )
    {
        GdkEventButton * event = reinterpret_cast <GdkEventButton *> (ev);
        if( event->button == 3 && (event->window == get_bin_window()->gobj()))
        {
            TreeViewColumn      * column;
            int                   cell_x,
                                  cell_y;
            int                   tree_x,
                                  tree_y;
            TreePath              path;

            tree_x = event->x;
            tree_y = event->y;

            bool valid_path = get_path_at_pos (tree_x, tree_y, path, column, cell_x, cell_y);

            popup_prepare_actions (path, valid_path);
            popup_menu (path, valid_path);

            return true;
        }
    }

    return false;
}

void
MPX::TreeViewPopup::popup_menu (Gtk::TreePath const& path, bool valid_path)
{
    popup_utility_default();
}

void
MPX::TreeViewPopup::popup_utility_default ()
{
    Gtk::Menu * menu = dynamic_cast < Gtk::Menu* > (Util::get_popup (m_ui_manager, m_default_path)); 
    if (menu) // better safe than screwed
    {
        menu->popup (3, 0L); 
    }
}
