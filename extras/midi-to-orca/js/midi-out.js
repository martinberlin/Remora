/**
 * Outputs the midi into Orca format
 * @param {*} midi
 */
function midiOut(trackId = 1) {
    let notetimes = [];
    let notenotes = [];
    let noteocta = [];
    let tracks = $("div#midi-tracks"), settings = $("div#midi-settings"), octavesOut = $("#orca-octaves"), notesOut = $("#orca-notes"), bpmEl = $("input#bpm"), colsEl = $("input#cols"), wrap = $("input#wrap"), silenced = $("input#silence-detect"), trackEl = $("input:radio");
    // Clean out output areas
    octavesOut.val('');
    notesOut.val('');
    bpm = bpmEl.val();
    cols = colsEl.val();
    //console.log(midi.header); // Note: Not all midis have a tempo header
    midiHeaderTempos = midi.header.tempos;
    midiBpm = (midiHeaderTempos.length) ? Math.round(midiHeaderTempos[0].bpm) : 0;
    console.log("ORCΛ settings BPM:" + bpm + " COLS:" + cols + " / Midi BPM: " + midiBpm);
    let trackNr = 1;
    // Millis per beat from BMP calculation
    let mb = Math.round(60000 / bpmEl.val());
    let trackMillis = 0;
    let orcaOctaves = "";
    let orcaNotes = "";
    tracks.html('<b>Midi select track</b><br>');
    outPrefix = (wrap.is(":checked")) ? '#' : '';
    outPrepend = (wrap.is(":checked")) ? '#' : '';
    // Read the tracks
    midi.tracks.forEach(track => {
        //Tracks have notes and controlChanges for the moment we ignore the second ones
        if (trackId === trackNr) {
            if (debugMode) {
                console.log("Processing: " + track.instrument.name);
                console.log("Note    Time     Dur.    Name");
            }
            const notes = track.notes;
            let debugCnt = 1;
            let notesCnt = 1;

            // Iterate notes array
            notes.forEach(note => {
                noteTime = Math.round(1000 * note.time);
                if (debugMode && debugCnt <= 4) {
                    console.log(note.midi, note.time, note.duration, note.name);
                    console.log(noteTime, note.name);
                    debugCnt++;
                }
                // Extract Note // console.log('Note: '+note.name);
                // Check if # Sharp note 
                if (note.name.substr(1, 1) === '#') {
                    // Extract sharp
                    notesel = note.name.substr(0, 2);
                    octasel = note.name.substr(2, 1);
                }
                else {
                    notesel = note.name.substr(0, 1);
                    octasel = note.name.substr(1, 1);
                }
                // console.log(' orca: '+notemap[notesel]+' '+octasel);
                // Detect if the note lands into the output grid or there is a silence
                // The most important part of the music is the silence
                if (silenced.is(":checked")) {
                    // We generate the array here but output notes later
                    n = (notesel in notemap) ? notemap[notesel] : '';
                    notetimes.push(noteTime);
                    notenotes.push(n);
                    noteocta.push(octasel);
                }
                else {
                    // All notes together no silence is taken in account
                    orcaNotes += (notesel in notemap) ? notemap[notesel] : '';
                    orcaOctaves += octasel;
                    if (notesCnt % cols == 0) {
                        notesOut.val(notesOut.val() + outPrefix + orcaNotes + outPrepend + "\n");
                        orcaNotes = '';
                        octavesOut.val(octavesOut.val() + outPrefix + orcaOctaves + outPrepend + "\n");
                        orcaOctaves = '';
                    }
                }
                notesCnt++;
            });
            // All this will happen if detect silences is checked
            if (silenced.is(":checked")) {
                lastnotetime = (notetimes[notetimes.length - 1]);
                /* We iterate a stepper jumps millis per beat at a time from 0 to lastnotetime
                   that goes from initial to last note and cherry picks the note or silence
                */
                let lastIndex = notetimes[0];
                // We have two counters in place. NoteCnt is the notes index
                // gridIndex is the position in the grid
                notesCnt = 0;
                gridIndex = 1;
                orcaNotes = '';
                orcaOctaves = '';
                if (debugMode) {
                    console.log('Note end time: ' + lastnotetime);
                    console.log(notetimes);
                    console.log(noteocta);
                    console.log(notenotes);
                }
                debugCnt = 1;
                for (var i = notetimes[0]; i <= lastnotetime; i += mb) {
                    
                    if (notetimes[notesCnt] >= lastIndex && notetimes[notesCnt] <= i) {
                        // Note + octave lands in grid
                        orcaNotes += notenotes[notesCnt];
                        orcaOctaves += noteocta[notesCnt];
                    } else {
                    // We detected a silence
                        orcaNotes += ".";
                        orcaOctaves += ".";
                        // This will skip notes to maintein time (Bad option, but there is any other?)
                        while (i > notetimes[notesCnt]) {
                            notesCnt++;
                        }
                    }
                    // Jump to next row if Cols match
                    if (gridIndex % cols == 0) {
                        //console.log('%'+cols+' matched. orcaNotes:'+orcaNotes);
                        notesOut.val(notesOut.val() + outPrefix + orcaNotes + outPrepend + "\n");
                        octavesOut.val(octavesOut.val() + outPrefix + orcaOctaves + outPrepend + "\n");
                        orcaNotes = '';
                        orcaOctaves = '';
                    }
                    if (debugMode && debugCnt<=10) {
                        console.log("noteTime:" + notetimes[notesCnt] + ">=" + lastIndex + " && <= " + i);
                        if ((notetimes[notesCnt] >= lastIndex && notetimes[notesCnt] <= i)) {
                            console.log("note: "+notenotes[notesCnt]+" found for gridIndex: "+ gridIndex);
                        } else {
                            console.log("silence . added in gridIndex: "+ gridIndex);
                        }
                        debugCnt++;
                    }
                    lastIndex = i;
                    gridIndex++;
                }
            } // Silence mapping
        } // End of track loop
        // The track also has a channel and instrument
        trackChecked = (trackNr === trackId) ? "checked" : "";
        trackRadio = $('<input type="radio" name="track" value="' + trackNr + '" onchange="midiOut(' + trackNr + ')" id="t' + trackNr + '" ' + trackChecked + ' /> <label for="t' + trackNr + '">' + track.instrument.name + '</label><br>');
        tracks.append(trackRadio);
        trackNr++;
    });
}
