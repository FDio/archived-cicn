% A PARC specific refinement of:
%
% Stylish Article
% LaTeX Template
% Version 2.0 (13/4/14)
%
% Available from:
% http://www.LaTeXTemplates.com
%
% Original author:
% Mathias Legrand (legrand.mathias@gmail.com)
%
% License:
% CC BY-NC-SA 3.0 (http://creativecommons.org/licenses/by-nc-sa/3.0/)
% ---------------------------------------------------------------------
% Conference proceedings and article templates for
% personal open-archiving activities
% September 2012
% ---------------------------------------------------------------------

\NeedsTeXFormat{LaTeX2e}
\ProvidesClass{PARCOneColumn}[25/01/2012, v1.0]
\RequirePackage{ifthen}
\RequirePackage{calc}
\AtEndOfClass{\RequirePackage{microtype}}
\DeclareOption*{\PassOptionsToClass{\CurrentOption}{article}}
\ProcessOptions*
\LoadClass{article}
\RequirePackage{ifpdf}      % Needed to pick between latex and pdflatex

%----------------------------------------------------------------------
%	FONTS
%----------------------------------------------------------------------

%\RequirePackage{times}      % Loads the Times-Roman Fonts
\RequirePackage{palatino}    % Loads the Palatino Fonts
%\RequirePackage{mathptmx}   % Loads the Times-Roman Math Fonts

%----------------------------------------------------------------------
%	VARIOUS USEFUL PACKAGES
%----------------------------------------------------------------------

\RequirePackage[utf8]{inputenc}
\RequirePackage{amsmath,amsfonts,amssymb}
\RequirePackage{graphicx,xcolor}
\RequirePackage[english]{babel}
\RequirePackage{booktabs}
\RequirePackage{multicol}
\RequirePackage{tabularx}

%----------------------------------------------------------------------
%	MARGINS	
%----------------------------------------------------------------------
	  	  
\RequirePackage[left=2cm,%
                right=2cm,%
				top=2.25cm,%
				bottom=2.25cm,%
				headheight=11pt,%
				letterpaper]{geometry}%
				
%----------------------------------------------------------------------
%	FIGURES AND TABLES CAPTIONS
%----------------------------------------------------------------------

\RequirePackage[labelfont={bf,sf,small},%
                labelsep=period,%
                justification=raggedright]{caption}
\setlength{\abovecaptionskip}{0pt}
\setlength{\belowcaptionskip}{0pt}

%----------------------------------------------------------------------
%	PARC Colors
%----------------------------------------------------------------------

\definecolor{PARCBlue}{RGB}{32,84,105}
\definecolor{PARCDarkBlue}{RGB}{0,35,50}
\definecolor{PARCLightBlue}{RGB}{58,110,143}
\definecolor{PARCOrange}{RGB}{255,102,0}
\definecolor{PARCLightGray}{RGB}{213,213,213}
\definecolor{PARCDarkGray}{RGB}{58,58,58}

\definecolor{AbstractBackgroundColor}{RGB}{58,110,143}

\definecolor{SectionColor}{RGB}{0,0,90} % Color of the article title and sections
%				
%----------------------------------------------------------------------
%	PAGE HEADER
%----------------------------------------------------------------------
%
\RequirePackage{fancyhdr}  % Needed to define custom headers/footers
\headheight=13.6pt
\RequirePackage{lastpage}  % Number of pages in the document
\pagestyle{fancy}          % Enables the custom headers/footers
% Headers
\lhead{}%
\chead{}%
\rhead{\small\sffamily\bfseries\@PaperTitle\  --- \thepage/\pageref{LastPage}}
% Footers
\lfoot{\small\sffamily\bfseries\@LeftFooter}%
\cfoot{\small\sffamily\bfseries\@CenterFooter}%
\rfoot{\small\sffamily\bfseries\@RightFooter}%
\renewcommand{\headrulewidth}{0pt}% % No header rule
\renewcommand{\footrulewidth}{0pt}% % No footer rule

%----------------------------------------------------------------------
%	SECTION/SUBSECTION/PARAGRAPH SET-UP
%----------------------------------------------------------------------

\RequirePackage[explicit]{titlesec}
\titleformat{\section}
  {\color{PARCDarkBlue}\large\sffamily\bfseries}
  {}
  {0em}
  {\colorbox{PARCDarkBlue!10}{\parbox{\dimexpr\linewidth-2\fboxsep\relax}{\centering\arabic{section}. #1}}}
  []
\titleformat{name=\section,numberless}
  {\color{PARCDarkBlue}\large\sffamily\bfseries}
  {}
  {0em}
  {\colorbox{PARCDarkBlue!10}{\parbox{\dimexpr\linewidth-2\fboxsep\relax}{\centering#1}}}
  []  
\titleformat{\subsection}
  {\color{PARCDarkBlue}\sffamily\bfseries}
  {\thesubsection}
  {0.5em}
  {#1}
  []
\titleformat{\subsubsection}
  {\sffamily\small\bfseries}
  {\thesubsubsection}
  {0.5em}
  {#1}
  []    
\titleformat{\paragraph}[runin]
  {\sffamily\small\bfseries}
  {}
  {0em}
  {#1} 
\titlespacing*{\section}{0pc}{3ex \@plus4pt \@minus3pt}{5pt}
\titlespacing*{\subsection}{0pc}{2.5ex \@plus3pt \@minus2pt}{0pt}
\titlespacing*{\subsubsection}{0pc}{2ex \@plus2.5pt \@minus1.5pt}{0pt}
\titlespacing*{\paragraph}{0pc}{1.5ex \@plus2pt \@minus1pt}{10pt}

%----------------------------------------------------------------------
%	TABLEOFCONTENTS SET-UP
%----------------------------------------------------------------------
\newlength{\tocsep} 
\setlength\tocsep{2em} % Sets the indentation of the sections in the table of contents
\setcounter{tocdepth}{3} % Three levels in the table of contents section: sections, subsections and subsubsections

\usepackage{titletoc}
\contentsmargin{0cm}
\titlecontents{section}[\tocsep]
  {\addvspace{4pt}\small\bfseries\sffamily}
  {\contentslabel[\thecontentslabel]{\tocsep}}
  {}
  {\hfill\thecontentspage}
  []
\titlecontents{subsection}[\tocsep]
  {\addvspace{2pt}\sffamily}
  {\contentslabel[\thecontentslabel]{\tocsep}}
  {}
  {\ \titlerule*[.5pc]{.}\ \thecontentspage}
  []
\titlecontents*{subsubsection}[\tocsep]
  {\footnotesize\sffamily}
  {}
  {}
  {}
  [\ \textbullet\ ]  

%----------------------------------------------------------------------
%	MULTIPLE AUTHOR SET
%----------------------------------------------------------------------  
  
\newcount\@authcnt
\newcount\@tmpcnt\@tmpcnt\z@

\def\@affiliation{%
  \ifnum\@tmpcnt<\@authcnt
   \global\advance\@tmpcnt1
    \raggedright \csname @auth\romannumeral\the\@tmpcnt\endcsname\hfill\\%
   \let\next\@affiliation \vskip1pt
  \else
   \let\next\relax
  \fi
\next}
	 
\newcommand{\affiliation}[1]{%
    \global\advance\@authcnt1
    \expandafter\gdef\csname @auth\romannumeral\the\@authcnt\endcsname
    {#1}}

	
%----------------------------------------------------------------------
%	LIST CONTROL
%----------------------------------------------------------------------  

\RequirePackage{enumitem}
%\setlist{nolistsep} % Uncomment to remove spacing between items in lists (enumerate, itemize)

%----------------------------------------------------------------------
%	ABSTRACT+AUTHOR FRAME
%----------------------------------------------------------------------  

\newcommand{\PaperTitle}[2]{\def\@PaperTitle{#1}\def\@PaperSubtitle{#2}}
\newcommand{\Archive}[1]{\def\@Archive{#1}}
\newcommand{\Authors}[1]{\def\@Authors{#1}}
\newcommand{\JournalInfo}[1]{\def\@JournalInfo{#1}}
\newcommand{\Abstract}[1]{\def\@Abstract{#1}}
\newcommand{\Keywords}[1]{\def\@Keywords{#1}}
\newcommand{\Masthead}[1]{\def\@Masthead{#1}}

\newcommand{\LeftFooter}[1]{\def\@LeftFooter{#1}}
\newcommand{\CenterFooter}[1]{\def\@CenterFooter{#1}}
\newcommand{\RightFooter}[1]{\def\@RightFooter{#1}}

\LeftFooter{}
\CenterFooter{}
\RightFooter{}
%
% ---------------------------------------------------------------------
%
\newcommand{\NormalSansBold}[1]{\normalsize\sffamily\bfseries #1}
\newcommand{\SmallSansBold}[1]{\small\sffamily\bfseries #1}
\newcommand{\MastHeadText}[1]{\sffamily\fontsize{10}{12}\selectfont #1}
%
\newcommand{\MakePARCMastHead}[1]{
\setlength{\tabcolsep}{0pt}
\begin{tabular*}{\textwidth}{l @{\extracolsep{\fill}} p{0.618\textwidth} }
  \includegraphics[width=110pt]{parc_black_solid}&\vbox{\raggedleft\MastHeadText{#1}\vskip0.24mm}\\
\end{tabular*}
\vskip-5mm\hrule
}
%
\newcommand{\MakeTitle}[2]{%
{\raggedright\color{SectionColor}\sffamily\bfseries\fontsize{22}{25}\selectfont #1\par}%
{\raggedright\color{SectionColor}\sffamily\bfseries\fontsize{16}{24}\selectfont #2\par}%
}
%
\newcommand{\MakeAbstract}[2]{%
\parbox{\textwidth-6\fboxsep-2\fboxrule}{%
\ifx\@Keywords\@empty%
\sffamily\textbf{\abstractname}\\#1%
\else%
\sffamily\textbf{\abstractname}\\#1\\[4pt]%
\textbf{\keywordname}\\#2%
\fi%
}%
}%
%
\renewcommand{\@maketitle}{%
\centering{%
\thispagestyle{empty}%
\MakePARCMastHead{\@Masthead}%
\vskip30pt%
\MakeTitle{\@PaperTitle}{\@PaperSubtitle}%
\vskip10pt%
{\raggedright\color{SectionColor}\sffamily\fontsize{12}{16}\selectfont\@Authors\par}%
\vskip18pt%
\fcolorbox{SectionColor}{white}{%
\parbox{\textwidth-2\fboxsep-2\fboxrule}{\centering%
\colorbox{AbstractBackgroundColor!10}{%
\MakeAbstract{\@Abstract}{\@Keywords}
}%
\vskip4pt%
\begingroup%
\raggedright\sffamily\small%
\footnotesize\@affiliation\par%
\endgroup%%
}%
}%
\vskip25pt%
}%
}%
%----------------------------------------------------------------------
%	REFERENCES
%----------------------------------------------------------------------  

% Remove brackets from numbering in List of References
\renewcommand{\@biblabel}[1]{\bfseries\color{SectionColor}\textsuperscript{[#1]}}
%\setlength{\bibitemsep}{0cm}
\let\oldbibliography\thebibliography
\renewcommand{\thebibliography}[1]{%
\addcontentsline{toc}{section}{\refname}%
\oldbibliography{#1}%
\setlength\itemsep{0pt}}%
