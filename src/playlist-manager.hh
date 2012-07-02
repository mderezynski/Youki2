#ifndef MPX_PLAYLIST_MANAGER__HH
#define MPX_PLAYLIST_MANAGER__HH

#include <vector>
#include <string>
#include <set>
#include <glibmm.h>
#include <gtkmm.h>
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/ref.hpp>
#include "mpx/util-graphics.hh"
#include "mpx/widgets/cell-renderer-cairo-surface.hh"

namespace MPX
{
	    typedef std::vector<guint>		IntVec_t ;
	    typedef std::set<std::string>	StrSet_t ;

	    struct ArtistsExcerpt_t
	    {
		StrSet_t	Artists ;
		std::string	ArtistsJoined ;

		void
		update()
		{
		    ArtistsJoined = boost::algorithm::join( Artists, ", " ) ;
		}

		const std::string&
		get()
		{
		    return ArtistsJoined ;
		}
	    } ;

	    struct Playlist_t
	    {
		IntVec_t			    Playlist ;
		std::string			    Name ;
		ArtistsExcerpt_t		    Excerpt ;
		Glib::RefPtr<Gdk::Pixbuf>	    Icon ;
	    } ;

	    typedef boost::shared_ptr<Playlist_t>   Playlist_sp ;
	    typedef std::vector<Playlist_sp>	    Playlist_List_t ; 

	    class PlaylistManager
	    {
		protected: 

		    Playlist_List_t	m_Playlists ;

		    std::string		m_Playlists_Base_Dir ;

		    void
		    generate_artist_excerpt(Playlist_t&) ;

		    void
		    generate_icon(Playlist_t&) ;

		public:

		    PlaylistManager() ;

		    PlaylistManager(const PlaylistManager&) = delete ;

		    virtual ~PlaylistManager(){}

		    const Playlist_List_t&
		    get()
		    {
			return m_Playlists ;
		    }

		    void
		    load_playlists() ;
	    } ;

	    class PlaylistGUI : public Gtk::IconView
	    {
		typedef sigc::signal<void, const Playlist_sp&> SignalPlaylistSelected ;
		typedef sigc::signal<void> SignalPlaylistNoSelected ;

		protected:

		    struct PlaylistColumns : public Gtk::TreeModel::ColumnRecord
		    {
			Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf>>	    Icon ;
			Gtk::TreeModelColumn<std::string>		    Name ;
			Gtk::TreeModelColumn<Playlist_sp>		    Playlist ;

			PlaylistColumns()
			{
			    add(Icon);
			    add(Name);
			    add(Playlist);
			}
		    } ;

		    PlaylistColumns		    Columns ;
		    Glib::RefPtr<Gtk::ListStore>    Store ;

		    void
		    handle_selection_changed()
		    {
			std::vector<Gtk::TreePath> paths = get_selected_items() ;
		
			if(!paths.empty()) 	
			{
			    Gtk::TreeIter iter = Store->get_iter(paths[0]) ; 
			    Playlist_sp p_sp = (*iter)[Columns.Playlist] ;
			    Signal1.emit(boost::ref(p_sp)) ;
			}
			else
			{
			    Signal2.emit() ;
			}
		    }

		    SignalPlaylistSelected	Signal1 ;
		    SignalPlaylistNoSelected	Signal2 ;

		    sigc::connection		ConnSelectionChanged ;

		    void
		    connect_selection()
		    {
			ConnSelectionChanged = signal_selection_changed().connect(
			      sigc::mem_fun( *this, &PlaylistGUI::handle_selection_changed)
			) ;
		    }

		public:

		    SignalPlaylistSelected&
		    signal_playlist_selected()
		    {
			return Signal1 ;
		    }

		    SignalPlaylistNoSelected&
		    signal_playlist_no_selected()
		    {
			return Signal2 ;
		    }

		    PlaylistGUI(
		    )
		    {
			Store = Gtk::ListStore::create(Columns) ;

			set_model(Store) ;
   
			set_text_column(Columns.Name) ; 
			set_pixbuf_column(Columns.Icon) ; 
			
			set_selection_mode(Gtk::SELECTION_SINGLE ) ;
			set_item_width(286) ;

			connect_selection() ;

			override_background_color(Util::make_rgba(1,1,1)) ;
		    }

		    virtual
		    ~PlaylistGUI(){}

		    void
		    clear_selection()		    
		    {
			ConnSelectionChanged.block() ;
			unselect_all() ;
			ConnSelectionChanged.unblock() ;
		    }

		    void
		    load_playlists(
			  const Playlist_List_t& list
		    )
		    {
			Store->clear() ;

			for(auto sp : list)
			{
			    Gtk::TreeIter iter = Store->append() ;

			    (*iter)[Columns.Icon] = sp->Icon ; 
			    (*iter)[Columns.Name] = sp->Name ; 
			    (*iter)[Columns.Playlist] = sp ; 
			}
		    }
	    } ;
}

#endif
