let midi = {};

let notemap = {'A':'A', 'B':'B', 'C':'C', 'D':'D', 'E':'E', 'F':'F', 'G':'G',
         'A#':'a', 'B#':'b', 'C#':'c', 'D#':'d', 'E#':'e', 'F#':'f', 'G#':'g',  };

/**
 * Outputs the midi into Orca format
 * @param {*} midi 
 */
function midiOut(trackId = 1) {
    let notetimes = [];
    let notenotes = [];
    let tracks = $("div#midi-tracks"),
    settings   = $("div#midi-settings"),
    octavesOut = $("#orca-octaves"),
    notesOut   = $("#orca-notes"),
    bpmEl      = $("input#bpm"),
    colsEl     = $("input#cols"),
    wrap       = $("input#wrap"),
    silenced   = $("input#silence-detect"),
    trackEl    = $("input:radio")
    ;
    // Clean out output areas
    octavesOut.val('');
    notesOut.val('');
    
    bpm  = bpmEl.val();
    cols = colsEl.val();
    //console.log(midi.header); // Note: Not all midis have a tempo header
    midiHeaderTempos = midi.header.tempos;
    midiBpm = (midiHeaderTempos.length) ? Math.round(midiHeaderTempos[0].bpm) : 0;
    console.log("ORCΛ settings BPM:"+bpm+" COLS:"+cols+" / Midi BPM: "+midiBpm);
    let trackNr = 1;
    // Millis per beat from BMP calculation
    let mb = Math.round(60000/bpmEl.val());
    let trackMillis = 0;
    let orcaOctaves = "";
    let orcaNotes   = "";
    tracks.html('<b>Midi select track</b><br>');

    outPrefix  = (wrap.is(":checked")) ? '#' :'';
    outPrepend = (wrap.is(":checked")) ? '#' :'';
    
    // Read the tracks
    midi.tracks.forEach(track => {
        //Tracks have notes and controlChanges for the moment we ignore the second ones
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
            // Extract Note // console.log('Note: '+note.name);
            // Check if # Sharp note 
            if (note.name.substr(1,1) === '#') {
                // Extract sharp
                notesel = note.name.substr(0,2);
                octasel = note.name.substr(2,1);
            } else {
                notesel = note.name.substr(0,1);
                octasel = note.name.substr(1,1);
            }
            // console.log(' orca: '+notemap[notesel]+' '+octasel);
            // Detect if the note lands into the output grid or there is a silence
            // The most important part of the music is the silence
            orcaOctaves += octasel;
            // We will not calculate silence for octaves 
            if (notesCnt % cols == 0)  {
                octavesOut.val(octavesOut.val()+outPrefix+orcaOctaves+outPrepend+"\n");
                orcaOctaves = '';
            }

            if (silenced.is(":checked")) {
                // We generate the array here but output notes later
                n = (notesel in notemap) ? notemap[notesel] : '';
                notetimes.push(noteTime);
                notenotes.push(n);
            } else {
                // All notes together no silence is taken in account
                orcaNotes += (notesel in notemap) ? notemap[notesel] : '';

                if (notesCnt % cols == 0)  {
                    notesOut.val(notesOut.val()+outPrefix+orcaNotes+outPrepend+"\n");
                    orcaNotes = '';
                }
            }           
            
            notesCnt++; 
        });
      
        // All this will happen if detect silences is checked
        if (silenced.is(":checked")) {
        lastnotetime = (notetimes[notetimes.length-1]);
        /* We iterate a stepper jumps millis per beat at a time from 0 to lastnotetime
           that goes from initial to last note and cherry picks the note or silence
        */
        let lastIndex = notetimes[0];

        // We have two counters in place. NoteCnt is the notes index
        // gridIndex is the position in the grid
        notesCnt = 0; 
        gridIndex = 1;
        orcaNotes = '';
        gridLines = 1;
        console.log('Last note time: '+lastnotetime);
        console.log(notetimes);
        console.log(notenotes);

        let degugTo = 100;
        for (var i = notetimes[0]; i <= lastnotetime; i+=mb) {
          if (gridIndex<degugTo) console.log("noteTime:"+notetimes[notesCnt]+">="+lastIndex+" && <= "+i);

          if (notetimes[notesCnt]>=lastIndex && notetimes[notesCnt]<=i) {
              // Note lands in grid
              orcaNotes += notenotes[notesCnt];
              if (gridIndex<degugTo) console.log(notenotes[notesCnt]+" at time:" +i+ " Note count: "+notesCnt);
              notesCnt++;

          } else {
              // We detected a silence
              if (gridIndex<degugTo) console.log('.');
              orcaNotes += ".";
              // This will skip notes to maintein time (Bad option, but there is any other?)
              while (i>notetimes[notesCnt]) {
                  notesCnt++;
              }
          }
          // Jump to next row if Cols match
          if (gridIndex % cols == 0)  {
              console.log('%'+cols+' matched. orcaNotes:'+orcaNotes);
            notesOut.val(notesOut.val()+outPrefix+orcaNotes+outPrepend+"\n");
            orcaNotes = '';
            if (gridIndex<degugTo) console.log('LINE '+gridLines+'________________');
            gridLines++;
          }
          lastIndex = i;
          gridIndex++;
        }
    } // Silence mapping
        
    } // End of track loop

        // The track also has a channel and instrument
        trackChecked = (trackNr === trackId) ? "checked" : "";
        trackRadio = $('<input type="radio" name="track" value="'+trackNr+'" onchange="midiOut('+trackNr+')" id="t'+trackNr+'" '+trackChecked+' /> <label for="t'+trackNr+'">'+ track.instrument.name +'</label><br>');
            tracks.append(trackRadio);
            trackNr++;
        });
    }

$(document).ready(function() {
    // Load a midi file in the browser. Demo: old-town-road.mid samba-pa-ti.mid tiersen_amelie.mid
    const midiPromise = Midi.fromUrl("midis/d-mode-rumours.mid"); // Returns promise
    midiPromise.then(function (midiIn) {
        midi = midiIn;
        midiOut();
    });
    calculateMillisPerBeat();
});

function calculateMillisPerBeat() {
    let bpm = $("input#bpm").val();
    let millisPerBeat = Math.round(60000/bpm);
    $("div#millis-beat").html(millisPerBeat +" millis per beat")
}
