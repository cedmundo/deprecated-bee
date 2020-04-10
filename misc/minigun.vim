" Vim syntax file
" Language: Minigun
" Maintainer: Carlos Martínez
" Latest Revision: 02 Jan 2020

if exists("b:current_syntax")
  finish
endif

syn keyword mgnKeywords def let if else in for skipwhite
syn region mgnBlock start="(" end=")" fold transparent
syn region mgnString oneline start='"' skip=/\\./ end='"'

let b:current_syntax = "mgn"

hi def link mgnKeywords     Keyword
hi def link mgnString       Constant
