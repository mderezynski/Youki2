#include <config.h>
#include "mpx/com/tagview.hh"

#include <gtkmm.h>
#include <cairomm/cairomm.h>
#include <boost/shared_ptr.hpp>
#ifdef HAVE_TR1
#include <tr1/cmath>
#else
#include <math.h>
#endif
#include "mpx/util-graphics.hh"

using namespace Glib;

namespace MPX
{
        void
        TagView::update_global_extents ()
        {
            m_Layout.RowHeight = 0;
            for(LayoutList::const_iterator i = m_Layout.List.begin(); i != m_Layout.List.end(); ++i)
            {
                if((*i)->m_Logical.get_height() > m_Layout.RowHeight)    
                {
                    m_Layout.RowHeight = (*i)->m_Logical.get_height();
                }
            }
        }

        inline void
        TagView::push_back_row (LayoutList const& row, int x)
        {
            m_Layout.Rows.push_back(row);
            m_Layout.Widths.push_back(x - (TAG_SPACING / m_Layout.Scale));
        }

        void
        TagView::layout ()
        {
            m_Layout.Scale = 1.;

            retry_pack:

            m_Layout.Rows.clear();
            m_Layout.Widths.clear();

            if(m_Layout.List.empty())
                return;

            if(get_allocation().get_height() < m_Layout.RowHeight)
                return;
    
            LayoutList row;
            double x = 0;
            int mw = 0;            
            for(LayoutList::const_iterator i = m_Layout.List.begin(); i != m_Layout.List.end(); ++i)
            {
                LayoutSP sp = *i;

		#ifdef HAVE_TR1
                if((x+(std::tr1::round(sp->m_Logical.get_width()-0.5))) > ((get_allocation().get_width()+0.5) / m_Layout.Scale)) 
		#else
                if((x+(round(sp->m_Logical.get_width()-0.5))) > ((get_allocation().get_width()+0.5) / m_Layout.Scale)) 
		#endif
                {
                    if((x - (TAG_SPACING / m_Layout.Scale)) > mw)
                    {
                        mw = x - (TAG_SPACING / m_Layout.Scale);
                    }
                    push_back_row (row, x);
                    row = LayoutList();
                    x = 0;
                }
   
                x += sp->m_Logical.get_width() + (TAG_SPACING / m_Layout.Scale);
                row.push_back(sp);
            }

            push_back_row (row, x);

            if((m_Layout.Rows.size() * m_Layout.RowHeight) > (get_allocation().get_height() / m_Layout.Scale)) 
            {
                if(m_Layout.Scale >= ACCEPTABLE_MIN_SCALE)
                {
                    m_Layout.Scale -= SCALE_STEP;
                    goto retry_pack;
                }
            }
#if 0
            else if((m_Layout.Rows.size() * m_Layout.RowHeight) <= (get_allocation().get_height() / m_Layout.Scale)) 
            {
                double diff = (get_allocation().get_height() / m_Layout.Scale) - (m_Layout.Rows.size() * m_Layout.RowHeight);
                if(diff > (2*m_Layout.RowHeight)) // arbitrary?
                {
                    m_Layout.Scale += SCALE_STEP*2.;
                    goto retry_pack;
                }
            }
#endif
        
            if(mw > (get_allocation().get_width() / m_Layout.Scale)) 
            {
                if(m_Layout.Scale >= ACCEPTABLE_MIN_SCALE)
                {
                    m_Layout.Scale -= SCALE_STEP;
                    goto retry_pack;
                }
            }

            double heightcorrection = (((get_allocation().get_height() - (m_Layout.Rows.size() * (m_Layout.RowHeight * m_Layout.Scale)))) / m_Layout.Scale) / 2.;
            double height_for_row = (heightcorrection * 2.) / (m_Layout.Rows.size() - 1);
            double ry = 0;

            int rowcounter = 0;

            WidthsT::const_iterator wi = m_Layout.Widths.begin(); 

            for(RowListT::iterator i = m_Layout.Rows.begin(); i != m_Layout.Rows.end(); ++i)
            {
                LayoutList & l = *i;

                double rx = (get_allocation().get_width() / 2.) - (((*wi) * m_Layout.Scale) / 2.); // center row
                //double spacing = (get_allocation().get_width() - ((*wi) * m_Layout.Scale)) / (l.size() - 1);
		#ifdef HAVE_TR1
                rx = std::tr1::fmax(0, rx);
		#else
                rx = fmax(0, rx);
		#endif
                rx /= m_Layout.Scale;
                for(LayoutList::const_iterator r = l.begin(); r != l.end(); ++r) 
                {
                    LayoutSP sp = *r;
                    sp->x = rx;

                    //if(m_Layout.Scale < 1.0)
                    //    sp->y = ry + height_for_row*rowcounter + ((m_Layout.RowHeight - sp->m_Logical.get_height())/2.); 
                    //else
                        sp->y = ry + heightcorrection + ((m_Layout.RowHeight - sp->m_Logical.get_height())/2.); 

                    rx += sp->m_Logical.get_width() + (TAG_SPACING / m_Layout.Scale); 
                }

                ry += m_Layout.RowHeight;
                wi++;
                rowcounter++;
            }
        }

