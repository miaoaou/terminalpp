% tkz-doc.cls
% encoding: utf8
% Copyright 2022  Alain Matthes
% This work may be distributed and/or modified under the
% conditions of the LaTeX Project Public License, either version 1.3
% of this license or (at your option) any later version.
% The latest version of this license is in
%   http://www.latex-project.org/lppl.txt
% and version 1.3 or later is part of all distributions of LaTeX
% version 2005/12/01 or later.
%
% This work has the LPPL maintenance status “maintained”.
%
% The Current Maintainer of this work is Alain Matthes.
%
% tkz-doc class for use with the tkz documentation
% ( AlterMundus Collection ).
%
%% Objet :  Classe LaTeX  and Styles to create documentation.
%<-------------- Définitions relatives au package --------------------------->
\newcommand*{\PackageName}{tkz-doc}
\newcommand*{\PackageVersion}{1.45c}
\newcommand*{\filedate}{2022/07/14}

\DeclareOption*{\PassOptionsToClass{\CurrentOption}{scrartcl}}

%<-------------------- Identification Part -------------------------------->
\NeedsTeXFormat{LaTeX2e}[1995/12/01]
\ProvidesClass{\PackageName}[%
            \filedate\space Version:\space\PackageVersion\space %
                     tkz documentation class based on scrartcl]
\newif\if@tkzcadre
\@tkzcadrefalse
\DeclareOption{cadre}{\@tkzcadretrue}
\ProcessOptions\relax
\LoadClass[titlepage,a4paper]{scrartcl}
%<----------------- loading some packages ---------------------------------->
%  dvipsnames loads a set of 68 cmyk colors as defined in the dvips driver.
%  svgnames loads a set of 151 rgb color names
%  obsolete usenames
\PassOptionsToPackage{dvipsnames,svgnames}{xcolor}
%<-------------------------------------------------------------------------->
\KOMAoptions{twoside=false}
%<-------------------------------------------------------------------------->
\RequirePackage{tikz}
\usetikzlibrary{decorations.shapes,
                decorations.text,
                decorations.pathreplacing,
                decorations.pathmorphing,
                decorations.markings,
                shadows}
\RequirePackage{ragged2e}
\RequirePackage[bottom,stable,ragged]{footmisc}
\raggedbottom
\RequirePackage{framed,eso-pic}
\RequirePackage[automark,markcase=ignoreuppercase]{scrlayer-scrpage}
\RequirePackage[yyyymmdd]{datetime}
\RequirePackage{booktabs,cellspace,multicol}
%<------------- define important words ------------------------------------->
\InputIfFileExists{tkz-doc.cfg}{\typeout{Local configuration file tkz-doc.cfg found and used}}{\typeout{tkz-doc.cfg not found}} %<-------------------------------------------------------------------------->
\pagestyle{scrheadings}
\clearscrheadings
\clearscrplain
\ohead{\textcolor{sectioncolor}{\pagemark}}
\ihead{\textcolor{sectioncolor}{\headmark}}
\rofoot{\textcolor{sectioncolor}{\tkznamecollection}}
\lofoot{\textcolor{sectioncolor}{\tkznameofpack}}

\automark[subsection]{section}
\KOMAoptions{headsepline=.4pt}
\addtokomafont{headsepline}{\color{sectioncolor}}
\setheadwidth[0pt]{text}
\KOMAoptions{footsepline=.4pt}
\addtokomafont{footsepline}{\color{sectioncolor}}
\setfootwidth[0pt]{text}
\renewcommand{\footnoterule}{\vspace*{-3pt}{%
   \color{sectioncolor}\hrule width 2in height 0.4pt}\vspace*{2.6pt}}
\deffootnote{1.5em}{1em}{\makebox[1.5em][l]{\thefootnotemark}}
\addtolength{\skip\footins}{\baselineskip}%
%<------------------–  En-tête et pied de page ------------------------------
\pagenumbering{arabic}
\newcommand\SectionFontStyle{\ttfamily}
\setkomafont{sectioning}{\bfseries\SectionFontStyle}
\setkomafont{pagenumber}{\bfseries\SectionFontStyle}
\setkomafont{pagehead}{\small\ttfamily}
\setkomafont{pagefoot}{\small\ttfamily}
\setkomafont{descriptionlabel}{\itshape}
\addtokomafont{sectioning}{\color{sectioncolor}}
\renewcommand*{\raggedsection}{\raggedright}
%<---------------------------------------------------------------------------
\newcounter{cnt}\setcounter{cnt}{1}
\setcounter{secnumdepth}{3}%
\setcounter{tocdepth}{3}
%<------------------  Éxécution et chargement -----------------------------


