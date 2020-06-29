"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
var BinarySearch_1 = require("./BinarySearch");
var ControlChange_1 = require("./ControlChange");
var ControlChanges_1 = require("./ControlChanges");
var Instrument_1 = require("./Instrument");
var Note_1 = require("./Note");
var privateHeaderMap = new WeakMap();
/**
 * A Track is a collection of notes and controlChanges
 */
var Track = /** @class */ (function () {
    function Track(trackData, header) {
        var _this = this;
        /**
         * The name of the track
         */
        this.name = "";
        /**
         * The track's note events
         */
        this.notes = [];
        /**
         * The control change events
         */
        this.controlChanges = ControlChanges_1.createControlChanges();
        privateHeaderMap.set(this, header);
        if (trackData) {
            var nameEvent = trackData.find(function (e) { return e.type === "trackName"; });
            this.name = nameEvent ? nameEvent.text : "";
        }
        /** @type {Instrument} */
        this.instrument = new Instrument_1.Instrument(trackData, this);
        this.channel = 0;
        if (trackData) {
            var noteOns = trackData.filter(function (event) { return event.type === "noteOn"; });
            var noteOffs = trackData.filter(function (event) { return event.type === "noteOff"; });
            var _loop_1 = function () {
                var currentNote = noteOns.shift();
                // find the corresponding note off
                var offIndex = noteOffs.findIndex(function (note) { return note.noteNumber === currentNote.noteNumber; });
                if (offIndex !== -1) {
                    // once it's got the note off, add it
                    var noteOff = noteOffs.splice(offIndex, 1)[0];
                    this_1.addNote({
                        durationTicks: noteOff.absoluteTime - currentNote.absoluteTime,
                        midi: currentNote.noteNumber,
                        noteOffVelocity: noteOff.velocity / 127,
                        ticks: currentNote.absoluteTime,
                        velocity: currentNote.velocity / 127,
                    });
                }
            };
            var this_1 = this;
            while (noteOns.length) {
                _loop_1();
            }
            var controlChanges = trackData.filter(function (event) { return event.type === "controller"; });
            controlChanges.forEach(function (event) {
                _this.addCC({
                    number: event.controllerType,
                    ticks: event.absoluteTime,
                    value: event.value / 127,
                });
            });
            // const endOfTrack = trackData.find(event => event.type === "endOfTrack");
        }
    }
    /**
     * Add a note to the notes array
     * @param props The note properties to add
     */
    Track.prototype.addNote = function (props) {
        if (props === void 0) { props = {}; }
        var header = privateHeaderMap.get(this);
        var note = new Note_1.Note({
            midi: 0,
            ticks: 0,
            velocity: 1,
        }, {
            ticks: 0,
            velocity: 0,
        }, header);
        Object.assign(note, props);
        BinarySearch_1.insert(this.notes, note, "ticks");
        return this;
    };
    /**
     * Add a control change to the track
     * @param props
     */
    Track.prototype.addCC = function (props) {
        var header = privateHeaderMap.get(this);
        var cc = new ControlChange_1.ControlChange({
            controllerType: props.number,
        }, header);
        delete props.number;
        Object.assign(cc, props);
        if (!Array.isArray(this.controlChanges[cc.number])) {
            this.controlChanges[cc.number] = [];
        }
        BinarySearch_1.insert(this.controlChanges[cc.number], cc, "ticks");
        return this;
    };
    Object.defineProperty(Track.prototype, "duration", {
        /**
         * The end time of the last event in the track
         */
        get: function () {
            var lastNote = this.notes[this.notes.length - 1];
            if (lastNote) {
                return lastNote.time + lastNote.duration;
            }
            else {
                return 0;
            }
        },
        enumerable: true,
        configurable: true
    });
    Object.defineProperty(Track.prototype, "durationTicks", {
        /**
         * The end time of the last event in the track in ticks
         */
        get: function () {
            var lastNote = this.notes[this.notes.length - 1];
            if (lastNote) {
                return lastNote.ticks + lastNote.durationTicks;
            }
            else {
                return 0;
            }
        },
        enumerable: true,
        configurable: true
    });
    /**
     * Assign the json values to this track
     */
    Track.prototype.fromJSON = function (json) {
        var _this = this;
        this.name = json.name;
        this.channel = json.channel;
        this.instrument = new Instrument_1.Instrument(undefined, this);
        this.instrument.fromJSON(json.instrument);
        for (var number in json.controlChanges) {
            if (json.controlChanges[number]) {
                json.controlChanges[number].forEach(function (cc) {
                    _this.addCC({
                        number: cc.number,
                        ticks: cc.ticks,
                        value: cc.value,
                    });
                });
            }
        }
        json.notes.forEach(function (n) {
            _this.addNote({
                durationTicks: n.durationTicks,
                midi: n.midi,
                ticks: n.ticks,
                velocity: n.velocity,
            });
        });
    };
    /**
     * Convert the track into a JSON format
     */
    Track.prototype.toJSON = function () {
        // convert all the CCs to JSON
        var controlChanges = {};
        for (var i = 0; i < 127; i++) {
            if (this.controlChanges.hasOwnProperty(i)) {
                controlChanges[i] = this.controlChanges[i].map(function (c) { return c.toJSON(); });
            }
        }
        return {
            channel: this.channel,
            controlChanges: controlChanges,
            instrument: this.instrument.toJSON(),
            name: this.name,
            notes: this.notes.map(function (n) { return n.toJSON(); }),
        };
    };
    return Track;
}());
exports.Track = Track;
//# sourceMappingURL=Track.js.map