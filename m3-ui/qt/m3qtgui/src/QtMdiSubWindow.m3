(*******************************************************************************
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 2.0.11
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
*******************************************************************************)

UNSAFE MODULE QtMdiSubWindow;


FROM QtSize IMPORT QSize;
IMPORT QtMdiSubWindowRaw;
FROM QtWidget IMPORT QWidget;
FROM QtMenu IMPORT QMenu;
FROM QtNamespace IMPORT WindowTypes;


IMPORT WeakRef;
FROM QtMdiArea IMPORT QMdiArea;

PROCEDURE New_QMdiSubWindow0 (self:QMdiSubWindow; parent: QWidget;
flags: WindowTypes;
): QMdiSubWindow =
VAR
result : ADDRESS;
arg1tmp :=  LOOPHOLE(parent.cxxObj,ADDRESS);
BEGIN
result := QtMdiSubWindowRaw.New_QMdiSubWindow0(arg1tmp, ORD(flags));

  self.cxxObj := result;
  EVAL WeakRef.FromRef(self,Cleanup_QMdiSubWindow);

RETURN self;
END New_QMdiSubWindow0;

PROCEDURE New_QMdiSubWindow1 (self:QMdiSubWindow; parent: QWidget;
): QMdiSubWindow =
VAR
result : ADDRESS;
arg1tmp :=  LOOPHOLE(parent.cxxObj,ADDRESS);
BEGIN
result := QtMdiSubWindowRaw.New_QMdiSubWindow1(arg1tmp);

  self.cxxObj := result;
  EVAL WeakRef.FromRef(self,Cleanup_QMdiSubWindow);

RETURN self;
END New_QMdiSubWindow1;

PROCEDURE New_QMdiSubWindow2 (self:QMdiSubWindow;): QMdiSubWindow =
VAR
result : ADDRESS;
BEGIN
result := QtMdiSubWindowRaw.New_QMdiSubWindow2();

  self.cxxObj := result;
  EVAL WeakRef.FromRef(self,Cleanup_QMdiSubWindow);

RETURN self;
END New_QMdiSubWindow2;

PROCEDURE Delete_QMdiSubWindow ( self: QMdiSubWindow;
) =
VAR
selfAdr: ADDRESS := LOOPHOLE(self.cxxObj,ADDRESS);
BEGIN
QtMdiSubWindowRaw.Delete_QMdiSubWindow(selfAdr);
END Delete_QMdiSubWindow;

PROCEDURE QMdiSubWindow_sizeHint ( self: QMdiSubWindow;
): QSize =
VAR
ret:ADDRESS; result : QSize;
selfAdr: ADDRESS := LOOPHOLE(self.cxxObj,ADDRESS);
BEGIN
ret := QtMdiSubWindowRaw.QMdiSubWindow_sizeHint(selfAdr);

  result := NEW(QSize);
  result.cxxObj := ret;
  result.destroyCxx();

RETURN result;
END QMdiSubWindow_sizeHint;

PROCEDURE QMdiSubWindow_minimumSizeHint ( self: QMdiSubWindow;
): QSize =
VAR
ret:ADDRESS; result : QSize;
selfAdr: ADDRESS := LOOPHOLE(self.cxxObj,ADDRESS);
BEGIN
ret := QtMdiSubWindowRaw.QMdiSubWindow_minimumSizeHint(selfAdr);

  result := NEW(QSize);
  result.cxxObj := ret;
  result.destroyCxx();

RETURN result;
END QMdiSubWindow_minimumSizeHint;

PROCEDURE QMdiSubWindow_setWidget ( self: QMdiSubWindow;
 widget: QWidget;
) =
VAR
selfAdr: ADDRESS := LOOPHOLE(self.cxxObj,ADDRESS);
arg2tmp :=  LOOPHOLE(widget.cxxObj,ADDRESS);
BEGIN
QtMdiSubWindowRaw.QMdiSubWindow_setWidget(selfAdr, arg2tmp);
END QMdiSubWindow_setWidget;

PROCEDURE QMdiSubWindow_widget ( self: QMdiSubWindow;
): QWidget =
VAR
ret:ADDRESS; result : QWidget;
selfAdr: ADDRESS := LOOPHOLE(self.cxxObj,ADDRESS);
BEGIN
ret := QtMdiSubWindowRaw.QMdiSubWindow_widget(selfAdr);

  result := NEW(QWidget);
  result.cxxObj := ret;
  result.destroyCxx();

RETURN result;
END QMdiSubWindow_widget;

