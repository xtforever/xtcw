const static char *syntax_xt="
Most X programs attempt to use the same names for command line options and arguments. 
All applications written with the X Toolkit Intrinsics automatically accept the following options:

-display display
	This option specifies the name of the X server to use.

-geometry geometry
	This option specifies the initial size and location of the window.

-bg color, -background color
	Either option specifies the color to use for the window background.

-bd color, -bordercolor color
	Either option specifies the color to use for the window border.

-bw number, -borderwidth number
	Either option specifies the width in pixels of the window border. 

-fg color, -foreground color
	Either option specifies the color to use for text or graphics. 

-fn font, -font font
	Either option specifies the font to use for displaying text. 

-iconic
	This option indicates that the user would prefer that the application's windows initially 
        not be visible as if the windows had be immediately iconified by the user. 
        Window managers may choose not to honor the application's request. 

-name
	This option specifies the name under which resources for the application should be found. 
        This option is useful in shell aliases to distinguish between invocations of an application, 
        without resorting to creating links to alter the executable file name.

-rv, -reverse
	Either option indicates that the program should simulate reverse video if possible, 
        often by swapping the foreground and background colors. 
        Not all programs honor this or implement it correctly. It is usually only used on monochrome displays. 

+rv
	This option indicates that the program should not simulate reverse video. 
        This is used to override any defaults since reverse video doesn't always work properly. 

-selectionTimeout
	This option specifies the timeout in milliseconds within which two communicating 
        applications must respond to one another for a selection request. 

-synchronous
	This option indicates that requests to the X server should be sent synchronously, 
        instead of asynchronously. Since Xlib normally buffers requests to the server, 
        errors do not necessarily get reported immediately after they occur. 
        This option turns off the buffering so that the application can be debugged. 
        It should never be used with a working program. 

-title string
	This option specifies the title to be used for this window. This information is sometimes 
        used by a window manager to provide some sort of header identifying the window. 

-xnllanguage language[_territory][.codeset]
	This option specifies the language, territory, and codeset for use in 
        resolving resource and other filenames. 

-xrm resourcestring
	This option specifies a resource name and value to override any defaults. 
        It is also very useful for setting resources that don't have explicit command line arguments.


*wclInitResFile <filenames>
*WclResFiles <filenames>
	List of files to be loaded into resource db.

*wclTraceResFiles
        Trace loading of files by Wcl to stderr.

*wcTrace
        Enable debug output to stderr.

-ResFile <files>
-rf <files>
	List of files to be loaded into resource db. ONLY command line

-Trace
-tr
-trtd
-trtx
-trrf 
        Trace loading of files by Wcl to stderr.

-Warnings
        Show warnings on stderr

WclResFiles			List of files to be loaded into resource db.
WclInitResFile                  ONLY set by command line.
WclTraceResFiles		Trace loading of files by Wcl to stderr.
WclErrorDatabaseFile		File with overrides for Wcl warning messages.
WclWidgetResourceFiles          Files with widget resources not reported by
				XtGetConstraintResourceList() nor by
				XtGetResourceList(). ** not yet implemented **
WclTemplateFiles		List of files with templates.
WclTraceTemplateDef		Trace definition of templates to stderr.
WclVerboseWarnings		Some warnings are supressed normally, but this
				resource causes Wcl to act like ANSI C and
				give somewhat informative but usually annoying
				messages.
WclDynamicLibs                  List of full pathnames of libraries which
				can be named using -lName shorthand in
				WcDynamicAction(), WcDynamicCallback(), and in
				the new (Wcl 2.02) callback and method syntax.
WclSlowTightNames		Default: False.  Causes WcFullNameToWidget()
				to use the same algorithm as used by the Athena
				string-to-widget converter for widget names
				starting with `*' - specifically, the reference
				widget is moved up in the widget tree, parent
				by parent, with a call to XtNameToWidget()
				at each parent, until the top of the tree is
				reached.  This is slower than the default
				algorithm which tries the reference widget,
				then the root of the widget tree containing
				the reference widget.
WclAppClassName                 If non-null, then this name will be used as
				the class name for all widgets created using
				WcCreateRoot().
