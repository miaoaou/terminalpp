%%
%% This is file `exesheet.cls',
%% generated with the docstrip utility.
%%
%% The original source files were:
%%
%% exesheet.dtx  (with options: `class')
%% 
%% This is a generated file.
%% 
%% Copyright (C) 2020-2024 by Antoine Missier <antoine.missier@ac-toulouse.fr>
%% 
%% This file may be distributed and/or modified under the conditions of
%% the LaTeX Project Public License, either version 1.3 of this license
%% or (at your option) any later version. The latest version of this
%% license is in:
%% 
%%    http://www.latex-project.org/lppl.txt
%% 
%% and version 1.3 or later is part of all distributions of LaTeX version
%% 2005/12/01 or later.
%% 
\NeedsTeXFormat{LaTeX2e}[2005/12/01]
\ProvidesClass{exesheet}
[2024/02/13 v2.7 .dtx Typesetting exercise or exam sheets]

\RequirePackage{kvoptions}
\DeclareBoolOption[true]{exetoc}
\DeclareBoolOption[true]{setlist}
\DeclareStringOption[both]{output}
\DeclareStringOption[none]{display}
\DeclareBoolOption[false]{answerspace}
\DeclareStringOption[left]{marginpos}
\DeclareStringOption[expand]{marginwidth}
\DeclareStringOption[left]{noteragged}
\DeclareBoolOption[false]{checkpts}
\DeclareStringOption[false]{correct}
\DeclareOption*{\PassOptionsToClass{\CurrentOption}{article}}
\ProcessOptions \relax
\LoadClass{article}
\RequirePackage{exesheet}
\RequirePackage{schooldocs}
\endinput
%%
%% End of file `exesheet.cls'.
