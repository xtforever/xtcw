#include "config.h"

char *SERVER_IP = "192.168.0.160";
char *DEVICE_NAMES[] = { "MURPHY", "TOLA", "SAMSON", "GUS", "ZORG", "JOAB", "AMON", "ILCA" };
int DEVICE_PORTS[] = { 55001,55002,55003,55004,55005,55006,55007, 55008 };

String fallback_resources[] = { 
  "ctrl-test.width: 1200",
  "*highlightThickness: 4",
  "*beNiceToColormap:  False", 
  "*shapeStyle:        Rectangle",
  "*Background: #333333",
  "*BorderColor: #333333",

  "*Foreground: #80ff00",
  "*Text.borderWidth:           0",
  "*SimpleMenu.borderWidth:     0",
  "*Paned.internalBorderWidth:  0",
  "*SmeBSB.shadowWidth:  0", 
  "*XftFont: Serif-20",

  "*Ctrl.background: #555500",
  "*SelectedFg: #80ff00",
  "*ArmedFg: #80ff00",
  "*SelectedBg: #400000",
  "*ArmedBg: #A00000",
  
  "*Ctrl.borderWidth: 8",
  "*Ctrl.borderColor: #555500",
  "*Ctrl.textFont: Sans-22:style=Bold",
  "*Ctrl.unitFont: Sans-12",
  "*Ctrl.labelFont: Sans-14",
  "*Ctrl.hspace: 10",
  "*Ctrl.vspace: 20",
  "*Inc.textFont: Serif-48",
  "*Inc.Background: steelblue",
  "*Inc.textStr: +1",
  
  "*Select.labelFont: Serif-18",
  "*Select.valueFont: Sans-28:style=bold",
  "*Select.normalFg: white",
  "*Select.normalBg: grey33",
  "*Select.selectedFg: white",
  "*Select.selectedBg: red",
  "*Select.armedFg: black",
  "*Select.armedBg: green",
  "*BorderColor: white",
  "*BorderWidth: 8",
  
  NULL
};
