let midi = {};

notea = {'A':'A', 'B':'B', 'C':'C', 'D':'D', 'E':'E', 'F':'F', 'G':'G',
         'A#':'a', 'B#':'b', 'C#':'c', 'D#':'d', 'E#':'e', 'F#':'f', 'G#':'g',  };

/**
 * Outputs the midi into Orca format
 * @param {*} midi 
 */
function midiOut(trackId = 1) {

    let tracks = $("div#midi-tracks"),
    settings   = $("div#midi-settings"),
    octavesOut = $("#orca-octave"),
    notesOut = $("#orca-notes"),
    bpmEl  = $("input#bpm"),
    colsEl = $("input#cols"),
    wrap = $("input#wrap"),
    trackEl = $("input:radio")
    
    // Clean out output areas
    octavesOut.val('')
    notesOut.val('')

    // If promise is done
    bpm  = bpmEl.val();
    cols = colsEl.val();
    //console.log(midi.header);
    midiHeaderTempos = midi.header.tempos;
    midiBpm = (midiHeaderTempos.length) ? Math.round(midiHeaderTempos[0].bpm) : 0;
    console.log("ORCΛ settings BPM:"+bpm+" COLS:"+cols+" / Midi BPM: "+midiBpm);
    let trackNr = 1;
    let orcaNotes;
    tracks.html('<b>Midi select track</b><br>');

    outPrefix  = (wrap.is(":checked")) ? '#' :'';
    outPrepend = (wrap.is(":checked")) ? '#' :'';

    // Read the tracks
    midi.tracks.forEach(track => {
        //tracks have notes and controlChanges
        if (trackId === trackNr) {
        console.log("Processing: "+track.instrument.name);
        //notes are an array
        const notes = track.notes;
        console.log("Note    Time     Dur.    Name");
        let debug = 1; 
        let notesCnt = 1;
        notes.forEach(note => {
            noteTime = Math.round(1000*note.time);
            if (debug<=4) {
                console.log(note.midi, note.time, note.duration, note.name);
                debug ++; 
            } 
            if (debug>3 && debug<=6) {
                console.log(noteTime, note.name);
                debug ++; 
            }
            orcaNotes += note.name.substr(0,1);
            
            // Jump to next row
            if (notesCnt % cols == 0)  {
                notesOut.val(notesOut.val()+outPrefix+orcaNotes+outPrepend+"\n");
                orcaNotes = '';
            }
            
            notesCnt++; 
        });
        
        
    }
        // the track also has a channel and instrument
        trackChecked = (trackNr === trackId) ? "checked" : "";
        trackRadio = $('<input type="radio" name="track" value="'+trackNr+'" onchange="midiOut('+trackNr+')" id="t'+trackNr+'" '+trackChecked+' /> <label for="t'+trackNr+'">'+ track.instrument.name +'</label><br>');
            tracks.append(trackRadio);
            trackNr++;
        });
    }

$(document).ready(function() {

// Load a midi file in the browser. Demo: old-town-road.mid samba-pa-ti.mid tiersen_amelie.mid
    const midiPromise = Midi.fromUrl("midis/tiersen_amelie.mid"); // Returns promise
    midiPromise.then(function (midiIn) {
        midi = midiIn;
        midiOut();
    });
});

