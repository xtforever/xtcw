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
# tex definitions for wbuild documentation generator.  use with wbuildmac.tex

@doctype

@t filename "../tex" ".tex"
@t start "\\input wbuildmac.tex\n" "\\bye\n"
@t class "\\Class" ""
@t name "{" "}"
@t superclass "\\Super{" "}\n"
@t publicvars "\\Publicvars\n" "\\End"
@t superclass "\\Super{" "}\n"
@t publicvars "\\Publicvars\n" "\\End"
@t privatevars "\\Privatevars\n" "\\End"
@t actions "\\Actions\n" "\\End"
@t translations "\\Translations\n" "\\End"
@t exports "\\Exports\n" "\\End"
@t methods "\\Methods\n" "\\End"
@t imports "\\Imports\n" "\\End"
@t utilities "\\Utilities\n" "\\End"
@t classvars "\\Classvars\n" "\\End"
@t section "\\Section\n" "\n\n"
@t macro "\\Macro\n" "\\endMacro\n"
@t publicvar "\\Publicvar{" "}\n"
@t action "\\Action{" "}"
@t code "\\Code\n" "\\endCode\n"
@t table "\\Table" "\\endTable\n"
@t tablehead "{" "}\n"
@t row "" "\\cr\n"
@t resname "" ""
@t resclass "&" ""
@t restype "&" ""
@t resdefault "&" ""
@t inline "{\\tt " "}"
@t underline "{\\underline}" ""
@t backslash "{\\backslash}" ""
@t tilde "{\\tilde}" ""
@t hashmark "{\\hashmark}" ""
@t dollar "{\\dollar}" ""
@t less "{\\langle}" ""
@t greater "{\\rangle}" ""
@t percent "{\\percent}" ""
@t caret "{\\caret}" ""
@t ampersand "{\\ampersand}" ""
@t lbrace "{\\lbrace}" ""
@t rbrace "{\\rbrace}" ""
@t bar "{\\bar}" ""
@t at "@" ""
@t type "{\\type}" ""
@t incl "{\\incl}" ""
@t contraints "\\Constraints\n" "\\End"
@t constraint "\\Constraint{" "}\n"
@t privconstraints "\\PrivateConstraints\n" "\\End"
@t privconstraint "\\PrivateConstraint{" "}\n"
