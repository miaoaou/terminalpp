%%
%% Herbert Voss <hvoss@tug.org>
%% Copyright (C) 2023
%%
%% This program can be redistributed and/or modified under the terms
%% of the LaTeX Project Public License Distributed from CTAN archives
%% in directory macros/latex/base/lppl.txt.
%%
%% ||1in+evenside --- |1in+oddside ---||
%

\def\do@hvFloat@doublePAGE#1{%  image on left and right page with caption on the right ----------------------------------
  % #1-> 0/1 caption under/right
%  \global\setlength\hv@tempWidthA{\the\dimexpr1in+\oddsidemargin-\hvSet@bindCorrection}%
%  \global\setlength\hv@leftPageObjectWidth{\the\dimexpr\paperwidth-1in-\evensidemargin-\hvSet@bindCorrection}%
  \expandafter\global\expandafter\savebox\expandafter\hvObjectBox\expandafter{\ifhv@useOBox\usebox{\hvOBox}\else\hv@floatObject\fi}%
  \expandafter\global\expandafter\savebox\expandafter\hv@boxLeftPage\expandafter{\clipbox*{0 -\depth{} \hv@leftPageObjectWidth{} \height}{\usebox\hvObjectBox}}%
  \expandafter\global\expandafter\savebox\expandafter\hv@boxRightPage\expandafter{\clipbox*{\hv@leftPageObjectWidth{} -\depth{} \width{} \height}{\usebox\hvObjectBox}}%
  %\checkoddpage
  \ifoddpage
    \hvfloat@typeout{do@hvFloat@doublePAGE: oddpage}%
    \if@twocolumn
      \if@firstcolumn
        \hvfloat@typeout{do@hvFloat@doublePAGE: ifoddpage->twocolumn->firstcolumn}%
        \afterpage{\do@hvFloat@doublePAGECaptionRight{#1}}%
      \else
        \hvfloat@typeout{do@hvFloat@doublePAGE: ifoddpage->twocolumn->secondcolumn}%
        \do@hvFloat@doublePAGECaptionRight{#1}%
      \fi
    \else
      \afterpage{\do@hvFloat@doublePAGECaptionRight{#1}}%
    \fi  
  \else
    \hvfloat@typeout{do@hvFloat@doublePAGE: evenpage}%
    %---------
    \hv@atPageBeginfalse
    \stepcounter{hvfloat@ref}%
    \zsavepos{\thehvfloat@ref}%
    \@tempy=\dimexpr-\zposy{\thehvfloat@ref} sp+\paperheight-1in-\topmargin-\headheight-\headsep\relax
    \@tempx=\dimexpr-\zposx{\thehvfloat@ref} sp+\evensidemargin+1in\relax
    %\typeout{>>>>\the\@tempx >>>>>\the\@tempy >>>>\f@size}%
    \ifdim\@tempy > \z@ 
      %\typeout{>>>>Case 0, Leere Box oder Text auf der Seite \thehvfloat@ref}%
      \ifdim\@tempx < \z@ 
         %\typeout{>>>>Case 0-1, Text oben auf der Seite  \thehvfloat@ref}%
      \else
        %\typeout{>>>>Case 0-2, Absolut oben links auf der Seite  \thehvfloat@ref}%
        \hv@atPageBegintrue
      \fi
    \else
      \ifdim\@tempx > \z@ 
        %\typeout{>>>>Case 1,  Oben auf der Seite  \thehvfloat@ref, aber nicht am Anfang der Zeile}%
      \else
        %\typeout{>>>>Case 2,  Absolut oben links auf der Seite  \thehvfloat@ref }%
        \hv@atPageBegintrue
      \fi
    \fi
  %---------
    \ifhv@atPageBegin
      \do@hvFloat@doublePAGECaptionRight{#1}%
    \else
      \if@twocolumn
        \if@firstcolumn
          \afterpage{\afterpage{\afterpage{afterpage{\do@hvFloat@doublePAGECaptionRight{#1}}}}}%
        \else
          \afterpage{\afterpage{\afterpage{\do@hvFloat@doublePAGECaptionRight{#1}}}}%
        \fi
      \else
        \afterpage{\afterpage{\do@hvFloat@doublePAGECaptionRight{#1}}}%   onecolumn/left page
      \fi
    \fi %end \hv@atPageBegintrue
  \fi
  \endgroup% startet at main macro
}
%
\def\do@hvFloat@doublePAGECaptionRight#1{%  image on left and right page with caption on the right ----------------------------------
  % #1-> 0/1 caption under/right
    \hvfloat@typeout{do@hvFloat@doublePAGECaptionRight->start}%
      \hfuzz=\maxdimen
      \expandafter\hvFloatSet\expandafter{\hv@save@setting}%
      \ifx\hv@caption@format\@empty\else\expandafter\captionsetup\expandafter{\hv@caption@format}\fi
      \ifx\hv@subcaption@format\@empty\else
        \expandafter
           \captionsetup\expandafter[\expandafter s\expandafter u\expandafter b\expandafter]\expandafter
                             {\hv@subcaption@format}%
      \fi
      \ifhv@useOBox
        \global\let\hvObjectBox\hvOBox
      \else      
        \global\savebox\hvObjectBox{\hv@floatObject}%
      \fi
      \noindent
      \global\hv@tempWidthA=\the\dimexpr1in+\oddsidemargin-\hvSet@bindCorrection%
      \global\hv@leftPageObjectWidth=\the\dimexpr\paperwidth-1in-\evensidemargin-\hvSet@bindCorrection%
      \clipbox*{0 -\depth{} \hv@leftPageObjectWidth{} \height}{\usebox\hvObjectBox}%
      \ifx\hv@label\@empty\else\label{\hv@label}\fi            
      \null\newpage\if@twocolumn\null\newpage\fi
      \expandafter\global\expandafter\savebox\expandafter\hvObjectBox\expandafter{\ifhv@useOBox\usebox{\hvOBox}\else\hv@floatObject\fi}%
      \noindent
      \hspace*{\dimexpr-\hv@tempWidthA}%
      \clipbox*{\the\hv@leftPageObjectWidth{} -\depth{} \width{} \height{}}{\usebox\hvObjectBox}%
      \ifx\hv@label\@empty\else\label{\hv@label-2}\fi            
      \begingroup
          \ifnum#1>0
            \medskip
            \ifdim\dp\hvObjectBox > \z@
              \rotatebox[origin=c]{90}{\parbox{\the\dimexpr\ht\hvObjectBox+\dp\hvObjectBox}{%
                \ifx\relax\hv@shortCap\relax
                  \captionof{\hv@floatType}{\hv@longCap}%
                \else
                  \captionof{\hv@floatType}[\hv@shortCap]{\hv@longCap}%
                \fi
              \ifx\hv@label\@empty\else\label{\hv@label-cap}\fi            
              }}%
            \else
              \rotatebox{90}{\parbox{\the\dimexpr\ht\hvObjectBox+\dp\hvObjectBox}{%
                \ifx\relax\hv@shortCap\relax
                  \captionof{\hv@floatType}{\hv@longCap}%
                \else
                  \captionof{\hv@floatType}[\hv@shortCap]{\hv@longCap}%
                \fi
              \ifx\hv@label\@empty\else\label{\hv@label-cap}\fi            
              }}%
            \fi
        \else% caption not rotated
          \par\noindent
          \parbox{\textwidth}{%
      \expandafter\hvFloatSet\expandafter{\hv@save@setting,capFormat={font={tiny},labelsep=newline}}%
              \ifx\relax\hv@shortCap\relax
                \captionof{\hv@floatType}{\hv@longCap}%
              \else
                \captionof{\hv@floatType}[\hv@shortCap]{\hv@longCap}%
              \fi
              \ifx\hv@label\@empty\else\label{\hv@label-cap}\fi
            }%
        \fi
        \endgroup
        \newpage\if@twocolumn\null\newpage\fi
}
%
