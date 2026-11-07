%%
%% This is file `phfextendedabstract.cls',
%% generated with the docstrip utility.
%%
%% The original source files were:
%%
%% phfextendedabstract.dtx  (with options: `package')
%% 
%% This is a generated file.
%% 
%% Copyright (C) 2021 by Philippe Faist, philippe.faist@bluewin.ch
%% 
%% This file may be distributed and/or modified under the conditions of
%% the LaTeX Project Public License, either version 1.3 of this license
%% or (at your option) any later version.  The latest version of this
%% license is in:
%% 
%%    http://www.latex-project.org/lppl.txt
%% 
%% and version 1.3 or later is part of all distributions of LaTeX version
%% 2005/12/01 or later.
%% 
\NeedsTeXFormat{LaTeX2e}[2005/12/01]
\ProvidesClass{phfextendedabstract}
    [2021/09/08 v1.0 phfextendedabstract class]
\RequirePackage{kvoptions}
\SetupKeyvalOptions{%
  family=phfea,%
  prefix=phfeaopt@%
}
\DeclareStringOption[]{papertype}
\DeclareStringOption[11pt]{ptsize}
\DeclareBoolOption[true]{sectiondecorations}
\DeclareBoolOption[true]{paragraphdecorations}
\DeclareVoidOption{noheadingdecorations}{%
  \phfeaopt@sectiondecorationsfalse
  \phfeaopt@paragraphdecorationsfalse
}
\DeclareBoolOption[true]{loadtheorems}
\DeclareBoolOption[true]{sansstyle}
\DeclareStringOption[1]{compressverticalspacing}
\DeclareBoolOption[true]{usehyperref}
\DeclareStringOption[extended]{pkgset}
\ProcessKeyvalOptions*
\providecommand\phfea@revtexopts{%
  aps,pra,%
  notitlepage,reprint,%
  onecolumn,tightenlines,%
  superscriptaddress,%
  nofootinbib%
}
\PassOptionsToClass{%
  \phfea@revtexopts,%
  \phfeaopt@ptsize,%
  \phfeaopt@papertype,%
}{revtex4-2}
\LoadClass{revtex4-2}
\RequirePackage{xparse}
\PassOptionsToPackage{%
  preset=reset,%
  pkgset=\phfeaopt@pkgset,%
  \ifphfeaopt@usehyperref
    hyperrefdefs={defer,noemail},%
  \fi
}{phfnote}
\RequirePackage{phfnote}
\RequirePackage{geometry}
\geometry{hmargin=0.75in,vmargin=0.75in,marginparwidth=0.5in,marginparsep=0.125in}
\ifphfeaopt@sansstyle
  \def\phfeaHeadingStyle{\sffamily}
\else
  \def\phfeaHeadingStyle{}
\fi
\def\phfeaTitleStyle{\phfeaHeadingStyle\Large}
\def\frontmatter@title@format{\phfeaTitleStyle\centering\parskip\z@skip}
\edef\phfeaVerticalSpacingCompressionFactor{\phfeaopt@compressverticalspacing}
\def\phfea@scaleglue#1#2{% {factor}{glueexpr}
  \glueexpr#2*\numexpr\dimexpr#1pt\relax\relax/65536\relax
}
\def\phfea@scalegluedpt#1#2{% {factor given as dimexpr in pt}{glueexpr}
  \glueexpr#2*\numexpr#1\relax/65536\relax
}
\def\phfeaDisplayVerticalSpacingFactorWeight{.5}
\def\phfea@adjustskipweighted#1#2{%
  #1=\glueexpr
    \phfea@scalegluedpt{%
      \dimexpr 1\p@ - #2\p@\relax
    }{#1}%
    +
    \phfea@scaleglue{%
      #2%
    }{%
      \phfea@scaleglue{%
        \phfeaVerticalSpacingCompressionFactor
      }{%
        #1
      }%
    }%
    \relax
}
\AtBeginDocument{%
  \phfea@adjustskipweighted\abovedisplayskip\phfeaDisplayVerticalSpacingFactorWeight
  \phfea@adjustskipweighted\belowdisplayskip\phfeaDisplayVerticalSpacingFactorWeight
  \phfea@adjustskipweighted\abovedisplayshortskip\phfeaDisplayVerticalSpacingFactorWeight
  \phfea@adjustskipweighted\belowdisplayshortskip\phfeaDisplayVerticalSpacingFactorWeight
}
\def\phfeaParskipVerticalSpacingFactorWeight{1}
\AtBeginDocument{%
  \phfea@adjustskipweighted\parskip\phfeaParskipVerticalSpacingFactorWeight
}
\setcounter{secnumdepth}{0}
\setcounter{tocdepth}{1}
\def\phfeaSectionBeforeSkip{1.5ex plus 0.8ex minus 0.25ex}
\def\phfeaSectionAfterHSkip{1em plus 0.2em}
\def\phfeaSectionStyle{\normalfont\normalsize\phfeaHeadingStyle\bfseries}
\def\phfeaSectionFormatHeading#1{#1}
\def\phfeaSectionDecorationSymbol{%
  \raisebox{0.2ex}{{\notesmaller[0.4]{\ensuremath{\blacksquare}}}}}