%<---------------  some macros --------------------------------------------
\newbox\tkzbox
\def\nodeshadowedone(#1,#2)#3{%
\node[myblue,above,inner sep=0pt]{\global\setbox\tkzbox=\hbox{#3}\copy\tkzbox};
\node[above,yscale=#2,inner sep=0pt,xslant=#1,myblue!60]{\box\tkzbox};
}%
 % some of the macros below are absolete
\def\nameoffile#1{\gdef\@nameoffile{#1}}
\def\@nameoffile{\@latex@warning@no@line{No \noexpand\nameoffile given}}
\long\def\defoffile#1{\protected@xdef\@defoffile{\@defoffile{#1}}}
\let\@defoffile\@empty
%<-------------------------------------------------------------------------
\newcommand*{\addbs}[1]{\texttt{\textbackslash#1}} %\cs parfois ??

%<-------------------------- NewMacroBox ----------------------------------->
\newcommand*{\ActivBoxName}{}
\newenvironment{NewMacroBox}[2]{%
\parindent=0pt
\renewcommand*{\ActivBoxName}{#1}
\def\FrameCommand{\color{MidnightBlue}{\vrule width 1pt\hspace{3pt}}}%
\MakeFramed{\advance\hsize-\width}%
\noindent
\makebox[0pt][l]{%
   \hspace{-4pt}%
   \setlength{\fboxrule}{1pt}%
   \setlength{\fboxsep}{3pt}%
   \fcolorbox{sectioncolor}{titlecolorbox}{%
      \rule{6pt}{0pt}%
      \makebox[\width+6pt][l]{\tkzcname{#1#2}}%
   }%
}\\*[.5\baselineskip]%
\noindent
% \ignorespaces\tkzcname{#1#3}
\Imacro{#1}%
}%
{%
\par\noindent%
\makebox[0pt][l]{%
\hspace{-6pt}%
\rule{6pt}{1pt}%
\rule{\textwidth}{1pt}%
\hspace{-1pt}%
\rule{1pt}{10pt}%
}%
\endMakeFramed
\noindent
\ignorespacesafterend
\par\medskip
}%
%<----------------- end NewMacroBox ----------------------------------------->


%<----------------------------- NewEnvBox ----------------------------------->
\let\fram@d=\MakeFramed  \let\endfram@d=\endMakeFramed
\newenvironment{NewEnvBox}[1]{%
\parindent=0pt
\renewcommand*{\ActivBoxName}{#1}
%\gdef\tkzEnvMacro{#1}
\def\FrameCommand{\color{sectioncolor}{\vrule width 1pt\hspace{3pt}}}%
\MakeFramed{\advance\hsize-\width}%
\noindent
\makebox[0pt][l]{%
\hspace{-4pt}%
\setlength{\fboxrule}{1pt}%
\setlength{\fboxsep}{3pt}%
\fcolorbox{sectioncolor}{titlecolorbox}{%
\rule{6pt}{0pt}%
\makebox[\width+6pt][l]{%
 \tkzcname{\textcolor{sectioncolor}{begin}\{\ActivBoxName\}}{\oarg{options}}%
 \hspace*{.25cm}\meta{environment contents}\hspace*{.25cm}%
 \tkzcname{\textcolor{sectioncolor}{end}\{\ActivBoxName\}}}%
}%
}\\*[.5\baselineskip]%
\noindent
% \ignorespaces\tkzcname{#1#3}
\Ienv{#1}
\color{MidnightBlue}}{%
\par\noindent%
\makebox[0pt][l]{%
\hspace{-6pt}%
\rule{6pt}{1pt}%
\rule{\textwidth}{1pt}%
\hspace{-1pt}%
\rule{1pt}{10pt}%
}%
\endMakeFramed
\noindent
\ignorespacesafterend
\par\medskip
}
%<------------------- end NewEnvBox ----------------------------------------->



%<--------------% Define mybox and box title style----------------------------->
% We need to save the node
% Every append after command might be useful to have this code

\def\savelastnode{\pgfextra\edef\tmpA{\tikzlastnode}\endpgfextra}
\def\restorelastnode{\pgfextra\edef\tikzlastnode{\tmpA}\endpgfextra}

\tikzstyle{mybox} = [draw=blue!50!black, very thick,
    rectangle, rounded corners, inner sep=10pt, inner ysep=20pt,text=darkgray]
\tikzstyle{fancytitle} =[fill=MidnightBlue!20, text=blue!50!black,rounded corners]
\tikzstyle{title} = [append after command={%
    \savelastnode node[fancytitle,right=10pt] at (\tikzlastnode.north west)%
    {#1}\restorelastnode}]
%<------------------- end mybox ----------------------------------------->
    

%---------------  mots importants et macros ---------------------------------
\providecommand*{\BS}{\texttt{\char92}}
\providecommand*{\tkz}{T{\em k}Z}
\providecommand*{\PGF}{\tkzname{PGF}}      
\providecommand*{\TIKZ}{Ti\textsl{k}Z} 
\providecommand*{\tikzname}{Ti{\em k}Z}
\providecommand*{\pgfname}{\textsc{pgf}}
\providecommand*{\tkzdft}[1]{{\texttt{#1}}}
\providecommand*{\tkzAttention}[3]{\ \\\llap{\textcolor{#3}{#1\hskip #2}}} 
\providecommand*{\tkzHand}{\ \\\llap{\textcolor{red}{\lefthand\hskip1em}}}
\providecommand*{\tkzBomb}{\ \\\llap{\textcolor{red}{\bomb\hskip1em}}}
\providecommand*{\tkzTwoBomb}{\ \\\llap{\textcolor{red}{\bomb\ \bomb\hskip1em}}}
\providecommand*{\tkzname}[1]{\textbf{\texttt{\textcolor{black}{#1}}}}
\providecommand*{\tkzimp}[1]{\textbf{#1}}
\providecommand*{\tkzHandBomb}{\ \\\llap{\textcolor{red}{\lefthand\ \bomb\hskip1em}}}
\providecommand\LaTeX{L\kern-.36em\raise.3ex\hbox{\sc a}\kern-.15em\TeX}
\renewcommand\TeX{T\kern-.1667em\lower.5ex\hbox{E}\kern-.125emX\spacefactor1000}
\providecommand\LATEX{\LaTeX} 
\providecommand\TEX{\TeX} 

%<----------------------------- def macro arg opt---------------------------->
\DeclareRobustCommand\cs[1]{\texttt{\char`\\#1}}
\def\cmd@to@cs#1#2{\char\number`#2\relax}
\def\cmd#1{\cs{\expandafter\cmd@to@cs\string#1}}

\providecommand\marg[1]{{\ttfamily\char`\{}\opt{\meta{#1}}{\ttfamily\char`\}}}
\providecommand\parg[1]{{\ttfamily(}\opt{\meta{#1}}{\ttfamily)}}
\providecommand\oarg[1]{{\ttfamily[}\opt{\meta{#1}}{\ttfamily]}}
\providecommand\ooarg[1]{{\ttfamily[}\meta{#1}{\ttfamily]}}
\providecommand\opt[1]{{\color{myblue}#1}}
\providecommand*{\var}[1]{\{\opt{\meta{#1}}\}}
\providecommand*{\varp}[1]{\big(\opt{\meta{#1}}\big)}
\providecommand*{\vara}[1]{\{#1\}}

% -- Macros pour indexation -------------------------------------------------
% It is necessary to think about \protect
\newcommand*{\tkzNamePack}[1]{\NamePack{#1}}
\newcommand*{\tkzNameMacro}[1]{\tkzcname{#1}\Imacro{#1}}
\newcommand*{\tkzNameEnv}[1]{\tkzname{#1}\Ienv{#1}}
\newcommand*{\Ilib}[1]{\index{Library_1@\texttt{Library}!\texttt{#1}}}
\newcommand*{\Ienv}[1]{\index{Environment_1@\texttt{Environment}!\texttt{#1}}}
\newcommand*{\IargEnv}[2]{\index{#1_2@\texttt{#1: arguments}!\texttt{#2}}}
\newcommand*{\IstyleEnv}[2]{\index{#1_3@\texttt{#1: styles}!\texttt{#2}}}
\newcommand*{\Iaccent}[2]{\index{#1@\texttt{#2}}}
\newcommand*{\IargNameEnv}[2]{\texttt{#2}\index{#1_2@\texttt{#1: arguments}!\texttt{#2}}}
\newcommand*{\IoptEnv}[2]{\index{#1_3@\texttt{#1: options}!\texttt{#2}}}
\newcommand*{\IoptNameEnv}[2]{\texttt{#2}\index{#1_3@\texttt{#1: options}!\texttt{#2}}}
\newcommand*{\Imacro}[1]{\index{#1_1@\texttt{\textbackslash#1}}}%n
\newcommand*{\Iarg}[2]{\index{#1_2@\texttt{\textbackslash#1: arguments}!\texttt{#2}}}
\newcommand*{\IargName}[2]{\texttt{#2}\index{#1_2@\texttt{\textbackslash#1: arguments}!\texttt{#2}}}
\newcommand*{\IoptName}[2]{\texttt{#2}\index{#1_3@\texttt{\textbackslash#1: options}!\texttt{#2}}}
\newcommand*{\Iopt}[2]{\index{#1_3@\texttt{\textbackslash#1: options}!\texttt{#2}}}
\newcommand*{\Istyle}[2]{\index{#1_3@\texttt{\textbackslash#1: styles}!\texttt{#2}}}
\newcommand*{\NameLib}[1]{\texttt{\textcolor{MidnightBlue}{#1}}\index{Ti\emph{k}Z Library !#1@\texttt{#1}}}
\newcommand*{\NameSys}[1]{\texttt{#1}\index{Operating System  !#1@\texttt{#1}}}
\newcommand*{\NameDist}[1]{\texttt{\textcolor{MidnightBlue}{#1}}\index{TeX Distributions  !#1@\texttt{#1}}}
\newcommand*{\NameFonct}[1]{\texttt{\textcolor{MidnightBlue}{#1}}\index{Functions !#1@\texttt{#1}}}
\newcommand*{\NamePack}[1]{\texttt{\textcolor{MidnightBlue}{#1}}\index{Package !#1@\texttt{#1}}}
\newcommand*{\TOenvline}[3]{\IoptNameEnv{\ActivBoxName}{#1}&\tkzdft{#2}&\ttfamily #3\\}
\newcommand*{\TOline}[3]{\IoptName{\ActivBoxName}{#1}&\tkzdft{#2}&\ttfamily  #3\\}
\newcommand*{\TAline}[3]{\IargName{\ActivBoxName}{#1}&\tkzdft{#2}&\ttfamily  #3\\}
\newcommand*{\TMline}[3]{\tkzcname{#1}\index{#1_1@\texttt{\textbackslash#1}}&\tkzdft{#2}&\ttfamily  #3\\}

\newcommand*{\tkzhname}[1]{\textbf{\texttt{\textcolor{MidnightBlue}{%
                      \textbackslash#1}}}}
\newcommand*{\tkzNameSys}[1]{\texttt{\textcolor{MidnightBlue}{#1}}\index{Operating System   !#1@\texttt{#1}}}
\newcommand*{\tkzNameDist}[1]{\texttt{\textcolor{MidnightBlue}{#1}}\index{TeX Distributions  !#1@\texttt{#1}}}          \newcommand{\red}{\color{BrickRed}}
% print #1 but add index
\newcommand*{\tkzcname}[1]{\Imacro{#1}\textbf{\texttt{\textcolor{MidnightBlue}{%
 \textbackslash#1}}}}
\renewcommand\TeX{T\kern-.1667em\lower.5ex\hbox{E}\kern-.125emX%
  \spacefactor1000 }
\providecommand\LaTeX{%
  L\kern-.36em\raise.3ex\hbox{\sc a}\kern-.15em\TeX}
\providecommand\LATEX{\LaTeX} 
%    Some definitions useful in producing this sort of documentation:
\chardef\bslash=`\\ % p. 424, TeXbook
%    Normalized (nonbold, nonitalic) tt font, to avoid font
%    substitution warning messages if tt is used inside section
%    headings and other places where odd font combinations might
%    result.
\newcommand{\ntt}{\normalfont\ttfamily}
%    command name
\newcommand{\cn}[1]{{\protect\ntt\bslash#1}}
%    LaTeX package name
\newcommand{\pkg}[1]{{\protect\ntt#1}}
%    File name
\newcommand{\fn}[1]{{\protect\ntt#1}}
%    environment name
\newcommand{\env}[1]{{\protect\ntt#1}}
\hfuzz1pc % Don't bother to report overfull boxes if overage is < 1pc
         

\DeclareRobustCommand\meta[1]{%
\ensuremath\langle
     \ifmmode \expandafter \nfss@text \fi
     {%
      \meta@font@select
      \edef\meta@hyphen@restore
        {\hyphenchar\the\font\the\hyphenchar\font}%
      \hyphenchar\font\m@ne
      \language\l@nohyphenation
      \/#1\/%
      \meta@hyphen@restore
     }\ensuremath\rangle
}
\def\meta@font@select{}

\renewcommand*{\idx@heading}{%
 \idx@@heading{\indexname}%
}
\renewenvironment{theindex}{%
  \@restonecoltrue
  \columnseprule \z@
  \columnsep 35\p@
  \idx@heading%
  \index@preamble\par\nobreak
  \thispagestyle{\indexpagestyle}\parindent\z@
  \setlength{\parskip}{\z@ \@plus .3\p@}%
  \setlength{\parfillskip}{\z@ \@plus 1fil}%
  \let\item\@idxitem
}{%
  \if@restonecol\onecolumn\else\clearpage\fi
}
%<----------- set colors of the document --------------------------
\pgfkeys{/tkzsetupcolor/.cd,
          background/.code    = {\global\edef\tkz@suc@bkc{#1}},
          text/.code          = {\global\edef\tkz@suc@txt{#1}}}
\def\tkzSetUpColors{\pgfutil@ifnextchar[{\tkz@SetUpColors}{%
                                         \tkz@SetUpColors[]}}
\def\tkz@SetUpColors[#1]{%
\begingroup
    \pgfkeys{tkzsetupcolor/.cd,
             background  = \tkz@background@color,
             text        = \tkz@text@color}
    \pgfqkeys{/tkzsetupcolor}{#1}
    \pagecolor{\tkz@suc@bkc}
    \color{\tkz@suc@txt}
\endgroup}

%<---------------------------------------------------------------------
%<------------------  couverture.  ------------------------------------
\renewcommand\maketitle{%
\null
\AddToShipoutPicture*{%
\setlength\unitlength{1mm}
\put(0,0){%
   \makebox(0,0)[lb]{%
      \color{myblue!10}\rule{\paperwidth}{\paperheight}%
      }%
   }%
\put(0,0){%
   \makebox(0,0)[lb]{%
      \color{myblue}\rule{40mm}{\paperheight}%
      }%
   }%
\put(20,180){% 270 en portrait 180 en landscape
   \rotatebox{90}{%
      \makebox(0,0)[r]{%
         \fontsize{30}{30}\color{myblue!50!white}%
         \bfseries
         \selectfont{\@title\ \tkzversionofpack}\hfill%
         }%
      }%
   }%
\put(60,60){%
\begin{pgfpicture}
\pgfsys@transformxyscale{.5}{.5}
\pgfmathdeclarerandomlist{color}{{myblue}{DarkBlue}{SteelBlue}{blue}{MidnightBlue}}
 \foreach \a in {1,...,5}{%
      \pgfmathrandominteger{\x}{1}{300}%
      \pgfmathrandominteger{\y}{1}{200}%
      \pgfmathrandominteger{\r}{20}{100}%
      \pgfmathrandominteger{\s}{5}{95}%
      \pgfmathrandominteger{\o}{50}{80}%
      \pgfmathrandomitem{\c}{color}%
            \pgfdeclareradialshading{ballshading}{%
            \pgfpoint{-10bp}{10bp}}%
      {%
       color(0bp)=(\c!15!white);
       color(9bp)=(\c!75!white);
       color(18bp)=(\c!70!black);
       color(25bp)=(\c!50!black);
       color(50bp)=(black)%
       }%
      \pgfsetfillopacity{\o/100}%
      \pgfpathcircle{\pgfpoint{+\x pt}{+\y pt}}{+\r pt}
      \pgfshadepath{ballshading}{\s}%
      \pgfusepath{}%
      }%
   \end{pgfpicture}}

\put(60,250){% 60,250 en portait  120,180
   \makebox(0,0)[l]{%
      \begin{tikzpicture}
         \nodeshadowedone(2,-1.25){\fontsize{60}{60}
         \selectfont{\tkznamecollection}}
      \end{tikzpicture}
      }%
   }%
\put(120,40){%
   \makebox(0,0)[l]{%
      \fontsize{17}{21}
      \textcolor{sectioncolor}{\selectfont{\tkzauthorofpack}}%
      }%
   }%
\put(120,20){%
   \makebox(0,0)[l]{%
      \textcolor{sectioncolor}{\url{\tkzurlauthorcom}}%
      }%
   }%
\put(120,30){%
   \makebox(0,0)[l]{%
      \textcolor{sectioncolor}{\today \ \ Documentation V.\tkzversionofdoc}}%
      }%
   }%
\setcounter{footnote}{0}%
}%
%<----------------------- End   cover ---------------------------->
%<--------- presentation first page  --------------------------------->
\newcommand\presentation{%
   \thispagestyle{empty}%
   \par\vspace*{3 cm}
    \begin{center}
      {\textcolor{myblue}{\Huge \@nameoffile}}
    \end{center}
    \par\vspace*{3 cm}
    \noindent
    \newlength{\tkz@lx}\settowidth{\tkz@lx}{\huge \tkznamecollection}%
    \advance\tkz@lx3em\quad
    \href{\tkzurlauthor}{%
    \rotatebox{90}{\hspace*{1.5em}{%
        \textcolor{sectioncolor}{\Huge \tkznamecollection}}}}
        \textcolor{sectioncolor}{\rule{2pt}{\tkz@lx}}\quad%
  \begin{minipage}[b]{.70\textwidth}
    {%
    \noindent\hfill
     \raggedleft\Large
     \href{mailto:al.ma@mac.com}{\textcolor{sectioncolor}{\tkzauthorofpack}}\par
     }%
    {\@defoffile}
   \par
  \end{minipage}
}
%<----------------------- Spc. Table  ---------------------------->
\newcommand{\tkzsubf}[2]{%
  {\small\begin{tabular}[t]{@{}c@{}}
  #1\\#2
  \end{tabular}}%
}

\renewcommand{\labelitemi}{--}
\setlength\parindent{0pt}
\RedeclareSectionCommand[tocnumwidth=3.5em]{part}
\RedeclareSectionCommand[tocnumwidth=3.5em]{section}
\RedeclareSectionCommand[tocnumwidth=3.5em]{subsection}
\RedeclareSectionCommand[tocnumwidth=3.5em]{subsubsection}
\renewcommand\partheadstartvskip{\clearpage\null\vfil}
\renewcommand\partheadmidvskip{\par\nobreak\vskip 20pt\thispagestyle{empty}}
\renewcommand\partheadendvskip{\vfil\clearpage}
\renewcommand\raggedpart{\centering}

%<-------------------------------------------------------------------------
% code for booktab font change command see jos-astuces.rtfd
\newcommand*{\@CellCode}[1]{}
\newcommand*{\thead}{\noalign{\iffalse}\fi\@thead}
\newcommand*{\@thead}[1][\em\ttfamily]{\gdef\@CellCode{#1}\iffalse{\fi}}
\newcommand*{\tbody}{\noalign{\gdef\@CellCode{}}}
\newcolumntype{I}{>{\@CellCode}}%

% table inside the cover ---------------------------->

\def\tkzTitleFrame#1{%
\edef\titleinframe{#1}
\author{\tkzauthorofpack}  
\title{\tkznameofpack}
\date{\today}
\clearpage
\thispagestyle{empty}
\maketitle
\null
\if@tkzcadre 
\AddToShipoutPicture*{%
\setlength\unitlength{1mm}
\if@landscape\def\tkz@posy{90}\def\tkz@posx{160}%
\else\def\tkz@posx{70}\def\tkz@posy{140}
\fi
\put(\tkz@posx,\tkz@posy){%
\begin{tikzpicture}[color=framecolor,
every node/.style={inner sep=0pt}]
 \node[draw,line width=2pt,double,color=framecolor,
 fill=myblue!10,opacity=.5,minimum width=300pt,minimum height=240pt] (vecbox) {};
 \node[text width=240pt]  at (vecbox.center){%
  \begin{center}
    \color{MidnightBlue}
      \fontsize{24}{48}
    \selectfont\titleinframe%
 \end{center}};
 \node[anchor=north west] at (vecbox.north west)
{\pgfornament[width=2cm]{61}}; 
\node[anchor=north east] at (vecbox.north east)
{\pgfornament[width=2cm,symmetry=v]{61}}; 
\node[anchor=south west] at (vecbox.south west)
{\pgfornament[width=2cm,symmetry=h]{61}}; 
\node[anchor=south east] at (vecbox.south east) {\pgfornament[width=2cm,symmetry=c]{61}};
\end{tikzpicture}
}
}\fi}
\endinput