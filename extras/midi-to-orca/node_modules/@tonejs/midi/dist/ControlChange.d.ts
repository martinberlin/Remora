import { Header } from "./Header";
export declare type ControlChangeName = "modulationWheel" | "breath" | "footController" | "portamentoTime" | "volume" | "balance" | "pan" | "sustain" | "portamentoTime" | "sostenuto" | "softPedal" | "legatoFootswitch" | "portamentoControl";
interface ControlChangeMap {
    [key: number]: ControlChangeName;
}
/**
 * A map of values to control change names
 */
export declare const controlChangeNames: ControlChangeMap;
export declare const controlChangeIds: {};
/**
 * @typedef ControlChangeEvent
 * @property {number} controllerType
 * @property {number=} value
 * @property {number=} absoluteTime
 */
/**
 * Represents a control change event
 */
export declare class ControlChange implements ControlChangeInterface {
    /**
     * The number value of the event
     */
    value: number;
    /**
     * The tick time of the event
     */
    ticks: number;
    /**
     * @param {ControlChangeEvent} event
     * @param {Header} header
     */
    constructor(event: any, header: Header);
    /**
     * The controller number
     * @readonly
     */
    readonly number: number;
    /**
     * return the common name of the control number if it exists
     */
    readonly name: ControlChangeName;
    /**
     * The time of the event in seconds
     */
    time: number;
    toJSON(): ControlChangeJSON;
}
export interface ControlChangeJSON {
    number: number;
    ticks: number;
    time: number;
    value: number;
}
export interface ControlChangeInterface {
    number: number;
    ticks: number;
    time: number;
    value: number;
}
export {};