\ifphfeaopt@sectiondecorations
  \def\phfeaSectionDecoration#1{%
    \makebox[\z@][r]{{#1}\hspace*{1.5ex}}%
  }
\else
  \def\phfeaSectionDecoration#1{}
\fi
\def\phfeaParagraphBeforeSkip{0.6ex plus 0.4ex minus 0.1ex}
\def\phfeaParagraphAfterHSkip{0.75em plus 0.15em}
\def\phfeaParagraphStyle{\normalfont\normalsize\phfeaHeadingStyle\small}
\def\phfeaParagraphFormatHeading#1{#1}
\def\phfeaParagraphDecorationSymbol{%
  \raisebox{0.2ex}{{\notesmaller[0.6]{\ensuremath{\triangleright}}}}}
\ifphfeaopt@paragraphdecorations
  \def\phfeaParagraphDecoration#1{%
    \makebox[\z@][r]{{#1}\hspace*{1ex}}%
  }
\else
  \def\phfeaParagraphDecoration#1{}
\fi
\def\phfea@ss@levelname#1{\ifcase#1\or section\or paragraph\fi}
\def\phfea@ss@levelName#1{\ifcase#1\or Section\or Paragraph\fi}
\def\phfea@ss@get#1#2{% <levelno><MacroPostfixName>
  \csname phfea\phfea@ss@levelName{#1}#2\endcsname}
\def\phfea@ss@getnoexpand#1#2{% <levelno><MacroPostfixName>
  \expandafter\noexpand\csname phfea\phfea@ss@levelName{#1}#2\endcsname}
\newtoks\phfea@ss@decorationtoks
\newtoks\phfea@ss@alttitle
\newtoks\phfea@ss@title
\NewDocumentCommand{\phfea@startsection}{m s t! d<> o m}{%
  \phfea@ss@decorationtoks={#4}%
  \phfea@ss@alttitle={#5}%
  \phfea@ss@title={#6}%
  \IfNoValueF{#4}{%
    \IfBooleanT{#3}{%
      \PackageWarning{phfextendedabstract}{section/paragraph: `!'
        modifier ignored when custom decoration `<...>' is specified}%
    }%
  }%
  \edef\x{%
    \noexpand\@startsection{\phfea@ss@levelname{#1}}%
      {#1}%
      {\z@}%
      {%
        \noexpand\phfea@scaleglue{\noexpand\phfeaVerticalSpacingCompressionFactor}{%
          \noexpand\glueexpr\phfea@ss@getnoexpand{#1}{BeforeSkip}\noexpand\relax
        }%
      }%
      {\if\relax\detokenize{#6}\relax
          \noexpand\z@
        \else
          -\noexpand\glueexpr\phfea@ss@getnoexpand{#1}{AfterHSkip}\noexpand\relax
        \fi}%
      {%
        \IfNoValueTF{#4}{%
          \IfBooleanTF{#3}{% with "!", no custom decoration --> nothing
          }{% no "!", no custom decoration
            \phfea@ss@getnoexpand{#1}{Decoration}%
            \phfea@ss@getnoexpand{#1}{DecorationSymbol}%
          }%
        }{% with custom decoration (ignores "!" argument)
          \phfea@ss@getnoexpand{#1}{Decoration}%
          {\the\phfea@ss@decorationtoks}%
        }%
        \phfea@ss@getnoexpand{#1}{Style}%
      }%
      \IfBooleanT{#2}{*}%
      \IfValueTF{#4}{[\the\phfea@ss@alttitle]}{%
        \IfBooleanTF{#2}{[]}{[\the\phfea@ss@title]}%
      }%
      {%
        \if\relax\detokenize{#6}\relax\else
          \noexpand\texorpdfstring{%
            \phfea@ss@getnoexpand{#1}{FormatHeading}{\the\phfea@ss@title}%
          }{%
            \the\phfea@ss@title
          }%
        \fi
      }%
  }%
  %\message{**** EMITTING @startsection: |\detokenize\expandafter{\x}| ***}%
  \x
}
\def\section{\phfea@startsection{1}}
\def\paragraph{\phfea@startsection{2}}
\def\phfea@nosectioncmd#1{%
  \ClassError{phfextendedabstract}{%
    %
    There is no `\string#1' command in `phfextendedabstract'
    documents.  You can only use `\string\section' and
    `\string\paragraph'.  If you find yourself needing additional
    sectioning levels, it might be that your extended abstract is too
    detailed and you should stick to a higher level description of
    your results.  If you do need additional section levels, it is
    likely that you will be better off with a different document
    class.  If you'd like to stick with `phfextendedabstract' and you
    really know what you're doing, then you could redefine the
    sectioning commands as necessary based on `\string\@startsection'
    as is done in standard LaTeX classes.  Good luck!
    %
  }{}%
}
\def\phfea@nosectioncmd@def#1{%
  \def#1{\phfea@nosectioncmd#1}%
}
\phfea@nosectioncmd@def\part
\phfea@nosectioncmd@def\chapter
\phfea@nosectioncmd@def\subsection
\phfea@nosectioncmd@def\subsubsection
\phfea@nosectioncmd@def\subparagraph
\def\phfeaListsVerticalSkip{0.6ex plus 0.4ex minus 0.1ex}
\def\phfeaListsItemSep{0.3ex plus 0.15ex minus 0.1ex}
\def\phfeaListsParSep{0.7\parskip}
\def\phfea@setup@enumitem{%
  \setlist{%
    itemsep={%
      \phfea@scaleglue{%
        \phfeaVerticalSpacingCompressionFactor}{\phfeaListsItemSep}},
    parsep={%
      \phfea@scaleglue{%
        \phfeaVerticalSpacingCompressionFactor}{\phfeaListsParSep}},
    topsep={%
      \phfea@scaleglue{%
        \phfeaVerticalSpacingCompressionFactor}{\phfeaListsVerticalSkip}},
  }%
  \newlist{enumerate*}{enumerate*}{1}
  \setlist[enumerate*]{
    label={(\roman*)},
    before={},
    itemjoin={{ }},
    itemjoin*={{ and }}
  }%
}
\@ifpackageloaded{enumitem}{\phfea@setup@enumitem}{}
\def\phfeaDefineTheoremStyle{%
  \newtheoremstyle{phfextendedabstractthm}%
    {\phfea@scaleglue{%
        \phfeaVerticalSpacingCompressionFactor}{\phfeaListsVerticalSkip}}%
    {\phfea@scaleglue{%
        \phfeaVerticalSpacingCompressionFactor}{\phfeaListsVerticalSkip}}%
    {\itshape}%
    {\z@}%
    {\bfseries\itshape}%
    {:}%
    {0.8em}%
    {\thmname{##1\thmnumber{ ##2}\thmnote{ (##3)}}}%
}
\ifdefined\newtheoremstyle
  \phfeaDefineTheoremStyle
\fi
\ifphfeaopt@loadtheorems
  \ifdefined\newtheoremstyle\else
    \ClassError{phfextendedabstract}{%
      %
      Impossible to load theorems (`loadtheorems=true') because there
      is no \string\newtheoremstyle\space command that was defined.
      Consider setting `pkgset=...' so that a theroems-related package
      (e.g., amsthm) is loaded!  (e.g. `pkgset=minimal', `pkgset=rich'
      or `pkgset=extended').  Alternatively, set the class option
      `loadtheorems=false' and you can then manually define any
      theorem environments you'd like using your favorite pacakges.
      %
    }{}
  \fi
  \PassOptionsToPackage{proofref=false,theoremstyle=phfextendedabstractthm}{phfthm}
  \RequirePackage{phfthm}
\fi
%% \let\rtxapsbibsection\bibsection
%% \def\bibsection{%
%%   \par\section{\refname}\leavevmode\par\addvspace{4pt}\relax
%% %%  \par\noindent\rule{6em}{.4pt}\par\addvspace{6pt}\relax
%% }
\endinput
%%
%% End of file `phfextendedabstract.cls'.