PROCEDURE QMdiSubWindow_maximizedButtonsWidget ( self: QMdiSubWindow;
): QWidget =
VAR
ret:ADDRESS; result : QWidget;
selfAdr: ADDRESS := LOOPHOLE(self.cxxObj,ADDRESS);
BEGIN
ret := QtMdiSubWindowRaw.QMdiSubWindow_maximizedButtonsWidget(selfAdr);

  result := NEW(QWidget);
  result.cxxObj := ret;
  result.destroyCxx();

RETURN result;
END QMdiSubWindow_maximizedButtonsWidget;

PROCEDURE QMdiSubWindow_maximizedSystemMenuIconWidget ( self: QMdiSubWindow;
): QWidget =
VAR
ret:ADDRESS; result : QWidget;
selfAdr: ADDRESS := LOOPHOLE(self.cxxObj,ADDRESS);
BEGIN
ret := QtMdiSubWindowRaw.QMdiSubWindow_maximizedSystemMenuIconWidget(selfAdr);

  result := NEW(QWidget);
  result.cxxObj := ret;
  result.destroyCxx();

RETURN result;
END QMdiSubWindow_maximizedSystemMenuIconWidget;

PROCEDURE QMdiSubWindow_isShaded ( self: QMdiSubWindow;
): BOOLEAN =
VAR
selfAdr: ADDRESS := LOOPHOLE(self.cxxObj,ADDRESS);
BEGIN
RETURN QtMdiSubWindowRaw.QMdiSubWindow_isShaded(selfAdr);
END QMdiSubWindow_isShaded;

PROCEDURE QMdiSubWindow_setOption ( self: QMdiSubWindow;
option: SubWindowOption;
on: BOOLEAN;
) =
VAR
selfAdr: ADDRESS := LOOPHOLE(self.cxxObj,ADDRESS);
BEGIN
QtMdiSubWindowRaw.QMdiSubWindow_setOption(selfAdr, ORD(option), on);
END QMdiSubWindow_setOption;

PROCEDURE QMdiSubWindow_setOption1 ( self: QMdiSubWindow;
option: SubWindowOption;
) =
VAR
selfAdr: ADDRESS := LOOPHOLE(self.cxxObj,ADDRESS);
BEGIN
QtMdiSubWindowRaw.QMdiSubWindow_setOption1(selfAdr, ORD(option));
END QMdiSubWindow_setOption1;

PROCEDURE QMdiSubWindow_testOption ( self: QMdiSubWindow;
arg2: SubWindowOption;
): BOOLEAN =
VAR
selfAdr: ADDRESS := LOOPHOLE(self.cxxObj,ADDRESS);
BEGIN
RETURN QtMdiSubWindowRaw.QMdiSubWindow_testOption(selfAdr, ORD(arg2));
END QMdiSubWindow_testOption;

PROCEDURE QMdiSubWindow_setKeyboardSingleStep ( self: QMdiSubWindow;
step: INTEGER;
) =
VAR
selfAdr: ADDRESS := LOOPHOLE(self.cxxObj,ADDRESS);
BEGIN
QtMdiSubWindowRaw.QMdiSubWindow_setKeyboardSingleStep(selfAdr, step);
END QMdiSubWindow_setKeyboardSingleStep;

PROCEDURE QMdiSubWindow_keyboardSingleStep ( self: QMdiSubWindow;
): INTEGER =
VAR
selfAdr: ADDRESS := LOOPHOLE(self.cxxObj,ADDRESS);
BEGIN
RETURN QtMdiSubWindowRaw.QMdiSubWindow_keyboardSingleStep(selfAdr);
END QMdiSubWindow_keyboardSingleStep;

PROCEDURE QMdiSubWindow_setKeyboardPageStep ( self: QMdiSubWindow;
step: INTEGER;
) =
VAR
selfAdr: ADDRESS := LOOPHOLE(self.cxxObj,ADDRESS);
BEGIN
QtMdiSubWindowRaw.QMdiSubWindow_setKeyboardPageStep(selfAdr, step);
END QMdiSubWindow_setKeyboardPageStep;

PROCEDURE QMdiSubWindow_keyboardPageStep ( self: QMdiSubWindow;
): INTEGER =
VAR
selfAdr: ADDRESS := LOOPHOLE(self.cxxObj,ADDRESS);
BEGIN
RETURN QtMdiSubWindowRaw.QMdiSubWindow_keyboardPageStep(selfAdr);
END QMdiSubWindow_keyboardPageStep;

