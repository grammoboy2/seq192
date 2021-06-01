#ifndef SEQ24_PIANOROLL
#define SEQ24_PIANOROLL

#include <gtkmm.h>

#include "../core/perform.h"
#include "pianokeys.h"

using namespace Gtk;

class PianoRoll : public DrawingArea {

    public:

        PianoRoll(perform * p, sequence * seq, PianoKeys * pianokeys);
        ~PianoRoll();

    protected:

        bool on_draw(const Cairo::RefPtr<Cairo::Context>& cr);


    private:

        perform            *m_perform;
        sequence           *m_sequence;
        PianoKeys          *m_pianokeys;

        Cairo::RefPtr<Cairo::ImageSurface> m_surface;

        // zoom: ticks per pixel
        int                 m_zoom;



        bool on_motion_notify_event(GdkEventMotion* event);
        bool on_leave_notify_event(GdkEventCrossing* event);




};

#endif
