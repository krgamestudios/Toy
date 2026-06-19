" Vim syntax file
" Language: Toy

if exists("b:current_syntax")
  finish
endif

" === Strings ===
syn region toyString start=/"/ skip=/\\"/ end=/"/ contains=toyEscape
syn match toyEscape /\\[nt"\\]/ contained

" === Numbers ===
syn match toyInteger "\<\d[\d_]*\>"
syn match toyFloat "\<\d[\d_]*\.\d[\d_]*\>"

" === Booleans ===
syn keyword toyBoolean true false

" === Keywords ===
syn keyword toyStatement print assert if else while for in of import export as pass
syn keyword toyDeclaration fn class var
syn keyword toyControl break continue return yield
syn keyword toyConstKw const

" === Types ===
syn keyword toyType Bool Int Float String Function Opaque Any
syn keyword toyCollection Array Table

" === Function calls (identifier followed by '(') ===
syn match toyFunction "\<\w\+\ze("

" === Identifiers (general) ===
syn match toyIdentifier "\<\w\+\>"

" === Operators and Symbols ===
syn match toySymbol "[=:]"
syn match toyOperator "[-=:+*/%!<><=>&|?.]\+"

syn match toyBracket "[()[\]{}]"
syn match toyComma "[,;]"

" === Comments ===
syn match toyCommentLine "//.*$"
syn region toyCommentBlock start="/\*" end="\*/" fold

" === Highlighting links ===
hi def link toyCommentLine   Comment
hi def link toyCommentBlock  Comment

hi def link toyString        String
hi def link toyEscape        SpecialChar

hi def link toyInteger       Number
hi def link toyFloat         Number
hi def link toyBoolean       Boolean

hi def link toyStatement     Statement
hi def link toyDeclaration   Keyword
hi def link toyControl       Conditional
hi def link toyType          Type
hi def link toyCollection    Type
hi def link toyConstKw       Type

hi def link toyFunction      Function
hi def link toyIdentifier    Identifier

hi def link toySymbol        Operator
hi def link toyOperator      Operator
hi def link toyBracket       Delimiter
hi def link toyComma         Delimiter

" Set the current syntax
let b:current_syntax = "toy"
