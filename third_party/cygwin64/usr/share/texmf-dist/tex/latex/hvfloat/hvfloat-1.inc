%% Herbert Voss <hvoss@tug.org>
%% Copyright (C) 2023
%%
%% This program can be redistributed and/or modified under the terms
%% of the LaTeX Project Public License Distributed from CTAN archives
%% in directory macros/latex/base/lppl.txt.
%%
%%
%% ||1in+evenside --- |1in+oddside ---||
%
\def\do@hvFloat@doubleFULLPAGE#1{%  image on left and right page with caption before/below/right/after
  % #1-> 0/1 caption under/right
  \hvfloat@typeout{>>> do@hvFloat@doubleFULLPAGE: #1}%
  \expandafter\hvFloatSet\expandafter{\hv@save@setting}%    
  \ifx\hv@caption@format\@empty\else\expandafter\captionsetup\expandafter{\hv@caption@format}\fi
      \ifx\hv@subcaption@format\@empty\else
        \expandafter
           \captionsetup\expandafter[\expandafter s\expandafter u\expandafter b\expandafter]\expandafter
                             {\hv@subcaption@format}%
  \fi
  \ifx\hvSet@capPos\hv@After \global\hv@@capPos=1
  \else
    \ifx\hvSet@capPos\hv@Before  \global\hv@@capPos=0
    \else
      \global\hv@@capPos=2 % other caption type
  \fi\fi
  %\checkoddpage
  \global\savebox{\hvObjectBox}{\ifhv@useOBox\usebox{\hvOBox}\else\hv@floatObject\fi}%
  \global\hv@tempWidthA=\dimexpr-\oddsidemargin-1in-\parindent+\hvSet@bindCorrection%  the width of the right side offset
  \global\hv@tempWidthB=\dimexpr\ht\hvCaptionBox+\wd\hvObjectBox+2\hvSet@bindCorrection%
  \global\hv@leftPageObjectWidth=\dimexpr\paperwidth-\hvSet@bindCorrection%
%  \savebox\hv@leftBox{\clipbox*{0 0 \the\hv@leftPageObjectWidth{} \height}{\usebox\hvObjectBox}}%
%  \savebox\hv@rightBox{\clipbox*{\the\hv@leftPageObjectWidth{} \depth{} \width{} \height}{\usebox\hvObjectBox}}%
%  \expandafter\captionsetup\expandafter{\hv@caption@format}%
  \ifoddpage
    \hvfloat@typeout{do@hvFloat@doubleFULLPAGE: oddpage}%
    \ifcase\hv@@capPos % =0 Caption before
      \hvfloat@typeout{do@hvFloat@doubleFULLPAGE: oddpage/caption before (0)}%
      \if@twocolumn
        \hvfloat@typeout{do@hvFloat@doubleFULLPAGE: oddpage/caption before/twocolumn}%
        \if@firstcolumn
          \hvfloat@typeout{do@hvFloat@doubleFULLPAGE: oddpage/caption before/twocolumn/firstcolumn}%
          \ifhv@twoColumnCaption
            \hvfloat@typeout{do@hvFloat@doubleFULLPAGE: afterpage -> oddpage/caption before/twocolumn/firstcolumn/twoColumnCaption}%
            \set@Normal@Bottom@Caption*
            \afterpage{\do@hvFloat@doubleFULLPAGE@CaptionBefore}%
          \else
            \hvfloat@typeout{do@hvFloat@doubleFULLPAGE: afterpage -> oddpage/caption before/twocolumn/firstcolumn/oneColCaption}%
            \afterpage{\set@Normal@Bottom@Caption\do@hvFloat@doubleFULLPAGE@CaptionBefore}%
          \fi
        \else
          \hvfloat@typeout{do@hvFloat@doubleFULLPAGE: oddpage/caption before/twocolumn/secondcolumn}%
          \set@Normal@Bottom@Caption
          \do@hvFloat@doubleFULLPAGE@CaptionBefore
        \fi
      \else
        \hvfloat@typeout{do@hvFloat@doubleFULLPAGE: oddpage/caption before/onecolumn}%
        \set@Normal@Bottom@Caption
        \do@hvFloat@doubleFULLPAGE@CaptionBefore
      \fi
    \or  % =1 Caption after
      \hvfloat@typeout{do@hvFloat@doubleFULLPAGE: oddpage/caption after (1)}%
      \ifhv@twoColumnCaption
        \hvfloat@typeout{do@hvFloat@doubleFULLPAGE: oddpage/caption after/twoColCaption (1)}%
        \if@firstcolumn
          \hvfloat@typeout{do@hvFloat@doubleFULLPAGE: oddpage/caption after/firstcolumn (1)}%
          \afterpage{\do@hvFloat@doubleFULLPAGE@CaptionAfterTwoCol}%
        \else
          \hvfloat@typeout{do@hvFloat@doubleFULLPAGE: oddpage/caption after/secondcolumn (1)}%
          \do@hvFloat@doubleFULLPAGE@CaptionAfterTwoCol
        \fi
      \else
        \hvfloat@typeout{do@hvFloat@doubleFULLPAGE: oddpage/caption after/oneColCaption (1)}%
        \if@twocolumn
          \hvfloat@typeout{do@hvFloat@doubleFULLPAGE: oddpage/caption after/twoColCaption/twocolumn (1)}%
          \if@firstcolumn
            \hvfloat@typeout{do@hvFloat@doubleFULLPAGE: afterpage -> oddpage/caption after/twoColCaption/twocolumn/firstcolumn (1)}%
            \afterpage{\do@hvFloat@doubleFULLPAGE@CaptionAfter}%
          \else
            \hvfloat@typeout{do@hvFloat@doubleFULLPAGE: oddpage/caption after/twoColCaption/twocolumn/secondcolumn (1)}%
            \do@hvFloat@doubleFULLPAGE@CaptionAfter
          \fi
        \else
          \hvfloat@typeout{do@hvFloat@doubleFULLPAGE: oddpage/caption after/twoColCaption/onecolumn (1)}%
          \do@hvFloat@doubleFULLPAGE@CaptionAfter
        \fi
      \fi
    \else% \ifcase >1 all other Captions
      \if@twocolumn
        \if@firstcolumn
          \afterpage{\do@hvFloat@doubleFULLPAGE@CaptionOther{#1}}%
        \else
          \do@hvFloat@doubleFULLPAGE@CaptionOther{#1}%
        \fi
      \else % \if@twocolumn
        \do@hvFloat@doubleFULLPAGE@CaptionOther{#1}%
      \fi
    \fi%  \ifcase
  \else% we have an even page
    \hvfloat@typeout{do@hvFloat@doubleFULLPAGE: evenpage (1 -- \thepage)}%
    \ifcase\hv@@capPos%   Before
      \hvfloat@typeout{do@hvFloat@doubleFULLPAGE: evenpage/caption before (0 -- \thepage)}%
      \if@twocolumn
        \hvfloat@typeout{do@hvFloat@doubleFULLPAGE: evenpage/caption before/twocolumn (0 -- \thepage)}%
        \if@firstcolumn
          \hvfloat@typeout{do@hvFloat@doubleFULLPAGE: evenpage/caption before/twocolumn/firstcolumn (0 -- \thepage)}%
          \ifhv@twoColumnCaption
            \hvfloat@typeout{do@hvFloat@doubleFULLPAGE: afterpage^2->evenpage/caption before/twocolumn/firstcolumn/twocolCap (0 -- \thepage)}%
            \afterpage{\afterpage\set@Normal@Bottom@Caption*\afterpage{\do@hvFloat@doubleFULLPAGE@CaptionBefore{#1}}}%
          \else
            \hvfloat@typeout{do@hvFloat@doubleFULLPAGE: afterpage^2 -> evenpage/caption before/twocolumn/firstcolumn/oneColCap (0 -- \thepage)}%
            \afterpage{\afterpage{\afterpage{\set@Normal@Bottom@Caption\do@hvFloat@doubleFULLPAGE@CaptionBefore{#1}}}}%
          \fi
        \else
          \hvfloat@typeout{do@hvFloat@doubleFULLPAGE: afterpage^2->evenpage/caption before/twocolumn/secondcolumn (0 -- \thepage)}%
          \afterpage{\afterpage{\set@Normal@Bottom@Caption\do@hvFloat@doubleFULLPAGE@CaptionBefore{#1}}}%
        \fi
      \else
        \hvfloat@typeout{do@hvFloat@doubleFULLPAGE: afterpage -> evenpage/caption before/onecolumn (0 -- \thepage)}%
        \afterpage{\set@Normal@Bottom@Caption\do@hvFloat@doubleFULLPAGE@CaptionBefore}%
      \fi
    \or % capPos after
      \hvfloat@typeout{do@hvFloat@doubleFULLPAGE: evenpage/caption after (1 -- \thepage)}%
      \if@twocolumn
        \hvfloat@typeout{do@hvFloat@doubleFULLPAGE: evenpage/caption after/twocolumn (1 -- \thepage)}%
        \if@firstcolumn
          \hvfloat@typeout{do@hvFloat@doubleFULLPAGE: evenpage/caption after/twocolumn/firstcolumn (1 -- \thepage)}%
          \ifhv@twoColumnCaption
            \hvfloat@typeout{do@hvFloat@doubleFULLPAGE: afterpage -> evenpage/caption after/twocolumn/firstcolumn/twoColCaption (1 -- \thepage)}%
            \afterpage{\afterpage\afterpage{\do@hvFloat@doubleFULLPAGE@CaptionAfterTwoCol{#1}}}%
          \else
            \hvfloat@typeout{do@hvFloat@doubleFULLPAGE: afterpage -> evenpage/caption after/twocolumn/firstcolumn/oneColCaption (1 -- \thepage)}%
            \afterpage{\afterpage\afterpage{\do@hvFloat@doubleFULLPAGE@CaptionAfter{#1}}}%
          \fi
        \else
          \hvfloat@typeout{do@hvFloat@doubleFULLPAGE: evenpage/caption after/twocolumn/secondcolumn (1 -- \thepage)}%
          \ifhv@twoColumnCaption
            \hvfloat@typeout{do@hvFloat@doubleFULLPAGE: afterpage1 2 -> evenpage/caption after/twocolumn/secondcolumn/twoColCaption (1 -- \thepage)}%
            \afterpage{\afterpage{\do@hvFloat@doubleFULLPAGE@CaptionAfterTwoCol{#1}}}%
          \else
            \hvfloat@typeout{do@hvFloat@doubleFULLPAGE: afterpage1 2 -> evenpage/caption after/twocolumn/secondcolumn/oneColCaption (1 -- \thepage)}%
            \afterpage{\afterpage{\do@hvFloat@doubleFULLPAGE@CaptionAfter{#1}}}%
          \fi
        \fi
      \else
        \hvfloat@typeout{do@hvFloat@doubleFULLPAGE: afterpage -> evenpage/caption after/onecolumn (1 -- \thepage)}%
        \afterpage{\do@hvFloat@doubleFULLPAGE@CaptionAfter}%
      \fi
    \else % \ifcase Any other caption
      \if@twocolumn
        \if@firstcolumn
          \afterpage{\afterpage{\afterpage{\do@hvFloat@doubleFULLPAGE@CaptionOther{#1}}}}%
        \else
          \afterpage{\afterpage{\do@hvFloat@doubleFULLPAGE@CaptionOther{#1}}}%
        \fi
      \else
        \afterpage{\do@hvFloat@doubleFULLPAGE@CaptionOther{#1}}%
      \fi    
    \fi% \ifcase
  \fi% main ifoddpage
  \endgroup% started at main macro \hvFloat
}

\def\set@Normal@Bottom@Caption{\@ifnextchar*\set@Normal@Bottom@CaptionStar\set@Normal@Bottom@Caption@}
\def\set@Normal@Bottom@Caption@{%
    \begin{\hv@floatType}[!b]
      \expandafter\hvFloatSet\expandafter{\hv@save@setting}%    
      \ifhv@separatorLine\drawSepLine\fi
%      \expandafter\captionsetup\expandafter{\hv@caption@format}%
      \ifhv@onlyText
        \hv@longCap
      \else
        \ifx\hv@shortCap\@empty
          \captionof{\hv@floatType}{\hv@longCap}%
        \else
          \captionof{\hv@floatType}[\hv@shortCap]{\hv@longCap}%
        \fi
      \fi
      \ifx\hv@label\@empty\else\label{\hv@label-cap}\fi
    \end{\hv@floatType}%
}
\def\set@Normal@Bottom@CaptionStar*{%
    \begin{\hv@floatType*}[!b]
      \expandafter\hvFloatSet\expandafter{\hv@save@setting}%    
      \ifhv@separatorLine\drawSepLine\fi
%      \expandafter\captionsetup\expandafter{\hv@caption@format}%
      \ifhv@onlyText
        \hv@longCap
      \else
        \ifx\hv@shortCap\@empty
          \captionof{\hv@floatType}{\hv@longCap}%
        \else
          \captionof{\hv@floatType}[\hv@shortCap]{\hv@longCap}%
        \fi
      \fi
      \ifx\hv@label\@empty\else\label{\hv@label-cap}\fi
    \end{\hv@floatType*}%
}

\def\do@hvFloat@doubleFULLPAGE@CaptionBefore{%
  \afterpage{%
    \let\c@hv@tempCNTfigA\c@figure
    \let\c@hv@tempCNTtabA\c@table
    \hfuzz=\maxdimen
    \global\savebox{\hvObjectBox}{\ifhv@useOBox\usebox{\hvOBox}\else\hv@floatObject\fi}%
    \vspace*{\the\dimexpr-1in-\voffset-\topmargin-\headheight-\headsep-\baselineskip-\parskip+1.5\lineskip}%  no interlineskip
    \hspace*{\the\dimexpr-\evensidemargin-\parindent-1in}%
    \thispagestyle{empty}%
    \ifx\hv@floatType\hv@figure
      \global\refstepcounter{hv@tempCNTfigB}%
    \else
      \global\refstepcounter{hv@tempCNTtabB}%   before caption
    \fi
    \expandafter\label\expandafter{\hv@label}%
    \clipbox*{0 0 \the\hv@leftPageObjectWidth{} \height}{\usebox\hvObjectBox}%
    \afterpage{%
      \if@twocolumn\newpage\null\newpage\fi
      \global\savebox{\hvObjectBox}{\ifhv@useOBox\usebox{\hvOBox}\else\hv@floatObject\fi}%
      \thispagestyle{empty}%
      \vspace*{\the\dimexpr-1in-\voffset-\topmargin-\headheight-\headsep-\baselineskip-\parskip+1.5\lineskip}%    -0.5\paperheight+0.5\ht\hvObjectBox
      \hspace*{\hv@tempWidthA}%
      \global\savebox{\hvObjectBox}{\ifhv@useOBox\usebox{\hvOBox}\else\hv@floatObject\fi}%
      \clipbox*{\the\hv@leftPageObjectWidth{} \dp\hvObjectBox{} \wd\hvObjectBox{} \ht\hvObjectBox}{\usebox\hvObjectBox}%
      \ifx\hv@floatType\hv@figure
        \global\refstepcounter{hv@tempCNTfigA}%
      \else
        \global\refstepcounter{hv@tempCNTtabA}%   before caption
      \fi
      \expandafter\label\expandafter{\hv@label-2}%
      \newpage\if@twocolumn\null\newpage\fi
  }}%
}

\def\do@hvFloat@doubleFULLPAGE@CaptionAfter{%
  \afterpage{%
    \hfuzz=\maxdimen
    \let\c@hv@tempCNTfigA\c@figure
    \let\c@hv@tempCNTtabA\c@table
    \global\savebox{\hvObjectBox}{\ifhv@useOBox\usebox{\hvOBox}\else\hv@floatObject\fi}%
    \vspace*{\the\dimexpr-1in-\voffset-\topmargin-\headheight-\headsep-\baselineskip-\parskip+1.5\lineskip}%  no interlineskip
    \hspace*{\the\dimexpr-\evensidemargin-\parindent-1in}%
    \thispagestyle{empty}%
    \clipbox*{0 \the\dp\hvObjectBox{} \the\hv@leftPageObjectWidth{} \the\ht\hvObjectBox}{\usebox\hvObjectBox}%
    \ifx\hv@floatType\hv@figure
      \refstepcounter{hv@tempCNTfigA}%
    \else
      \refstepcounter{hv@tempCNTtabA}%   before caption
    \fi
    \ifx\hv@label\@empty\else\label{\hv@label}\fi            
      \newpage\if@twocolumn\null\newpage\fi
      \global\savebox{\hvObjectBox}{\ifhv@useOBox\usebox{\hvOBox}\else\hv@floatObject\fi}%
      \thispagestyle{empty}%
      \vspace*{\the\dimexpr-1in-\voffset-\topmargin-\headheight-\headsep-\baselineskip-\parskip+1.5\lineskip}%    -0.5\paperheight+0.5\ht\hvObjectBox
      \hspace*{\hv@tempWidthA}%
      \global\savebox{\hvObjectBox}{\ifhv@useOBox\usebox{\hvOBox}\else\hv@floatObject\fi}%
      \clipbox*{\the\hv@leftPageObjectWidth{} \dp\hvObjectBox{} \wd\hvObjectBox{} \ht\hvObjectBox}{\usebox\hvObjectBox}%
      \ifx\hv@floatType\hv@figure
        \refstepcounter{hv@tempCNTfigB}%
      \else
        \refstepcounter{hv@tempCNTtabB}%   before caption
      \fi
      \expandafter\label\expandafter{\hv@label-2}%
      \newpage\if@twocolumn\null\newpage\fi
      \begin{\hv@floatType}[!b]
        \ifhv@separatorLine\drawSepLine\fi
%        \expandafter\captionsetup\expandafter{\hv@caption@format}%
        \ifhv@onlyText
          \hv@longCap
        \else
          \ifx\hv@shortCap\@empty
            \captionof{\hv@floatType}{\hv@longCap}%
          \else
            \captionof{\hv@floatType}[\hv@shortCap]{\hv@longCap}%
          \fi
        \fi
        \ifx\hv@label\@empty\else\label{\hv@label-cap}\fi            
      \end{\hv@floatType}%
  }%
}

\def\do@hvFloat@doubleFULLPAGE@CaptionAfterTwoCol{%
  \hvfloat@typeout{do@hvFloat@doubleFULLPAGE@CaptionAfterTwoCol: start}  %
  \afterpage{%
    \let\c@hv@tempCNTfigA\c@figure
    \let\c@hv@tempCNTtabA\c@table
    \hvfloat@typeout{do@hvFloat@doubleFULLPAGE@CaptionAfterTwoCol: afterpage start}  %
    \global\savebox{\hvObjectBox}{\ifhv@useOBox\usebox{\hvOBox}\else\hv@floatObject\fi}%
    \vspace*{\the\dimexpr-1in-\voffset-\topmargin-\headheight-\headsep-\baselineskip-\parskip+1.5\lineskip}%  no interlineskip
    \hspace*{\the\dimexpr-\evensidemargin-\parindent-1in}%
    \thispagestyle{empty}%
    \ifx\hv@floatType\hv@figure
      \refstepcounter{hv@tempCNTfigA}%
    \else
      \refstepcounter{hv@tempCNTtabA}%   before caption
    \fi
    \ifx\hv@label\@empty\else\label{\hv@label}\fi
    \clipbox*{0 0 \the\hv@leftPageObjectWidth{} \height}{\usebox\hvObjectBox}%
    \newpage\if@twocolumn\null\newpage\fi    
    \hvfloat@typeout{do@hvFloat@doubleFULLPAGE@CaptionAfterTwoCol: insert newpage}  %
      \global\savebox{\hvObjectBox}{\ifhv@useOBox\usebox{\hvOBox}\else\hv@floatObject\fi}%
      \thispagestyle{empty}%
      \vspace*{\the\dimexpr-1in-\voffset-\topmargin-\headheight-\headsep-\baselineskip-\parskip+1.5\lineskip}%    -0.5\paperheight+0.5\ht\hvObjectBox
      \hspace*{\hv@tempWidthA}%
      \global\savebox{\hvObjectBox}{\ifhv@useOBox\usebox{\hvOBox}\else\hv@floatObject\fi}%
      \clipbox*{\the\hv@leftPageObjectWidth{} \dp\hvObjectBox{} \wd\hvObjectBox{} \ht\hvObjectBox}{\usebox\hvObjectBox}%
      \ifx\hv@floatType\hv@figure
        \refstepcounter{hv@tempCNTfigB}%
      \else
        \refstepcounter{hv@tempCNTtabB}%   before caption
      \fi
      \ifx\hv@label\@empty\else\label{\hv@label-2}\fi
      \newpage\if@twocolumn\null\newpage\fi
      \begin{\hv@floatType*}[!b]
        \hv@twoColumnCaptiontrue
        \ifhv@separatorLine\drawSepLine\fi
%        \expandafter\captionsetup\expandafter{\hv@caption@format}%
        \ifhv@onlyText
          \hv@longCap
        \else
          \ifx\hv@shortCap\@empty
            \captionof{\hv@floatType}{\hv@longCap}%
          \else
            \captionof{\hv@floatType}[\hv@shortCap]{\hv@longCap}%
          \fi
        \fi
        \ifx\hv@label\@empty\else\label{\hv@label-cap}\fi
      \end{\hv@floatType*}%
  }%
  \hvfloat@typeout{do@hvFloat@doubleFULLPAGE@CaptionAfterTwoCol: afterpage done}  %
}

\def\do@hvFloat@doubleFULLPAGE@CaptionOther#1{%
  \afterpage{%
    \let\c@hv@tempCNTfigA\c@figure
    \let\c@hv@tempCNTtabA\c@table
    \vspace*{\the\dimexpr-1in-\voffset-\topmargin-\headheight-\headsep-\baselineskip-\parskip+1.5\lineskip}%    -0.5\paperheight+0.5\ht\hvObjectBox
    \hspace*{\the\dimexpr-\evensidemargin-\parindent-1in}%
    \global\savebox{\hvObjectBox}{\ifhv@useOBox\usebox{\hvOBox}\else\hv@floatObject\fi}%
    \thispagestyle{empty}%
    \hfuzz=\maxdimen
    \global\savebox{\hvObjectBox}{\ifhv@useOBox\usebox{\hvOBox}\else\hv@floatObject\fi}%
    \clipbox*{0 \depth{} \hv@leftPageObjectWidth{} \height}{\usebox\hvObjectBox}%
    \ifx\hv@floatType\hv@figure
      \refstepcounter{hv@tempCNTfigA}%
    \else
      \refstepcounter{hv@tempCNTtabA}%   before caption
    \fi
    \label{\hv@label}%
    \afterpage{%
      \if@twocolumn\newpage\if@firstcolumn\else\null\newpage\fi\fi
      \global\savebox{\hvObjectBox}{\ifhv@useOBox\usebox{\hvOBox}\else\hv@floatObject\fi}%
      \thispagestyle{empty}%
      \vspace*{\the\dimexpr-1in-\voffset-\topmargin-\headheight-\headsep-\baselineskip-\parskip+1.5\lineskip}%    -0.5\paperheight+0.5\ht\hvObjectBox
      \hspace*{\hv@tempWidthA}%
      \clipbox*{\hv@leftPageObjectWidth{} \dp\hvObjectBox{} \wd\hvObjectBox{} \ht\hvObjectBox}{\usebox\hvObjectBox}%
      \ifx\hv@floatType\hv@figure
        \refstepcounter{hv@tempCNTfigB}%
      \else
        \refstepcounter{hv@tempCNTtabB}%   before caption
      \fi
      \expandafter\label\expandafter{\hv@label-2}%
      \savebox\hvCaptionBox{\parbox{0.9\ht\hvObjectBox}{\captionof*{\hv@floatType}{\hv@longCap}}}%
      \ifnum#1 > 0\relax    % rotation with 90° 
%        \setlength\hv@tempWidthB{\dimexpr\ht\hvCaptionBox+\wd\hvObjectBox+2\hvSet@bindCorrection}%
        \ifdim\hv@tempWidthB < 2\paperwidth        
          \rotatebox[origin=lb]{90}{\makebox[\paperheight][c]{\parbox{0.8\ht\hvObjectBox}{%
%            \expandafter\captionsetup\expandafter{\hv@caption@format}%
            \ifhv@onlyText
              \hv@longCap
            \else
              \ifx\hv@shortCap\@empty
                \captionof{\hv@floatType}{\hv@longCap}%
              \else
                \captionof{\hv@floatType}[\hv@shortCap]{\hv@longCap}%
              \fi
            \fi
          }}}% rotatebox
          \ifx\hv@label\@empty\else\label{\hv@label-cap}\fi
        \else% ifdim: no space left on page
          \put(-2\ht\hvCaptionBox,0.5\ht\hvObjectBox){\makebox(0,0){\rotatebox{90}{\minipage{\textwidth}\centering
            \parbox{0.8\textwidth}{%
              \ifhv@onlyText
	        \hv@longCap
  	      \else
                \ifx\hv@shortCap\@empty
                  \captionof{\hv@floatType}{\hv@longCap}%
                \else
                  \captionof{\hv@floatType}[\hv@shortCap]{\hv@longCap}%
                \fi
              \fi
            }%
            \ifx\hv@label\@empty\else\label{\hv@label-cap}\fi
          \endminipage}}}%
        \fi
      \else% ifnum, caption not rotated, under or over the right page
        \hv@tempWidthA=\the\dimexpr\paperheight-\ht\hvObjectBox\relax
        \ifdim\hv@tempWidthA > \ht\hvCaptionBox 
        \else 
          \put(\the\dimexpr1.5\paperwidth-\wd\hvObjectBox-\hvSet@bindCorrection,\the\dimexpr\ht\hvCaptionBox+\abovecaptionskip+\belowcaptionskip){\makebox[0pt][c]{\parbox{\textwidth}{%%   
%            \expandafter\captionsetup\expandafter{\hv@caption@format}%
  	    \ifhv@onlyText
	      \hv@longCap
  	    \else
              \ifx\hv@shortCap\@empty
                \captionof{\hv@floatType}{\hv@longCap}%
              \else
                \captionof{\hv@floatType}[\hv@shortCap]{\hv@longCap}%
              \fi
            \fi
          }}}%
          \ifx\hv@label\@empty\else\label{\hv@label-cap}\fi
        \fi
      \fi% end \ifnum#1>0
      \newpage\if@twocolumn\null\newpage\fi
    }%
  }%
}


\def\setBottomCaption{%
  \ifhv@twoColumnCaption
    \begin{\hv@floatType*}[!b]%
  \else
    \begin{\hv@floatType}[!b]%
  \fi
  \expandafter\hvFloatSet\expandafter{\hv@save@setting}%    
  \ifhv@separatorLine\drawSepLine\fi
  \par
  \usebox\hvCaptionBox
  \ifhv@twoColumnCaption
    \end{\hv@floatType*}%
  \else
    \end{\hv@floatType}%
  \fi
}

\def\setPageObject{%
  \hvfloat@typeout{>>>> setPageObject/start environment ...}%
  \afterpage{\checkoddpage}%
  \ifhv@star
    \begin{\hv@floatType*}[p]%
  \else
    \begin{\hv@floatType}[p]%
  \fi
  \hv@tempcnt=\@floatpenalty% suppress "float too big" message
  \hfuzz=\maxdimen
  \expandafter\hvFloatSet\expandafter{\hv@save@setting}%    
    \ifhv@FULLPAGE
    \hvfloat@typeout{>>>> setPageObject/FULLPAGE}%
    \@floatpenalty=\z@%   no message that float too large
    \vspace*{\the\dimexpr-1in-\voffset-\topmargin-\headheight-\headsep}%-0.5\baselineskip%
    \if@twoside
      \hvfloat@typeout{>>>> setPageObject/FULLPAGE/twoside}%
      \ifoddpage
        \hvfloat@typeout{>>>> setPageObject/FULLPAGE/twoside/oddpage (\thepage)}%
        \hspace*{\the\dimexpr-\oddsidemargin-\parindent-1in}%
      \else
        \hvfloat@typeout{>>>> setPageObject/FULLPAGE/twoside/evenpage (\thepage)}%
        \hspace*{\the\dimexpr-\evensidemargin-\parindent-1in}%
      \fi
    \else
      \hvfloat@typeout{>>>> setPageObject/FULLPAGE/oneside}%
      \hspace*{\the\dimexpr-\oddsidemargin-\parindent-1in}%
    \fi
    \hvfloat@typeout{>>>> setPageObject/FULLPAGE/set pagestyle empty}%
    \AtBeginShipoutNext{\thispagestyle{empty}}%  to prevent heading/footer
%    \afterpage{\AddToHookNext{shipout/after}{\thispagestyle{empty}}}%
    \usebox\hvObjectBox
  \else
    \hvfloat@typeout{>>>> setPageObject/NO FULLPAGE}%
    \usebox\hvObjectBox
  \fi
  \hvfloat@typeout{>>>> setPageObject/stop environment}%
  \ifhv@star
    \end{\hv@floatType*}%
  \else
    \end{\hv@floatType}%
  \fi
  \@floatpenalty=\hv@tempcnt%   restore
}
