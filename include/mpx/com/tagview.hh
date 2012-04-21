#ifndef MPX_TAGVIEW_HH
#define MPX_TAGVIEW_HH

#include <gtkmm.h>
#include <cairomm/cairomm.h>
#include <boost/shared_ptr.hpp>
#include "mpx/widgets/widgetloader.hh"
#include <libglademm/xml.h>

namespace MPX
{
class TagView : public Gtk::DrawingArea
{
        static bool gsignals_initialized; // We need this because we have no class_init in gtkmm

        struct CachedLayout
        {
        };

        static double TAG_SPACING; 
        static double ACCEPTABLE_MIN_SCALE;
        static double SCALE_STEP; 

        struct Layout
        {
            Glib::RefPtr<Pango::Layout> m_Layout; // cached
            Cairo::RefPtr<Cairo::ImageSurface> m_Cached;
            Pango::Rectangle m_Ink, m_Logical;
            double x, y;
            double amplitude;
            bool active;
            std::string m_Text;

            Layout () : active(false) {}
        };
        
        typedef boost::shared_ptr<Layout> LayoutSP;
        typedef std::vector<LayoutSP> LayoutList;
        typedef std::vector<LayoutList> RowListT;
        typedef std::vector<double> WidthsT;

        struct MainLayout
        {
            RowListT    Rows;
            WidthsT     Widths;
            double      RowHeight;
            double      Scale;
            LayoutList  List;

            void reset ()
            {
                List.clear();
                Rows.clear();
                Widths.clear();
                Scale = 1.;
            }
        };

        MainLayout m_Layout; 
        double motion_x, motion_y;
        std::string m_ActiveTagName;
        int m_ActiveRow;
        guint m_GSignalTagClicked; 
        bool m_Display;

    private:
        
        void
        update_global_extents ();

        inline void
        push_back_row (LayoutList const& row, int x);

        void
        layout ();

    protected:

        virtual bool
        on_button_press_event (GdkEventButton * event);

        virtual bool
        on_motion_notify_event (GdkEventMotion * event);

        virtual bool
        on_leave_notify_event (GdkEventCrossing * event);

        virtual bool
        on_configure_event (GdkEventConfigure * event);

        virtual bool
        on_expose_event (GdkEventExpose * event);

    public:

        TagView ();
        virtual ~TagView ();

        std::string const&
        get_active_tag () const;
            
        void
        add_tag (std::string const& text, double amplitude);

        void
        clear ();

        void
        display(bool);
};
}

#endif
