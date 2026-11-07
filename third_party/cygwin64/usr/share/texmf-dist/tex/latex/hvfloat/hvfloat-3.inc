%% Herbert Voss <hvoss@tug.org>
%% Copyright (C) 2023
%%
%% This program can be redistributed and/or modified under the terms
%% of the LaTeX Project Public License Distributed from CTAN archives
%% in directory macros/latex/base/lppl.txt.
%%
%%
%% ------------------------------------- the doublepage obejcts ---------------------------
%% ||1in+evenside --- |1in+oddside ---||
%
\def\do@hvFloat@doublePage#1{%  image on left and right page with caption on the right page 
  % #1-> 0/1 caption under/right
  \setcounter{topnumber}{1}%
  \setcounter{dbltopnumber}{1}%
  \hvfloat@typeout{----------------------------- do@hvFloat@doublePage#1 --------------------------}%
  \hvfloat@typeout{>>>do@hvFloat@doublePage: begin with definitions of \hv@floatObject}%
  \global\hv@leftPageObjectWidth=\the\dimexpr\paperwidth-1in-\evensidemargin-\hvSet@bindCorrection\relax
  \global\hv@tempWidthA=\the\dimexpr1in+\oddsidemargin-\hvSet@bindCorrection\relax
  \xdef\hv@caption@format@temp{\hv@caption@format}%  it gets lost otherwise for next afterpage
  \@dblfpbot=0\p@ \@plus 1fil%
  \global\hv@switchTypefalse
  \setcounter{hv@tempCNTfigA}{\value{figure}}%
  \setcounter{hv@tempCNTfigB}{\value{figure}}%
  \setcounter{hv@tempCNTtabA}{\value{table}}%
  \setcounter{hv@tempCNTtabB}{\value{table}}%
  \savebox\hvCaptionBox{%   NO rotation
    \minipage{\textwidth}%% minipage, to get hyphenation
      \let\@captype\hv@floatType
      \caption*{\hv@longCap}%
    \endminipage}%
  \global\savebox\hvObjectBox{\ifhv@useOBox\usebox{\hvOBox}\else\hv@floatObject\fi}%
  \ifnum#1=0\relax    % no rotation, caption below
%    \hvfloat@typeout{Texthöhe: \the\textheight}%
%    \hvfloat@typeout{Objekthöhe: \the\ht\hvObjectBox}%
%    \hvfloat@typeout{Captionhöhe: \the\ht\hvCaptionBox}%
    \@tempdima=\dimexpr\ht\hvObjectBox+\ht\hvCaptionBox+\abovecaptionskip+\belowcaptionskip +\textfloatsep\relax%+\floatsep\relax
%    \hvfloat@typeout{Summe: \the\@tempdima}%
    \ifdim\@tempdima > \dimexpr\textheight-\hv@minTextlines\relax
%      \hvfloat@typeout{hvfloat: switched to floattype p}%
      \hv@switchTypetrue
    \fi
  \fi
  \hvfloat@typeout{do@hvFloat@doublePage:hv@tempWidthA=\the\hv@tempWidthA}%
  \ifhv@forceOutput
    \do@hvFloat@doublePAGECaptionRight{#1}%   no cheque
  \else
  %\checkoddpage
  \ifoddpage
    \hvfloat@typeout{do@hvFloat@doublePage:oddpage}%
    \if@twocolumn
      \hvfloat@typeout{do@hvFloat@doublePage: oddpage/twocolumn}%
      \if@firstcolumn
        \hvfloat@typeout{do@hvFloat@doublePage:oddpage->twocolumn->firstcolumn}%
        \ifhv@switchType
          \hvfloat@typeout{hvfloat: switched to floattype p}%
          \afterpage{\do@hvFloat@doublePAGECaptionRight{#1}}%
        \else
          \hvfloat@typeout{calling do@hvFloat@doublePageCaptionRight}%
          %\afterpage{
          \afterpage{\do@hvFloat@doublePageCaptionRight{#1}}%}%
        \fi
      \else
%        \hvfloat@typeout{do@hvFloat@doublePage:oddpage->twocolumn->secondcolumn}%
        \ifhv@tugboat
          \do@hvFloat@doublePageCaptionRight{#1}%
        \else
          \ifhv@switchType
            \do@hvFloat@doublePAGECaptionRight{#1}%
          \else
            \afterpage{\do@hvFloat@doublePageCaptionRight{#1}}%
          \fi
        \fi
      \fi
    \else% onecolumn
      \hvfloat@typeout{do@hvFloat@doublePage: oddpage/onecolumn}%
      \ifhv@switchType
        \hvfloat@typeout{do@hvFloat@doublePage: oddpage/onecolumn/switchType}%
        \do@hvFloat@doublePAGECaptionRight{#1}%
      \else
        \hvfloat@typeout{do@hvFloat@doublePage: call do@hvFloat@doublePageCaptionRight #1}%
        \ifhv@forceLeft
          \do@hvFloat@doublePageCaptionRight{#1}%   no \afterpage
        \else
          \afterpage{\do@hvFloat@doublePageCaptionRight{#1}}%
        \fi
      \fi
    \fi  
  \else% we have an even page
    \hvfloat@typeout{do@hvFloat@doublePage:evenpage -> (\thepage)}%
    %\typeout{>>>>Setze hv@atPageBeginfalse}%
    \global\hv@atPageBeginfalse
    \stepcounter{hvfloat@ref}%
    \zsavepos{\thehvfloat@ref}%
    \@tempy=\dimexpr-\zposy{\thehvfloat@ref} sp+\paperheight-1in-\topmargin-\headheight-\headsep\relax
    \@tempx=\dimexpr-\zposx{\thehvfloat@ref} sp+\evensidemargin+1in\relax
    %\typeout{>>>>\the\@tempx >>>>>\the\@tempy >>>>\f@size}%
    \ifdim\@tempy > \dimexpr\f@size pt + 2pt % Interlinespace
      %\typeout{>>>>Case 0, Leere Box oder Text auf der Seite \thehvfloat@ref}%
    \else
      \ifdim\@tempx = \z@ 
        %\typeout{>>>>Case 2,  Absolut oben links auf der Seite  \thehvfloat@ref }%
        \global\hv@atPageBegintrue
      \else
        %\typeout{>>>>Case 1,  Oben auf der Seite  \thehvfloat@ref, aber nicht am Anfang der Zeile}%
      \fi
    \fi
  %---------
    \ifhv@atPageBegin% At the top left of the even page -> output image
       \do@hvFloat@doublePageCaptionRight{#1}
    \else
      \if@twocolumn
        \if@firstcolumn
          \ifhv@switchType
            \afterpage{\afterpage{\do@hvFloat@doublePAGECaptionRight{#1}}}%
          \else
            \afterpage{\afterpage{\afterpage{\do@hvFloat@doublePageCaptionRight{#1}}}}%
          \fi
        \else% second column
          \ifhv@switchType
            \afterpage{\do@hvFloat@doublePAGECaptionRight{#1}}%
          \else
            %\afterpage{
            \afterpage{\do@hvFloat@doublePageCaptionRight{#1}}%}%
          \fi
        \fi
      \else% onecolumn
        \hvfloat@typeout{do@hvFloat@doublePage:evenpage/onecolumn -> (\thepage)}%
        \ifhv@switchType
          \hvfloat@typeout{do@hvFloat@doublePage:evenpage/onecolumn/switchType -> (\thepage)}%
          \afterpage{\do@hvFloat@doublePAGECaptionRight{#1}}%
        \else
          \hvfloat@typeout{do@hvFloat@doublePage:evenpage/onecolumn/noSwitchType -> (\thepage)}%
          \ifhv@forceLeft
            \hvfloat@typeout{do@hvFloat@doublePage:evenpage/onecolumn/noSwitchType/forceLeft -> (\thepage)}%
            \afterpage{\do@hvFloat@doublePageCaptionRight{#1}}%
          \else
            \afterpage{\afterpage{\do@hvFloat@doublePageCaptionRight{#1}}}%
          \fi
        \fi
      \fi
    \fi
  \fi
  \let\@fptop\hv@fptop
  \fi
 \endgroup% started at main macro \hvFloat
 \hvfloat@typeout{----------------------------- do@hvFloat@doublePage#1 ... Ende ----------------------}%
}
%
\def\do@hvFloat@doublePageCaptionRight#1{%  image on left and right page with caption on the right page ----------------------------------
  \setcounter{topnumber}{1}%
  \setcounter{dbltopnumber}{1}%
  \hvfloat@typeout{do@hvFloat@doublePageCaptionRight->start}%
  \do@hvFloat@doublePageCaptionRightObjectLeft{0pt}% 
  \do@hvFloat@doublePageCaptionRightObjectRight{#1}%
}
%
\def\do@hvFloat@doublePageCaptionRightObjectLeft#1{% left part of the object
  \begin{\hv@floatType*}[t]
    \hvfloat@typeout{>>>doublePage: begin with left side of the object \hv@floatObject}%
    \hv@set@noverticalSpace
    \hfuzz=\maxdimen
    \let\c@hv@tempCNTfigA\c@figure
    \let\c@hv@tempCNTtabA\c@table
    \expandafter\hvFloatSet\expandafter{\hv@save@setting}% 
    \hvfloat@typeout{Float position parameter is for left page: !t}%    
    \global\savebox{\hvObjectBox}{\ifhv@useOBox\usebox{\hvOBox}\else\hv@floatObject\fi}%
    \clipbox*{0 -\depth{} \hv@leftPageObjectWidth{} \height}{\usebox\hvObjectBox}%
    \ifx\hv@label\@empty
    \else
      \ifx\hv@floatType\hv@figure
        \global\refstepcounter{hv@tempCNTfigA}%
      \else
        \global\refstepcounter{hv@tempCNTtabA}%   before caption
      \fi
      \label{\hv@label}%
    \fi        
    \ifhv@global@sameHeight
      \hvfloat@typeout{text should be of same height of both pages}%
      \par\noindent\phantom{\parbox{\textwidth}{\caption*{\hv@longCap}}}%
%      \vspace{-2pt}%
    \fi
    \hv@reset@noverticalSpace
    \hvfloat@typeout{>>>doublePage: end with left side of the object \hv@floatObject}%
  \end{\hv@floatType*}%
}

\def\do@hvFloat@doublePageCaptionRightObjectRight#1{% right part of the object
  \begin{\hv@floatType*}[t]
    \hvfloat@typeout{>>>doublePage: begin with right side of the object \hv@floatObject}%
    \hv@set@noverticalSpace
    \hfuzz=\maxdimen
    \let\c@hv@tempCNTfigA\c@figure
    \let\c@hv@tempCNTtabA\c@table
    \expandafter\hvFloatSet\expandafter{\hv@save@setting}%    
    \global\savebox\hvObjectBox{\ifhv@useOBox\usebox{\hvOBox}\else\hv@floatObject\fi}%
    \hv@tempWidthA=\the\dimexpr1in+\oddsidemargin-\hvSet@bindCorrection\relax%   FÜR DTK
    \hspace*{-\hv@tempWidthA}%
    \hv@leftPageObjectWidth=\the\dimexpr\paperwidth-1in-\evensidemargin-\hvSet@bindCorrection\relax%  FÜR FTK
    \savebox\hv@rightBox{\clipbox*{\hv@leftPageObjectWidth{} -\depth{} \width{} \height{}}{\usebox\hvObjectBox}}%
    \hv@tempWidthB=\dimexpr\textwidth-\wd\hv@rightBox-\hvSet@bindCorrection+1in+\oddsidemargin-\hv@floatCapSep\relax
    \hvfloat@typeout{Height of right box: \the\ht\hv@rightBox}%
    \hvfloat@typeout{Depth of right box : \the\dp\hv@rightBox}%
       \ifdim\dp\hv@rightBox > \z@
           \raisebox{\depth}{\usebox\hv@rightBox}%
       \else
         \usebox\hv@rightBox
       \fi
       \c@hv@tempCNTfigB=\numexpr\c@figure-1\relax
       \c@hv@tempCNTtabB=\numexpr\c@table-1\relax
       \ifx\hv@label\@empty\else
         \ifx\hv@floatType\hv@figure
           \refstepcounter{hv@tempCNTfigB}%
         \else
           \refstepcounter{hv@tempCNTtabB}%   before caption
         \fi
         \label{\hv@label-2}%
       \fi        
       \ifx\hv@caption@format@temp\@empty\else
         \expandafter\captionsetup\expandafter{\hv@caption@format@temp}%
       \fi
       \c@figure=\numexpr\c@hv@tempCNTfigB-1\relax
       \c@table=\numexpr\c@hv@tempCNTtabB-1\relax
       \ifnum#1>\z@		% caption on the right
         \hvfloat@typeout{doublePage: capAngle=\hvSet@capAngle}%
         \ifnum\hvSet@capAngle > \z@
           \hspace{\hv@floatCapSep}%        
           \rlap{\rotatebox{\hvSet@capAngle}{\parbox[b]{\the\dimexpr\ht\hvObjectBox+\dp\hvObjectBox}{%
             \abovecaptionskip=0pt% local inside parbox
             \belowcaptionskip=0pt% local inside parbox
             \ifx\relax\hv@shortCap\relax
               \caption{\hv@longCap}%
             \else
               \caption[\hv@shortCap]{\hv@longCap}%
             \fi
           }}}%
           \ifx\hv@label\@empty\else\label{\hv@label-cap}\fi
         \else
           \hv@tempWidthB=\dimexpr\textwidth-\wd\hv@rightBox-\hvSet@bindCorrection+1in+\oddsidemargin-\hv@floatCapSep\relax
           \hspace{\hv@floatCapSep}%
           \rlap{\parbox[b][\dimexpr\ht\hv@rightBox+\dp\hv@rightBox][\hv@@capVPos]{\hv@tempWidthB}{%
             \abovecaptionskip=0pt % local inside parbox
             \belowcaptionskip=0pt % local inside parbox
             \ifx\hv@shortCap\@empty
               \caption{\hv@longCap}%
             \else
               \caption[\hv@shortCap]{\hv@longCap}%
             \fi
           }}%
           \ifx\hv@label\@empty\else\label{\hv@label-cap}\fi
         \fi
       \else % #1 = 0 caption below
         \ifx\relax\hv@shortCap\relax
           \caption{\hv@longCap}%
         \else
           \caption[\hv@shortCap]{\hv@longCap}%
         \fi
         \ifx\hv@label\@empty\else\label{\hv@label-cap}\fi
       \fi
    \vspace{0pt}%
    \hv@reset@noverticalSpace
    \hvfloat@typeout{>>>doublePage: end with right side of the object \hv@floatObject}%
  \end{\hv@floatType*}
}