        bool
        TagView::on_button_press_event (GdkEventButton * event)
        {
            g_signal_emit (G_OBJECT(gobj()), m_GSignalTagClicked, 0, m_ActiveTagName.c_str());
            return false;
        }

        bool
        TagView::on_leave_notify_event (GdkEventCrossing * event)
        {
            motion_x = -1;
            motion_y = -1;
            queue_draw ();

            return false;
        }

        bool
        TagView::on_motion_notify_event (GdkEventMotion * event)
        {
            GdkModifierType state;
            int x, y;

            if (event->is_hint)
            {
              gdk_window_get_pointer (event->window, &x, &y, &state);
            }
            else
            {
              x = int (event->x);
              y = int (event->y);
              state = GdkModifierType (event->state);
            }

            motion_x = x;
            motion_y = y;

            m_ActiveTagName = std::string();
            m_ActiveRow = -1;
            int rowcounter = 0;
            for(RowListT::const_iterator i = m_Layout.Rows.begin(); i != m_Layout.Rows.end(); ++i)
            {
                LayoutList const& l = *i;
                for(LayoutList::const_iterator r = l.begin(); r != l.end(); ++r) 
                {
                    LayoutSP sp = *r;

                    if(((motion_x / m_Layout.Scale) >= sp->x) && ((motion_y / m_Layout.Scale) >= sp->y) && ((motion_x / m_Layout.Scale) < (sp->x + sp->m_Logical.get_width())) && ((motion_y / m_Layout.Scale) < (sp->y + sp->m_Logical.get_height())))
                    {
                        sp->active = true;
                        m_ActiveTagName = sp->m_Text;
                        m_ActiveRow = rowcounter;
                    }
                    else
                    {
                        sp->active = false;
                    }
                }
                rowcounter++;
            }

            queue_draw ();

            return false;
        }

        bool
        TagView::on_configure_event (GdkEventConfigure * event)
        {
            update_global_extents ();
            layout ();
            queue_draw ();
    
            return false;
        }

