/**
 * @module i18n
 * @overview
 *
 * `i18n` is a TypeScript implementation of Project Fluent, a localization
 * framework designed to unleash the expressive power of the natural language.
 */

declare global {
    let CurrentLocale: string
}

export { FluentBundle, FluentVariable, TextTransform } from './bundle'
export { FluentResource } from './resource'
export {
    FluentValue,
    FluentType,
    FluentFunction,
    FluentNone,
    FluentNumber,
    FluentDateTime,
} from './types'

export const HELLO = 'WORDL'
