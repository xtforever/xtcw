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
# nroff documentation format description for wbuild.
# generally its output can be used directly as a manpage
# I know the lines are long; I'm not sure if wbuild can handle breaking
# them up and I don't care.

@doctype @shortdoc

@t filename "../man" ".3"
@t start "'\\\" t\n" ""
@t class ".TH \"\" 3 \"\" \"Version Unknown To Mankind\" \"Free Widget Foundation\"\n" ""
@t name ".SH NAME\n" "\n.SH DESCRIPTION\n"
@t superclass "" ""
@t publicvars "\n.SS \"Public variables\"\n" "\n"
@t privatevars "\n.SS \"Private variables\"\n" "\n"
@t actions "\n.SS \"Actions\"\n" "\n"
@t translations "\n.SS \"Translations\"\n" ""
@t exports "\n.SS \"Exports\"\n" ""
@t methods "\n.hi\n.SS \"Methods\"\n" "\n.hi\n"
@t imports "\n.SH \"Imports\"\n" "\n"
@t utilities "\n.SH \"Utilities\"\n" "\n"
@t classvars "\n.SH \"Class variables\"\n" "\n"
@t section "\n" "\n"
@t macro "\\fBdef\\fP " ""
@t publicvar "\n.TP\n.I \"" "\"\n"
@t action "\n.TP\n.I \"" "\n\n"
@t code "\n.nf\n" "\n.fi\n"
@t table "\n.ps -2\n.TS\ncenter box;\ncBsss\nlB|lB|lB|lB\nl|l|l|l.\n" "\n.TE\n.ps +2\n"
@t tablehead "" "\nName\tClass\tType\tDefault\n"
@t row "" "\n"
@t resname "" ""
@t resclass "\t" ""
@t restype "\t" ""
@t resdefault "\t" ""
@t inline "\\fI" "\\fP"
@t underline "_" ""
@t backslash "\B\B" ""
@t tilde "~" ""
@t hashmark "#" ""
@t dollar "$" ""
@t less "<" ""
@t greater ">" ""
@t percent "%" ""
@t caret "^" ""
@t ampersand "" ""
@t lbrace "{" ""
@t rbrace "}" ""
@t bar "|" ""
@t at "@" ""
@t type "\n.B type\n" ""
@t incl "\n.B incl\n" ""
@t constraints "\n.SS \"Constraint resources\"\n" ""
@t constraint "\n.TP\n.I \"" "\"\n"
@t privconstraints "\n.SS \"Private constraint resources\"\n" ""
@t privconstraint "\n.TP\n.I \"" "\"\n"
