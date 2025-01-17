// This file is part of seq192
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.


#ifndef SEQ192_SEQUENCE
#define SEQ192_SEQUENCE

class sequence;

#include <string>
#include <list>
#include <stack>

#include "event.h"
#include "midibus.h"
#include "globals.h"
#include "mutex.h"

enum draw_type
{
    DRAW_FIN = 0,
    DRAW_NORMAL_LINKED,
    DRAW_NOTE_ON,
    DRAW_NOTE_OFF
};

class sequence
{

  private:

    /* holds the events */
    list < event > m_list_event;
    list < event > m_list_event_draw;
    static list < event > m_list_clipboard;

    list < event > m_list_undo_hold; // seqdata

    stack < list < event > >m_list_undo;
    stack < list < event > >m_list_redo;

    /* markers */
    list < event >::iterator m_iterator_play;
    list < event >::iterator m_iterator_draw;

    /* contains the proper midi channel */
    char m_midi_channel;
    char m_bus;

    /* outputs to sequence to this Bus on midichannel */
    mastermidibus *m_masterbus;

    /* map for noteon, used when muting, to shut off current
       messages */
    int m_playing_notes[c_midi_notes];

    /* states */
    bool m_was_playing;
    bool m_playing;
    bool m_recording;
    bool m_quanized_rec;
    bool m_thru;
    bool m_queued;
    bool m_resume;
    bool m_resume_next;

    /* flag indicates that contents has changed from
       a recording */
    bool m_dirty_main;
    bool m_dirty_edit;

    /* named sequence */
    string m_name;

    /* where were we */
    long m_last_tick;
    long m_queued_tick;

    long m_starting_tick;

    /* length of sequence in pulses
       should be powers of two in bars */
    long m_length;
    long m_snap_tick;

    /* these are just for the editor to mark things
       in correct time */
    //long m_length_measures;
    long m_time_beats_per_measure;
    long m_time_beat_width;

    /* locking */
    smutex m_mutex;

    /* used to idenfity which events are ours in the out queue */
    //unsigned char m_tag;

    /* takes an event this sequence is holding and
       places it on our midibus */
    void put_event_on_bus (event * a_e);

    /* resetes the location counters */
    void reset_loop();

    void remove_all();

    /* mutex */
    void lock ();
    void unlock ();

    long adjust_offset( long a_offset );
    void remove( list<event>::iterator i );
    void remove( event* e );


  public:

    sequence ();
    ~sequence ();

    /* seqdata hold for undo */
    void set_hold_undo (bool a_hold);
    int get_hold_undo ();

    bool m_have_undo;
    bool m_have_redo;

    void push_undo (bool a_hold = false);
    void pop_undo ();
    void pop_redo ();

    //
    //  Gets and Sets
    //

    void set_have_undo();
    void set_have_redo();

    /* name */
    void set_name (string a_name);
    void set_name (char *a_name);

    void set_measures (long a_length_measures);
    long get_measures();

    void set_bpm (long a_beats_per_measure);
    long get_bpm();

    void set_bw (long a_beat_width);
    long get_bw();

    /* returns string of name */
    const char *get_name();

    /* length in ticks */
    void set_length (long a_len);
    long get_length ();

    /* returns last tick played..  used by
       editors idle function */
    long get_last_tick ();

    /* sets state.  when playing,
       and sequencer is running, notes
       get dumped to the alsa buffers */
    void set_playing (bool);
    bool get_playing ();
    void toggle_playing ();

    void toggle_queued();
    void off_queued();
    bool get_queued();
    long get_queued_tick();
    long get_times_played();
    void set_resume(bool a_resume);
    bool get_resume();

    void set_recording (bool);
    bool get_recording ();
    void set_snap_tick( int a_st );
    void get_quantized_rec( bool a_qr );
    bool get_quantized_rec( );

    void set_thru (bool);
    bool get_thru ();

    /* singals that a redraw is needed from recording */
    /* resets flag on call */
    bool is_dirty_main ();
    bool is_dirty_edit ();

    void set_dirty_main();
    void set_dirty_edit();
    void set_dirty();

    /* midi channel */
    unsigned char get_midi_channel ();
    void set_midi_channel (unsigned char a_ch);

    /* dumps contents to stdout */
    void print ();

    /* dumps notes from tick and prebuffers to
       ahead.  Called by sequencer thread - performance */
    void play (long a_tick);
    void set_orig_tick (long a_tick);

    //
    //  Selection and Manipulation
    //

    /* adds event to internal list */
    void add_event (const event * a_e);

    bool intersectNotes( long position, long position_note, long& start, long& end, long& note );
    bool intersectEvents( long posstart, long posend, long status, long& start );

    /* sets the midibus to dump to */
    void set_midi_bus (char a_mb);
    char get_midi_bus();

    void set_master_midi_bus (mastermidibus * a_mmb);

