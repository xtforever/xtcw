#     wbuild
#     Copyright (C) 1996  Joel N. Weber II <nemo@koa.iolani.honolulu.hi.us>
#     
#     This program is free software; you can redistribute it and/or
#     modify it under the terms of the GNU General Public License
#     as published by the Free Software Foundation; either version 2
#     of the License, or (at your option) any later version.
#     
#     This program is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#     GNU General Public License for more details.
#     
#     You should have received a copy of the GNU General Public License
#     along with this program; if not, write to the Free Software
#     Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#
# texi.w: definitions for generating Texinfo documentation
# this assumes that you will @include wbuild's output from another .texi
# file and include the @node command in the parent file; there is no
# reasonable way to anticipate the back and forward arguments to the
# @node command, though letting makeinfo do the work might be ok.

@doctype @shortdoc

@t filename "../doc" ".texi"
@t start "@c Generated automagically by wbuild; please don't edit\n" ""
@t class "" ""
@t name "@node " "\n"
@t name2 "@chapter " "\n\n"
@t superclass "" ""
#@t menu "\n\n@menu\n" "@end menu\n\n"
@t publicvars "\n@section Public Variables\n\n@itemize @bullet"
	"\n\n@end itemize\n\n"
#@t publicmenu "* " " Public Variables::\n"
@t privatevars "\n@section Private Variables\n\n@itemize @bullet"
	"\n\n@end itemize\n\n"
#@t privatemenu "* " " Private Variables::\n"
@t actions "\n@section Actions\n\n@itemize @bullet\n\n"
	"\n\n@end itemize\n\n"
#@t actionmenu "* " " Actions::\n"
@t exports "\n@section Exports\n\n@itemize @bullet\n\n"
	"\n\n@end itemize\n\n"
#@t exportmenu "* " " Exports::\n"
@t methods "\n@section Methods\n\n@itemize @bullet\n\n"
	"\n\n@end itemize\n\n"
#@t methodmenu "* " " Methods::\n"
@t imports "\n@section Imports\n\n@itemize @bullet\n\n"
	"\n\n@end itemize\n\n"
#@t importmenu "* " " Imports::\n"
@t utilities "\n@section Utilities\n\n@itemize @bullet\n\n"
	"\n\n@end itemize\n\n"
#@t utilmenu "* " " Utilities::\n"
@t translations "\n@section Translations\n\n@itemize @bullet\n\n"
	"\n\n@end itemize\n\n"
#@t transmenu "* " " Translations\n"
@t classvars "\n@section Class Variables\n\n@itemize @bullet\n\n"
	"\n\n@end itemize\n\n"
@t constraints "\n@section Constraint Resources\n\n@itemize @bullet\n\n"
	"\n\n@end itemize\n\n"
@t privconstraints "\n@section Private Constraint Resources\n\n@itemize @bullet\n\n"
	"\n\n@end itemize\n\n"
@t constraint "\n\n@item\n" ":\n"
@t privconstraint "\n\n@item\n" ":\n"
@t section "" ""
@t macro "\n\n@item\n" ":\n"
@t publicvar "\n\n@item\n" ":\n"
@t action "\n\n@item\n" ":\n"
@t code "\n@example\n" "\n@end example\n"
@t table "\n@example\n" "@end example\n"
@t tablehead "\n" "\nName\tClass\tType\tDefault\n"
@t row "" "\n"
@t resname "" ""
@t resclass "\t" ""
@t restype "\t" ""
@t resdefault "\t" ""
@t inline "@code{" "}"
@t underline "_" ""
@t backslash "\B" ""
@t tilde "~" ""
@t hashmark "#" ""
@t dollar "$" ""
@t less "<" ""
@t greater ">" ""
@t percent "%" ""
@t caret "^" ""
@t ampersand "&" ""
@t lbrace "@{" ""
@t rbrace "@}" ""
@t bar "|" ""
@t at "@@" ""
@t type "\n@code{type} " ""
@t incl "\n@code{incl} " ""
