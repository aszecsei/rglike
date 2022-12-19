export interface Message {
    id: string
    value: Pattern | null
    attributes: Record<string, Pattern>
}

export interface Term {
    id: string
    value: Pattern
    attributes: Record<string, Pattern>
}

export type Pattern = string | ComplexPattern

export type ComplexPattern = Array<PatternElement>

export type PatternElement = string | Expression

export type Expression =
    | SelectExpression
    | VariableReference
    | TermReference
    | MessageReference
    | FunctionReference
    | Literal

export interface SelectExpression {
    type: 'select'
    selector: Expression
    variants: Array<Variant>
    star: number
}

export interface VariableReference {
    type: 'var'
    name: string
}

export interface TermReference {
    type: 'term'
    name: string
    attr: string | null
    args: Array<Expression | NamedArgument>
}

export interface MessageReference {
    type: 'mesg'
    name: string
    attr: string | null
}

export interface FunctionReference {
    type: 'func'
    name: string
    args: Array<Expression | NamedArgument>
}

export interface Variant {
    key: Literal
    value: Pattern
}

export interface NamedArgument {
    type: 'narg'
    name: string
    value: Literal
}

export type Literal = StringLiteral | NumberLiteral

export interface StringLiteral {
    type: 'str'
    value: string
}

export interface NumberLiteral {
    type: 'num'
    value: number
    precision: number
}
