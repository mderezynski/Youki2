#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif 

#include <glibmm.h>
#include <glib/gi18n.h>
#include <gtkmm.h>

#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>
#include "mpx/widgets/widgetloader.hh"
#include "library-manager.hh"

namespace MPX
{
    namespace
    {
        const std::string ui_path = DATA_DIR G_DIR_SEPARATOR_S "ui" G_DIR_SEPARATOR_S "libman.ui";
    }

    //// LibraryManager
    LibraryManager*
        LibraryManager::create ()
    {
        return new LibraryManager(Gtk::Builder::create_from_file (ui_path));
    }

    LibraryManager::~LibraryManager ()
    {
    }

    LibraryManager::LibraryManager(
        const Glib::RefPtr<Gtk::Builder>& builder
    )
    : WidgetLoader<Gtk::Window>(builder, "libman")
    {
    }
}  // namespace MPX
