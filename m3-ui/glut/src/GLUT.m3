(*******************************************************************************
 * This file was automatically generated by SWIG (http://www.swig.org/).
 * Version 1.3.36
 *
 * Do not make changes to this file unless you know what you are doing --
 * modify the SWIG interface file instead.
 *******************************************************************************)

UNSAFE MODULE GLUT;

IMPORT GLUTRaw;
IMPORT M3toC;
IMPORT Ctypes AS C;


IMPORT GL;

PROCEDURE Init (VALUE pargc: CARDINAL; READONLY argv: ARRAY OF TEXT; ) =
  VAR
    arg1: C.int;
    arg2: C.char_star;
  BEGIN
    arg1 := pargc;
    arg2 := M3toC.SharedTtoS(argv[0]);
    GLUTRaw.Init(arg1, arg2);
    M3toC.FreeSharedS(argv[0], arg2);
  END Init;

PROCEDURE InitWindowPosition (x, y: INTEGER; ) =
  BEGIN
    GLUTRaw.InitWindowPosition(x, y);
  END InitWindowPosition;

PROCEDURE InitWindowSize (width, height: INTEGER; ) =
  BEGIN
    GLUTRaw.InitWindowSize(width, height);
  END InitWindowSize;

PROCEDURE InitDisplayMode (displayMode: CARDINAL; ) =
  BEGIN
    GLUTRaw.InitDisplayMode(displayMode);
  END InitDisplayMode;

PROCEDURE InitDisplayString (displayMode: TEXT; ) =
  VAR arg1: C.char_star;
  BEGIN
    arg1 := M3toC.SharedTtoS(displayMode);
    GLUTRaw.InitDisplayString(arg1);
    M3toC.FreeSharedS(displayMode, arg1);
  END InitDisplayString;

PROCEDURE MainLoop () =
  BEGIN
    GLUTRaw.MainLoop();
  END MainLoop;

PROCEDURE CreateWindow (title: TEXT; ): INTEGER =
  VAR
    arg1  : C.char_star;
    result: INTEGER;
  BEGIN
    arg1 := M3toC.SharedTtoS(title);
    result := GLUTRaw.CreateWindow(arg1);
    M3toC.FreeSharedS(title, arg1);
    RETURN result;
  END CreateWindow;

PROCEDURE CreateSubWindow (window, x, y, width, height: INTEGER; ):
  INTEGER =
  BEGIN
    RETURN GLUTRaw.CreateSubWindow(window, x, y, width, height);
  END CreateSubWindow;

PROCEDURE DestroyWindow (window: INTEGER; ) =
  BEGIN
    GLUTRaw.DestroyWindow(window);
  END DestroyWindow;

PROCEDURE SetWindow (window: INTEGER; ) =
  BEGIN
    GLUTRaw.SetWindow(window);
  END SetWindow;

PROCEDURE GetWindow (): INTEGER =
  BEGIN
    RETURN GLUTRaw.GetWindow();
  END GetWindow;

PROCEDURE SetWindowTitle (title: TEXT; ) =
  VAR arg1: C.char_star;
  BEGIN
    arg1 := M3toC.SharedTtoS(title);
    GLUTRaw.SetWindowTitle(arg1);
    M3toC.FreeSharedS(title, arg1);
  END SetWindowTitle;

PROCEDURE SetIconTitle (title: TEXT; ) =
  VAR arg1: C.char_star;
  BEGIN
    arg1 := M3toC.SharedTtoS(title);
    GLUTRaw.SetIconTitle(arg1);
    M3toC.FreeSharedS(title, arg1);
  END SetIconTitle;

PROCEDURE ReshapeWindow (width, height: INTEGER; ) =
  BEGIN
    GLUTRaw.ReshapeWindow(width, height);
  END ReshapeWindow;

PROCEDURE PositionWindow (x, y: INTEGER; ) =
  BEGIN
    GLUTRaw.PositionWindow(x, y);
  END PositionWindow;

PROCEDURE ShowWindow () =
  BEGIN
    GLUTRaw.ShowWindow();
  END ShowWindow;

PROCEDURE HideWindow () =
  BEGIN
    GLUTRaw.HideWindow();
  END HideWindow;

PROCEDURE IconifyWindow () =
  BEGIN
    GLUTRaw.IconifyWindow();
  END IconifyWindow;

PROCEDURE PushWindow () =
  BEGIN
    GLUTRaw.PushWindow();
  END PushWindow;

PROCEDURE PopWindow () =
  BEGIN
    GLUTRaw.PopWindow();
  END PopWindow;

PROCEDURE FullScreen () =
  BEGIN
    GLUTRaw.FullScreen();
  END FullScreen;

PROCEDURE PostWindowRedisplay (window: INTEGER; ) =
  BEGIN
    GLUTRaw.PostWindowRedisplay(window);
  END PostWindowRedisplay;

PROCEDURE PostRedisplay () =
  BEGIN
    GLUTRaw.PostRedisplay();
  END PostRedisplay;

PROCEDURE SwapBuffers () =
  BEGIN
    GLUTRaw.SwapBuffers();
  END SwapBuffers;

PROCEDURE WarpPointer (x, y: INTEGER; ) =
  BEGIN
    GLUTRaw.WarpPointer(x, y);
  END WarpPointer;

PROCEDURE SetCursor (cursor: INTEGER; ) =
  BEGIN
    GLUTRaw.SetCursor(cursor);
  END SetCursor;

PROCEDURE EstablishOverlay () =
  BEGIN
    GLUTRaw.EstablishOverlay();
  END EstablishOverlay;

PROCEDURE RemoveOverlay () =
  BEGIN
    GLUTRaw.RemoveOverlay();
  END RemoveOverlay;

PROCEDURE UseLayer (layer: GL.GLenum; ) =
  BEGIN
    GLUTRaw.UseLayer(layer);
  END UseLayer;

PROCEDURE PostOverlayRedisplay () =
  BEGIN
    GLUTRaw.PostOverlayRedisplay();
  END PostOverlayRedisplay;

PROCEDURE PostWindowOverlayRedisplay (window: INTEGER; ) =
  BEGIN
    GLUTRaw.PostWindowOverlayRedisplay(window);
  END PostWindowOverlayRedisplay;

PROCEDURE ShowOverlay () =
  BEGIN
    GLUTRaw.ShowOverlay();
  END ShowOverlay;

PROCEDURE HideOverlay () =
  BEGIN
    GLUTRaw.HideOverlay();
  END HideOverlay;

PROCEDURE DestroyMenu (menu: INTEGER; ) =
  BEGIN
    GLUTRaw.DestroyMenu(menu);
  END DestroyMenu;

PROCEDURE GetMenu (): INTEGER =
  BEGIN
    RETURN GLUTRaw.GetMenu();
  END GetMenu;

PROCEDURE SetMenu (menu: INTEGER; ) =
  BEGIN
    GLUTRaw.SetMenu(menu);
  END SetMenu;

PROCEDURE AddMenuEntry (label: TEXT; value: INTEGER; ) =
  VAR arg1: C.char_star;
  BEGIN
    arg1 := M3toC.SharedTtoS(label);
    GLUTRaw.AddMenuEntry(arg1, value);
    M3toC.FreeSharedS(label, arg1);
  END AddMenuEntry;

PROCEDURE AddSubMenu (label: TEXT; subMenu: INTEGER; ) =
  VAR arg1: C.char_star;
  BEGIN
    arg1 := M3toC.SharedTtoS(label);
    GLUTRaw.AddSubMenu(arg1, subMenu);
    M3toC.FreeSharedS(label, arg1);
  END AddSubMenu;

PROCEDURE ChangeToMenuEntry
  (item: INTEGER; label: TEXT; value: INTEGER; ) =
  VAR arg2: C.char_star;
  BEGIN
    arg2 := M3toC.SharedTtoS(label);
    GLUTRaw.ChangeToMenuEntry(item, arg2, value);
    M3toC.FreeSharedS(label, arg2);
  END ChangeToMenuEntry;

PROCEDURE ChangeToSubMenu (item: INTEGER; label: TEXT; value: INTEGER; ) =
  VAR arg2: C.char_star;
  BEGIN
    arg2 := M3toC.SharedTtoS(label);
    GLUTRaw.ChangeToSubMenu(item, arg2, value);
    M3toC.FreeSharedS(label, arg2);
  END ChangeToSubMenu;

PROCEDURE RemoveMenuItem (item: INTEGER; ) =
  BEGIN
    GLUTRaw.RemoveMenuItem(item);
  END RemoveMenuItem;

PROCEDURE AttachMenu (button: INTEGER; ) =
  BEGIN
    GLUTRaw.AttachMenu(button);
  END AttachMenu;

PROCEDURE DetachMenu (button: INTEGER; ) =
  BEGIN
    GLUTRaw.DetachMenu(button);
  END DetachMenu;

PROCEDURE CreateMenu (callback: CallBack1T; ): INTEGER =
  BEGIN
    RETURN GLUTRaw.CreateMenu(callback);
  END CreateMenu;

PROCEDURE TimerFunc
  (time: CARDINAL; callback: CallBack1T; value: INTEGER; ) =
  BEGIN
    GLUTRaw.TimerFunc(time, callback, value);
  END TimerFunc;

PROCEDURE IdleFunc (callback: CallBack0T; ) =
  BEGIN
    GLUTRaw.IdleFunc(callback);
  END IdleFunc;

PROCEDURE KeyboardFunc (callback: CallBack5T; ) =
  BEGIN
    GLUTRaw.KeyboardFunc(callback);
  END KeyboardFunc;

PROCEDURE SpecialFunc (callback: CallBack3T; ) =
  BEGIN
    GLUTRaw.SpecialFunc(callback);
  END SpecialFunc;

PROCEDURE ReshapeFunc (callback: CallBack2T; ) =
  BEGIN
    GLUTRaw.ReshapeFunc(callback);
  END ReshapeFunc;

PROCEDURE VisibilityFunc (callback: CallBack1T; ) =
  BEGIN
    GLUTRaw.VisibilityFunc(callback);
  END VisibilityFunc;

PROCEDURE DisplayFunc (callback: CallBack0T; ) =
  BEGIN
    GLUTRaw.DisplayFunc(callback);
  END DisplayFunc;

PROCEDURE MouseFunc (callback: CallBack4T; ) =
  BEGIN
    GLUTRaw.MouseFunc(callback);
  END MouseFunc;

PROCEDURE MotionFunc (callback: CallBack2T; ) =
  BEGIN
    GLUTRaw.MotionFunc(callback);
  END MotionFunc;

PROCEDURE PassiveMotionFunc (callback: CallBack2T; ) =
  BEGIN
    GLUTRaw.PassiveMotionFunc(callback);
  END PassiveMotionFunc;

PROCEDURE EntryFunc (callback: CallBack1T; ) =
  BEGIN
    GLUTRaw.EntryFunc(callback);
  END EntryFunc;

PROCEDURE KeyboardUpFunc (callback: CallBack5T; ) =
  BEGIN
    GLUTRaw.KeyboardUpFunc(callback);
  END KeyboardUpFunc;

PROCEDURE SpecialUpFunc (callback: CallBack3T; ) =
  BEGIN
    GLUTRaw.SpecialUpFunc(callback);
  END SpecialUpFunc;

PROCEDURE JoystickFunc (callback: CallBack6T; pollInterval: INTEGER; ) =
  BEGIN
    GLUTRaw.JoystickFunc(callback, pollInterval);
  END JoystickFunc;

PROCEDURE MenuStateFunc (callback: CallBack1T; ) =
  BEGIN
    GLUTRaw.MenuStateFunc(callback);
  END MenuStateFunc;

PROCEDURE MenuStatusFunc (callback: CallBack3T; ) =
  BEGIN
    GLUTRaw.MenuStatusFunc(callback);
  END MenuStatusFunc;

PROCEDURE OverlayDisplayFunc (callback: CallBack0T; ) =
  BEGIN
    GLUTRaw.OverlayDisplayFunc(callback);
  END OverlayDisplayFunc;

PROCEDURE WindowStatusFunc (callback: CallBack1T; ) =
  BEGIN
    GLUTRaw.WindowStatusFunc(callback);
  END WindowStatusFunc;

PROCEDURE SpaceballMotionFunc (callback: CallBack3T; ) =
  BEGIN
    GLUTRaw.SpaceballMotionFunc(callback);
  END SpaceballMotionFunc;

PROCEDURE SpaceballRotateFunc (callback: CallBack3T; ) =
  BEGIN
    GLUTRaw.SpaceballRotateFunc(callback);
  END SpaceballRotateFunc;

PROCEDURE SpaceballButtonFunc (callback: CallBack2T; ) =
  BEGIN
    GLUTRaw.SpaceballButtonFunc(callback);
  END SpaceballButtonFunc;

PROCEDURE ButtonBoxFunc (callback: CallBack2T; ) =
  BEGIN
    GLUTRaw.ButtonBoxFunc(callback);
  END ButtonBoxFunc;

PROCEDURE DialsFunc (callback: CallBack2T; ) =
  BEGIN
    GLUTRaw.DialsFunc(callback);
  END DialsFunc;

PROCEDURE TabletMotionFunc (callback: CallBack2T; ) =
  BEGIN
    GLUTRaw.TabletMotionFunc(callback);
  END TabletMotionFunc;

PROCEDURE TabletButtonFunc (callback: CallBack4T; ) =
  BEGIN
    GLUTRaw.TabletButtonFunc(callback);
  END TabletButtonFunc;

PROCEDURE Get (query: GL.GLenum; ): INTEGER =
  BEGIN
    RETURN GLUTRaw.Get(query);
  END Get;

PROCEDURE DeviceGet (query: GL.GLenum; ): INTEGER =
  BEGIN
    RETURN GLUTRaw.DeviceGet(query);
  END DeviceGet;

PROCEDURE GetModifiers (): INTEGER =
  BEGIN
    RETURN GLUTRaw.GetModifiers();
  END GetModifiers;

PROCEDURE LayerGet (query: GL.GLenum; ): INTEGER =
  BEGIN
    RETURN GLUTRaw.LayerGet(query);
  END LayerGet;

PROCEDURE BitmapCharacter (VAR font: REFANY; character: INTEGER; ) =
  BEGIN
    GLUTRaw.BitmapCharacter(font, character);
  END BitmapCharacter;

PROCEDURE BitmapWidth (VAR font: REFANY; character: INTEGER; ): INTEGER =
  BEGIN
    RETURN GLUTRaw.BitmapWidth(font, character);
  END BitmapWidth;

PROCEDURE StrokeCharacter (VAR font: REFANY; character: INTEGER; ) =
  BEGIN
    GLUTRaw.StrokeCharacter(font, character);
  END StrokeCharacter;

PROCEDURE StrokeWidth (VAR font: REFANY; character: INTEGER; ): INTEGER =
  BEGIN
    RETURN GLUTRaw.StrokeWidth(font, character);
  END StrokeWidth;

PROCEDURE BitmapLength (VAR font: REFANY; READONLY string: TEXT; ):
  INTEGER =
  VAR
    arg2  : C.char_star;
    result: INTEGER;
  BEGIN
    arg2 := M3toC.SharedTtoS(string);
    result := GLUTRaw.BitmapLength(font, arg2);
    M3toC.FreeSharedS(string, arg2);
    RETURN result;
  END BitmapLength;

PROCEDURE StrokeLength (VAR font: REFANY; READONLY string: TEXT; ):
  INTEGER =
  VAR
    arg2  : C.char_star;
    result: INTEGER;
  BEGIN
    arg2 := M3toC.SharedTtoS(string);
    result := GLUTRaw.StrokeLength(font, arg2);
    M3toC.FreeSharedS(string, arg2);
    RETURN result;
  END StrokeLength;

PROCEDURE WireCube (size: LONGREAL; ) =
  BEGIN
    GLUTRaw.WireCube(size);
  END WireCube;

PROCEDURE SolidCube (size: LONGREAL; ) =
  BEGIN
    GLUTRaw.SolidCube(size);
  END SolidCube;

PROCEDURE WireSphere (radius: LONGREAL; slices, stacks: INTEGER; ) =
  BEGIN
    GLUTRaw.WireSphere(radius, slices, stacks);
  END WireSphere;

PROCEDURE SolidSphere (radius: LONGREAL; slices, stacks: INTEGER; ) =
  BEGIN
    GLUTRaw.SolidSphere(radius, slices, stacks);
  END SolidSphere;

PROCEDURE WireCone (base, height: LONGREAL; slices, stacks: INTEGER; ) =
  BEGIN
    GLUTRaw.WireCone(base, height, slices, stacks);
  END WireCone;

PROCEDURE SolidCone (base, height: LONGREAL; slices, stacks: INTEGER; ) =
  BEGIN
    GLUTRaw.SolidCone(base, height, slices, stacks);
  END SolidCone;

PROCEDURE WireTorus
  (innerRadius, outerRadius: LONGREAL; sides, rings: INTEGER; ) =
  BEGIN
    GLUTRaw.WireTorus(innerRadius, outerRadius, sides, rings);
  END WireTorus;

PROCEDURE SolidTorus
  (innerRadius, outerRadius: LONGREAL; sides, rings: INTEGER; ) =
  BEGIN
    GLUTRaw.SolidTorus(innerRadius, outerRadius, sides, rings);
  END SolidTorus;

PROCEDURE WireDodecahedron () =
  BEGIN
    GLUTRaw.WireDodecahedron();
  END WireDodecahedron;

PROCEDURE SolidDodecahedron () =
  BEGIN
    GLUTRaw.SolidDodecahedron();
  END SolidDodecahedron;

PROCEDURE WireOctahedron () =
  BEGIN
    GLUTRaw.WireOctahedron();
  END WireOctahedron;

PROCEDURE SolidOctahedron () =
  BEGIN
    GLUTRaw.SolidOctahedron();
  END SolidOctahedron;

PROCEDURE WireTetrahedron () =
  BEGIN
    GLUTRaw.WireTetrahedron();
  END WireTetrahedron;

PROCEDURE SolidTetrahedron () =
  BEGIN
    GLUTRaw.SolidTetrahedron();
  END SolidTetrahedron;

PROCEDURE WireIcosahedron () =
  BEGIN
    GLUTRaw.WireIcosahedron();
  END WireIcosahedron;

PROCEDURE SolidIcosahedron () =
  BEGIN
    GLUTRaw.SolidIcosahedron();
  END SolidIcosahedron;

PROCEDURE WireTeapot (size: LONGREAL; ) =
  BEGIN
    GLUTRaw.WireTeapot(size);
  END WireTeapot;

PROCEDURE SolidTeapot (size: LONGREAL; ) =
  BEGIN
    GLUTRaw.SolidTeapot(size);
  END SolidTeapot;

PROCEDURE GameModeString (string: TEXT; ) =
  VAR arg1: C.char_star;
  BEGIN
    arg1 := M3toC.SharedTtoS(string);
    GLUTRaw.GameModeString(arg1);
    M3toC.FreeSharedS(string, arg1);
  END GameModeString;

PROCEDURE EnterGameMode (): INTEGER =
  BEGIN
    RETURN GLUTRaw.EnterGameMode();
  END EnterGameMode;

PROCEDURE LeaveGameMode () =
  BEGIN
    GLUTRaw.LeaveGameMode();
  END LeaveGameMode;

PROCEDURE GameModeGet (query: GL.GLenum; ): INTEGER =
  BEGIN
    RETURN GLUTRaw.GameModeGet(query);
  END GameModeGet;

PROCEDURE VideoResizeGet (query: GL.GLenum; ): INTEGER =
  BEGIN
    RETURN GLUTRaw.VideoResizeGet(query);
  END VideoResizeGet;

PROCEDURE SetupVideoResizing () =
  BEGIN
    GLUTRaw.SetupVideoResizing();
  END SetupVideoResizing;

PROCEDURE StopVideoResizing () =
  BEGIN
    GLUTRaw.StopVideoResizing();
  END StopVideoResizing;

PROCEDURE VideoResize (x, y, width, height: INTEGER; ) =
  BEGIN
    GLUTRaw.VideoResize(x, y, width, height);
  END VideoResize;

PROCEDURE VideoPan (x, y, width, height: INTEGER; ) =
  BEGIN
    GLUTRaw.VideoPan(x, y, width, height);
  END VideoPan;

PROCEDURE SetColor (color: INTEGER; red, green, blue: REAL; ) =
  BEGIN
    GLUTRaw.SetColor(color, red, green, blue);
  END SetColor;

PROCEDURE glutGetColor (color, component: INTEGER; ): REAL =
  BEGIN
    RETURN GLUTRaw.glutGetColor(color, component);
  END glutGetColor;

PROCEDURE CopyColormap (window: INTEGER; ) =
  BEGIN
    GLUTRaw.CopyColormap(window);
  END CopyColormap;

PROCEDURE IgnoreKeyRepeat (ignore: INTEGER; ) =
  BEGIN
    GLUTRaw.IgnoreKeyRepeat(ignore);
  END IgnoreKeyRepeat;

PROCEDURE SetKeyRepeat (repeatMode: INTEGER; ) =
  BEGIN
    GLUTRaw.SetKeyRepeat(repeatMode);
  END SetKeyRepeat;

PROCEDURE ForceJoystickFunc () =
  BEGIN
    GLUTRaw.ForceJoystickFunc();
  END ForceJoystickFunc;

PROCEDURE ExtensionSupported (extension: TEXT; ): INTEGER =
  VAR
    arg1  : C.char_star;
    result: INTEGER;
  BEGIN
    arg1 := M3toC.SharedTtoS(extension);
    result := GLUTRaw.ExtensionSupported(arg1);
    M3toC.FreeSharedS(extension, arg1);
    RETURN result;
  END ExtensionSupported;

PROCEDURE ReportErrors () =
  BEGIN
    GLUTRaw.ReportErrors();
  END ReportErrors;

PROCEDURE MainLoopEvent () =
  BEGIN
    GLUTRaw.MainLoopEvent();
  END MainLoopEvent;

PROCEDURE LeaveMainLoop () =
  BEGIN
    GLUTRaw.LeaveMainLoop();
  END LeaveMainLoop;

PROCEDURE MouseWheelFunc (callback: CallBack4T; ) =
  BEGIN
    GLUTRaw.MouseWheelFunc(callback);
  END MouseWheelFunc;

PROCEDURE CloseFunc (callback: CallBack0T; ) =
  BEGIN
    GLUTRaw.CloseFunc(callback);
  END CloseFunc;

PROCEDURE WMCloseFunc (callback: CallBack0T; ) =
  BEGIN
    GLUTRaw.WMCloseFunc(callback);
  END WMCloseFunc;

PROCEDURE MenuDestroyFunc (callback: CallBack0T; ) =
  BEGIN
    GLUTRaw.MenuDestroyFunc(callback);
  END MenuDestroyFunc;

PROCEDURE SetOption (option_flag: GL.GLenum; value: INTEGER; ) =
  BEGIN
    GLUTRaw.SetOption(option_flag, value);
  END SetOption;

PROCEDURE GetWindowData (): REFANY =
  BEGIN
    RETURN GLUTRaw.GetWindowData();
  END GetWindowData;

PROCEDURE SetWindowData (VAR data: REFANY; ) =
  BEGIN
    GLUTRaw.SetWindowData(data);
  END SetWindowData;

PROCEDURE GetMenuData (): REFANY =
  BEGIN
    RETURN GLUTRaw.GetMenuData();
  END GetMenuData;

PROCEDURE SetMenuData (VAR data: REFANY; ) =
  BEGIN
    GLUTRaw.SetMenuData(data);
  END SetMenuData;

PROCEDURE BitmapHeight (VAR font: REFANY; ): INTEGER =
  BEGIN
    RETURN GLUTRaw.BitmapHeight(font);
  END BitmapHeight;

PROCEDURE StrokeHeight (VAR font: REFANY; ): REAL =
  BEGIN
    RETURN GLUTRaw.StrokeHeight(font);
  END StrokeHeight;

PROCEDURE BitmapString (VAR font: REFANY; READONLY string: TEXT; ) =
  VAR arg2: C.char_star;
  BEGIN
    arg2 := M3toC.SharedTtoS(string);
    GLUTRaw.BitmapString(font, arg2);
    M3toC.FreeSharedS(string, arg2);
  END BitmapString;

PROCEDURE StrokeString (VAR font: REFANY; READONLY string: TEXT; ) =
  VAR arg2: C.char_star;
  BEGIN
    arg2 := M3toC.SharedTtoS(string);
    GLUTRaw.StrokeString(font, arg2);
    M3toC.FreeSharedS(string, arg2);
  END StrokeString;

PROCEDURE WireRhombicDodecahedron () =
  BEGIN
    GLUTRaw.WireRhombicDodecahedron();
  END WireRhombicDodecahedron;

PROCEDURE SolidRhombicDodecahedron () =
  BEGIN
    GLUTRaw.SolidRhombicDodecahedron();
  END SolidRhombicDodecahedron;

PROCEDURE WireSierpinskiSponge (num_levels: INTEGER;
                                VAR offset: ARRAY [0 .. 2] OF LONGREAL;
                                    scale : LONGREAL;                   ) =
  BEGIN
    GLUTRaw.WireSierpinskiSponge(num_levels, offset, scale);
  END WireSierpinskiSponge;

PROCEDURE SolidSierpinskiSponge
  (    num_levels: INTEGER;
   VAR offset    : ARRAY [0 .. 2] OF LONGREAL;
       scale     : LONGREAL;                   ) =
  BEGIN
    GLUTRaw.SolidSierpinskiSponge(num_levels, offset, scale);
  END SolidSierpinskiSponge;

PROCEDURE WireCylinder
  (radius, height: LONGREAL; slices, stacks: INTEGER; ) =
  BEGIN
    GLUTRaw.WireCylinder(radius, height, slices, stacks);
  END WireCylinder;

PROCEDURE SolidCylinder
  (radius, height: LONGREAL; slices, stacks: INTEGER; ) =
  BEGIN
    GLUTRaw.SolidCylinder(radius, height, slices, stacks);
  END SolidCylinder;

PROCEDURE GetProcAddress (procName: TEXT; ): REF CallBack0T =
  VAR
    arg1  : C.char_star;
    result: REF CallBack0T;
  BEGIN
    arg1 := M3toC.SharedTtoS(procName);
    result := GLUTRaw.GetProcAddress(arg1);
    M3toC.FreeSharedS(procName, arg1);
    RETURN result;
  END GetProcAddress;


BEGIN
END GLUT.