PROCEDURE QMdiSubWindow_setSystemMenu ( self: QMdiSubWindow;
 systemMenu: QMenu;
) =
VAR
selfAdr: ADDRESS := LOOPHOLE(self.cxxObj,ADDRESS);
arg2tmp :=  LOOPHOLE(systemMenu.cxxObj,ADDRESS);
BEGIN
QtMdiSubWindowRaw.QMdiSubWindow_setSystemMenu(selfAdr, arg2tmp);
END QMdiSubWindow_setSystemMenu;

PROCEDURE QMdiSubWindow_systemMenu ( self: QMdiSubWindow;
): QMenu =
VAR
ret:ADDRESS; result : QMenu;
selfAdr: ADDRESS := LOOPHOLE(self.cxxObj,ADDRESS);
BEGIN
ret := QtMdiSubWindowRaw.QMdiSubWindow_systemMenu(selfAdr);

  result := NEW(QMenu);
  result.cxxObj := ret;
  result.destroyCxx();

RETURN result;
END QMdiSubWindow_systemMenu;

PROCEDURE QMdiSubWindow_mdiArea ( self: QMdiSubWindow;
): REFANY =
VAR
ret:ADDRESS; result : QMdiArea;
selfAdr: ADDRESS := LOOPHOLE(self.cxxObj,ADDRESS);
BEGIN
ret := QtMdiSubWindowRaw.QMdiSubWindow_mdiArea(selfAdr);

  result := NEW(QMdiArea);
  result.cxxObj := ret;
  result.destroyCxx();

RETURN result;
END QMdiSubWindow_mdiArea;

PROCEDURE QMdiSubWindow_showSystemMenu ( self: QMdiSubWindow;
) =
VAR
selfAdr: ADDRESS := LOOPHOLE(self.cxxObj,ADDRESS);
BEGIN
QtMdiSubWindowRaw.QMdiSubWindow_showSystemMenu(selfAdr);
END QMdiSubWindow_showSystemMenu;

PROCEDURE QMdiSubWindow_showShaded ( self: QMdiSubWindow;
) =
VAR
selfAdr: ADDRESS := LOOPHOLE(self.cxxObj,ADDRESS);
BEGIN
QtMdiSubWindowRaw.QMdiSubWindow_showShaded(selfAdr);
END QMdiSubWindow_showShaded;

PROCEDURE Cleanup_QMdiSubWindow(<*UNUSED*>READONLY self: WeakRef.T; ref: REFANY) =
VAR obj : QMdiSubWindow := ref;
BEGIN
  Delete_QMdiSubWindow(obj);
 END Cleanup_QMdiSubWindow;

PROCEDURE Destroy_QMdiSubWindow(self : QMdiSubWindow) =
BEGIN
  EVAL WeakRef.FromRef(self,Cleanup_QMdiSubWindow);
END Destroy_QMdiSubWindow;

REVEAL
QMdiSubWindow =
QMdiSubWindowPublic BRANDED OBJECT
OVERRIDES
init_0 := New_QMdiSubWindow0;
init_1 := New_QMdiSubWindow1;
init_2 := New_QMdiSubWindow2;
sizeHint := QMdiSubWindow_sizeHint;
minimumSizeHint := QMdiSubWindow_minimumSizeHint;
setWidget := QMdiSubWindow_setWidget;
widget := QMdiSubWindow_widget;
maximizedButtonsWidget := QMdiSubWindow_maximizedButtonsWidget;
maximizedSystemMenuIconWidget := QMdiSubWindow_maximizedSystemMenuIconWidget;
isShaded := QMdiSubWindow_isShaded;
setOption := QMdiSubWindow_setOption;
setOption1 := QMdiSubWindow_setOption1;
testOption := QMdiSubWindow_testOption;
setKeyboardSingleStep := QMdiSubWindow_setKeyboardSingleStep;
keyboardSingleStep := QMdiSubWindow_keyboardSingleStep;
setKeyboardPageStep := QMdiSubWindow_setKeyboardPageStep;
keyboardPageStep := QMdiSubWindow_keyboardPageStep;
setSystemMenu := QMdiSubWindow_setSystemMenu;
systemMenu := QMdiSubWindow_systemMenu;
mdiArea := QMdiSubWindow_mdiArea;
showSystemMenu := QMdiSubWindow_showSystemMenu;
showShaded := QMdiSubWindow_showShaded;
destroyCxx := Destroy_QMdiSubWindow;
END;


BEGIN
END QtMdiSubWindow.
