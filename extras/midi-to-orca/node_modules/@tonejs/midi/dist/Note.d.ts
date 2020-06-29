import { Header } from "./Header";
/**
 * A Note consists of a noteOn and noteOff event
 */
export declare class Note implements NoteInterface {
    /**
     * The notes midi value
     */
    midi: number;
    /**
     * The normalized velocity (0-1)
     */
    velocity: number;
    /**
     * The velocity of the note off
     */
    noteOffVelocity: number;
    /**
     * The start time in ticks
     */
    ticks: number;
    /**
     * The duration in ticks
     */
    durationTicks: number;
    constructor(noteOn: NoteOnEvent, noteOff: NoteOffEvent, header: Header);
    /**
     * The note name and octave in scientific pitch notation, e.g. "C4"
     */
    name: string;
    /**
     * The notes octave number
     */
    octave: number;
    /**
     * The pitch class name. e.g. "A"
     */
    pitch: string;
    /**
     * The duration of the segment in seconds
     */
    duration: number;
    /**
     * The time of the event in seconds
     */
    time: number;
    /**
     * The number of measures (and partial measures) to this beat.
     * Takes into account time signature changes
     * @readonly
     */
    readonly bars: number;
    toJSON(): NoteJSON;
}
export interface NoteJSON {
    time: number;
    midi: number;
    name: string;
    velocity: number;
    duration: number;
    ticks: number;
    durationTicks: number;
}
export interface NoteOnEvent {
    ticks: number;
    velocity: number;
    midi: number;
}
export interface NoteOffEvent {
    ticks: number;
    velocity: number;
}
export interface NoteInterface {
    time: number;
    ticks: number;
    duration: number;
    durationTicks: number;
    midi: number;
    pitch: string;
    octave: number;
    name: string;
    noteOffVelocity: number;
    velocity: number;
}