    enum select_action_e
    {
        e_select,
        e_select_one,
        e_is_selected,
        e_would_select,
        e_deselect, // deselect under cursor
        e_toggle_selection, // sel/deselect under cursor
        e_remove_one // remove one note under cursor
    };

    /* select note events in range, returns number
       selected */
    int select_note_events (long a_tick_s, int a_note_h,
			    long a_tick_f, int a_note_l, select_action_e a_action );

    /* select events in range, returns number
       selected */
    int select_events (long a_tick_s, long a_tick_f, long a_event_width,
		       unsigned char a_status, unsigned char a_cc, select_action_e a_action);

    int select_event_handle( long a_tick_s, long a_tick_f,
                        unsigned char a_status,
                        unsigned char a_cc, int a_data_s, int a_range);

    int get_num_selected_notes ();
    int get_num_selected_events (unsigned char a_status, unsigned char a_cc);

    void select_all();

    /* given a note length (in ticks) and a boolean indicating even or odd,
    select all notes where the note on even occurs exactly on an even (or odd)
    multiple of note length.
    Example use: select every note that starts on an even eighth note beat.
    */
    int select_even_or_odd_notes(int note_len, bool even);

    void copy_selected();
    void paste_selected (long a_tick, int a_note);

    /* returns the 'box' of selected items */
    void get_selected_box (long *a_tick_s, int *a_note_h,
			   long *a_tick_f, int *a_note_l);

    /* returns the 'box' of selected items */
    void get_clipboard_box (long *a_tick_s, int *a_note_h,
			    long *a_tick_f, int *a_note_l);

    /* removes and adds readds selected in position */
    void move_selected_notes (long a_delta_tick, int a_delta_note);

    /* adds a single note on / note off pair */
    void add_note (long a_tick, long a_length, int a_note, bool a_paint = false);

    void add_event (long a_tick,
		    unsigned char a_status,
		    unsigned char a_d0, unsigned char a_d1, bool a_paint = false);

    void stream_event (event * a_ev);

    /* changes velocities in a ramping way from vel_s to vel_f  */
    void change_event_data_range (long a_tick_s, long a_tick_f,
				  unsigned char a_status,
				  unsigned char a_cc,
				  int a_d_s, int a_d_f);
				  //unsigned char a_d_s, unsigned char a_d_f);

    /* lfo tool */
    void change_event_data_lfo(double a_value, double a_range,
                               double a_speed, double a_phase, int a_wave,
                               unsigned char a_status, unsigned char a_cc);

    /* moves note off event */
    void increment_selected (unsigned char a_status, unsigned char a_control);
    void decrement_selected (unsigned char a_status, unsigned char a_control);

    void randomize_selected( unsigned char a_status, unsigned char a_control, int a_plus_minus );

    void adjust_data_handle( unsigned char a_status, int a_data );

    /* moves note off event */
    void grow_selected (long a_delta_tick);
    void stretch_selected(long a_delta_tick);

    /* deletes events */
    void remove_marked();
    bool mark_selected();
    void unpaint_all();

    /* unselects every event */
    void unselect ();

    /* verfies state, all noteons have an off,
       links noteoffs with their ons */
    void verify_and_link ();
    void link_new ();

    /* resets everything to zero, used when
       sequencer stops */
    void zero_markers();

    /* flushes a note to the midibus to preview its
       sound, used by the virtual paino */
    void play_note_on (int a_note);
    void play_note_off (int a_note);

    /* send a note off for all active notes */
    void off_playing_notes();

    //
    // Drawing functions
    //

    /* copy event list for thread-safe drawing */
    void reset_draw_list();

    /* each call seqdata( sequence *a_seq, int a_scale );fills the passed refrences with a
       events elements, and returns true.  When it
       has no more events, returns a false */
    draw_type get_next_note_event (long *a_tick_s,
				   long *a_tick_f,
				   int *a_note,
				   bool * a_selected, int *a_velocity);

    int get_lowest_note_event ();
    int get_highest_note_event ();

    bool get_next_event (unsigned char a_status,
                         unsigned char a_cc,
                         long *a_tick,
                         unsigned char *a_D0,
                         unsigned char *a_D1, bool * a_selected, int type = ALL_EVENTS);

    bool get_next_event (unsigned char *a_status, unsigned char *a_cc);

    sequence & operator= (const sequence & a_rhs);

    void fill_list (list < char >*a_list, int a_pos);

    void select_events (unsigned char a_status, unsigned char a_cc,
			bool a_inverse = false);
    void quantize_events (unsigned char a_status, unsigned char a_cc,
			 long a_snap_tick, int a_divide, bool a_linked =
			 false);
    void transpose_notes (int a_steps);
    void shift_events (int a_ticks);  // move selected events later/earlier in time
    void multiply_pattern( float a_multiplier );
    void reverse_pattern();
    void calulate_reverse(event &a_e);
};

#endif