        bool
        TagView::on_expose_event (GdkEventExpose * event)
        {
            using namespace Gtk;

            Cairo::RefPtr<Cairo::Context> cr = get_window()->create_cairo_context();

            int x, y, w, h;
            Gtk::Allocation a = get_allocation();
            x = a.get_x();
            y = a.get_y();
            w = a.get_width();
            h = a.get_height();

            cr->set_operator(Cairo::OPERATOR_SOURCE);
            cr->set_source_rgba(0., 0., 0., 1.);
            cr->rectangle(0, 0, w, h);
            cr->fill();

            if( !m_Display )
                return false;

            cr->scale( m_Layout.Scale, m_Layout.Scale );

            if(m_Layout.List.empty())
                return true;

            int rowcounter = 0;
            int tagcounter = 0;
            for(RowListT::const_iterator i = m_Layout.Rows.begin(); i != m_Layout.Rows.end(); ++i)
            {
                LayoutList const& l = *i;
                for(LayoutList::const_iterator r = l.begin(); r != l.end(); ++r) 
                {
                    LayoutSP sp = *r;

                    try{
                        if(sp->active)
                            cr->set_source_rgb(1., 0., 0.);
                        else
                        {
                            if((tagcounter % 2) == 0)
                                cr->set_source_rgb(0.22, 0.66, 0.36);
                            else
                                cr->set_source_rgb(0.10, 0.87, 0.30);
                        }

                        cr->move_to( sp->x , sp->y ); 
                        pango_cairo_show_layout(cr->cobj(), sp->m_Layout->gobj());
                    } catch (...) {}

                    tagcounter++;
                }
                rowcounter++;
            }

            try{
                if(!m_ActiveTagName.empty())
                {
                    cr->set_identity_matrix ();

                    double heightcorrection = (((h - (m_Layout.Rows.size() * (m_Layout.RowHeight * m_Layout.Scale)))) / m_Layout.Scale) / 2.;

                    int top_space = ((m_ActiveRow-1)*m_Layout.RowHeight) + heightcorrection;
                    int bottom_space = ( h - (m_ActiveRow*m_Layout.RowHeight+heightcorrection));
                    
                    int y = 0;    

                    Glib::RefPtr<Pango::Layout> Layout = create_pango_layout(Markup::escape_text(m_ActiveTagName));
                    Pango::AttrList list;
                    Pango::Attribute attr1 = Pango::Attribute::create_attr_size(12000);
                    Pango::Attribute attr2 = Pango::Attribute::create_attr_weight(Pango::WEIGHT_BOLD);
                    list.insert(attr1);
                    list.insert(attr2);
                    Layout->set_attributes(list);

                    Pango::Rectangle Ink, Logical;
                    Layout->get_pixel_extents(Ink, Logical);

                    int x = (w/2 - Logical.get_width()/2) - 12;

                    if(top_space > bottom_space)
                        y = (h/4) - (Logical.get_height()/2); 
                    else
                        y = h - (h/4) - (Logical.get_height()/2); 

                    Util::cairo_rounded_rect(cr, x, y, Logical.get_width() + 24, Logical.get_height()+12, 6.);                    
                    cr->set_operator(Cairo::OPERATOR_ATOP);
                    cr->set_source_rgba(0.65, 0.65, 0.65, 0.9);
                    cr->fill();

                    cr->move_to(x + 12, y + 6);
                    cr->set_source_rgba(1., 1., 1., 1.); 
                    pango_cairo_show_layout(cr->cobj(), Layout->gobj());
                }
            } catch (...) {}

            return true;
        }

        std::string const&
        TagView::get_active_tag () const
        {
            return m_ActiveTagName;
        }

        void
        TagView::clear ()
        {
            m_Layout.reset();
            queue_draw ();
        }

        void
        TagView::display(bool do_display)
        {
            m_Display = do_display;

            if( m_Display )
            {
                update_global_extents ();
                layout ();
            }

            queue_draw ();
        }

        void
        TagView::add_tag (std::string const& text, double amplitude)
        {
            LayoutSP sp (new Layout);                        
            sp->m_Text = text;
            sp->amplitude = amplitude;
            sp->m_Layout = create_pango_layout(text);

            Pango::AttrList list;
            Pango::Attribute attr1 = Pango::Attribute::create_attr_scale(amplitude);
            list.insert(attr1);
            sp->m_Layout->set_attributes(list);

            sp->m_Layout->get_pixel_extents(sp->m_Ink, sp->m_Logical);
            m_Layout.List.push_back(sp);

            if( m_Display )
            {
                update_global_extents ();
                layout ();
                queue_draw ();
            }
        }

        TagView::TagView ()
        : motion_x(0)
        , motion_y(0)
        , m_Display(true)
        {
            if(!gsignals_initialized)
            {
                m_GSignalTagClicked = 
                    g_signal_new ("tag-clicked",
                              G_OBJECT_CLASS_TYPE (G_OBJECT_CLASS (G_OBJECT_GET_CLASS(G_OBJECT(gobj())))),
                              GSignalFlags (G_SIGNAL_RUN_LAST | G_SIGNAL_DETAILED),
                              0,
                              NULL, NULL,
                              g_cclosure_marshal_VOID__STRING, G_TYPE_NONE, 1, G_TYPE_STRING); 
                gsignals_initialized = true;
            }
            else
            {
                m_GSignalTagClicked = g_signal_lookup("tag-clicked", get_type() );
            }

            gtk_widget_add_events(GTK_WIDGET(gobj()), Gdk::POINTER_MOTION_MASK | Gdk::BUTTON_PRESS_MASK | Gdk::LEAVE_NOTIFY_MASK);
        }

        TagView::~TagView ()
        {
        }

        double TagView::TAG_SPACING = 12;
        double TagView::ACCEPTABLE_MIN_SCALE = 0.0;
        double TagView::SCALE_STEP = 0.02;
        bool TagView::gsignals_initialized = false;
}
