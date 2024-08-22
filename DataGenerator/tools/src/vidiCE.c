/* vidiCE.c visualize a rectangle distribution */
#//
#// Copyright (c) 1994 - 2013 Norbert Beckmann  <nb@informatik.uni-bremen.de>
#//
/*
 * visualize the rectangles of a 2D rectangle file (using Xm widgets)
 ---
 * must be compiled using -D options to guarantee that
 * #define typcoord [double, float]
 * will be set properly
*/


/* Remark:
   setting colors similarly to
   XtVaSetValues(widget,XtVaTypedArg,
                        XmNbackground,XmRString,"#A0C4FF",strlen("#A0C4FF") + 1,
                        XtVaTypedArg,
                        XmNforeground,XmRString,"black",strlen("black") + 1,
                        NULL);
   would require further investigation! E.g. button-pressed-color needed. */

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/stat.h>
#include <math.h>
// #include <floatingpoint.h> anno 1996
#include <X11/Xfuncproto.h>
#include <X11/Intrinsic.h>
#include <X11/IntrinsicP.h>
// ^ include declaration of XtResizeWidget (which Intrinsic.h does not)
#include <X11/StringDefs.h>
#include <Xm/Xm.h>
#include <Xm/Form.h>
#include <Xm/RowColumn.h>
#include <Xm/PushB.h>
#include <Xm/ToggleB.h>
#include <Xm/TextF.h>
#include <Xm/DrawingA.h>
//-----
#include "RSTStdTypes.h"
/* RSTStdTypes.h mainly provides R*-tree standard constants, types and
   functions, normally not needed here.
   But type Rpint is used to provide a pointer compatible integer type on
   platforms, not serving a 32/64 bit transitional long integer and concerning
   I/O functions (Windows).
   Note that if a source includes RSTStdTypes.h, linking requires libPltfHelp
   or libUTIL. */

/* constants */

/* see comments at top */


/* types */

/*------------------------------------------------------------*/
typedef struct {
               typcoord cx, cy, ex, ey;
               } data_rect;
/* input format of coordinates,
   conversion to Xwindow format see "data_rect" below */
/*------------------------------------------------------------*/

typedef struct {
  Widget canvas;
  Dimension width, height;
  typcoord WorldXL, WorldXH, WorldYL, WorldYH;
  typcoord WorldXlength, WorldYlength;
  typcoord WorldXS, WorldYS;
  typcoord HorScale, VerScale;
  int linewidth;
  int func;
  int depth;
  Bool isInvert;
  Bool go;
  Bool MathOri;
  Bool ok;
  Bool slowMo;
  Bool counting;
  Bool internCall;
  GC gc;
  Pixmap pix;
  FILE *f;
  char message[161];
  } image_data, *ref_image_data;

/* type timespec_t already identically defined in Solaris (and Linux?) */
typedef struct timespec timespec_t;


/* declarations */
void create_gc(Widget w, image_data *data);
void init_environ(Widget w, Widget m_f, Widget nE_f,
                  image_data *data, int argc, char *argv[]);
void redisplay(Widget w, XtPointer data, XtPointer calldata);
void invert(Widget w, XtPointer data, XtPointer calldata);
void quit(Widget w, XtPointer data, XtPointer calldata);
void beep(Widget w, XtPointer data, XtPointer calldata);
void step(Widget w, XtPointer data, XtPointer calldata);
void cont(Widget w, XtPointer data, XtPointer calldata);
void stop(Widget w, XtPointer data, XtPointer calldata);
void reset(Widget w, XtPointer data, XtPointer calldata);
void initcan(Widget w, image_data *data);
void clear(Widget w, XtPointer data, XtPointer calldata);
void clearcan(Widget w, image_data *data);
void xor_func(Widget w, XtPointer data, XtPointer calldata);
void slow_func(Widget w, XtPointer data, XtPointer calldata);
void count_func(Widget w, XtPointer voiddata, XtPointer voidcalldata);
Rpint GetLOF(char *path); // GetLengthOfFile
int DecStrLen(int byteSize);

/* global variables */

Arg args[10];
timespec_t sleepTime;
Rpint rectCount;
// counting number string specification:
char *numbEntStr, *countStr;
int numbEntStrLen, countStrLen;
char CSF[16]; // CountStringFormat
// -----
Widget count_button, slow_button, number_field;
char s[160];


int main(int argc, char *argv[])

{
  image_data data;
  int msglen;
  Rpint dtm, ctm, rectsPerSec, timePerRect, timePerSleep;
  Widget toplevel, board, mainrow, headrow, namerow,
         stepbutrow, togcntbutrow, filebutrow, togxorbutrow, quitbutrow,
         canrow,
         canvas,
         message_field, numbEnt_field,
         step_button, cont_button, stop_button,
         // count_button, slow_button, //global
         reset_button, clear_button,
         xor_button,
         // number_field, // global
         quit_button;
  XColor tglColor, xorColor/*, fg, bg*/;
  Colormap cmap_cntSlw_b, cmap_xor_b;
  
  /* defaults */
  data.width= 512;
  data.height= 512;
  data.WorldXL= 0.0;
  data.WorldXH= 1.0;
  data.WorldYL= 0.0;
  data.WorldYH= 1.0;
  data.linewidth= 0;
  data.func= GXor;
  data.isInvert= FALSE;
  data.go= FALSE;
  data.MathOri= FALSE;
  data.ok= FALSE;
  data.slowMo= FALSE;
  data.counting= FALSE;
  data.internCall= FALSE;
  
  tglColor.flags= DoRed|DoGreen|DoBlue;
  xorColor.flags= DoRed|DoGreen|DoBlue;
  //fg.flags= DoRed|DoGreen|DoBlue;
  //bg.flags= DoRed|DoGreen|DoBlue;
  
  toplevel= XtInitialize("vidi","Vidi",NULL,0,&argc,argv);
  
  board= XtCreateManagedWidget("board",xmFormWidgetClass,toplevel,NULL,0);
  XtVaSetValues(board,XmNentryBorder,0,NULL);
  
  mainrow= XtCreateManagedWidget("mainrow",xmRowColumnWidgetClass,board,NULL,0);
  XtVaSetValues(mainrow,XmNorientation,XmHORIZONTAL,
                        XmNpacking,XmPACK_TIGHT,
                        XmNentryBorder,0,NULL);
  
  headrow= XtCreateManagedWidget("headrow",xmRowColumnWidgetClass,mainrow,NULL,0);
  XtVaSetValues(headrow,XmNorientation,XmVERTICAL,
                        XmNpacking,XmPACK_TIGHT,
                        XmNentryBorder,0,NULL);
  
  namerow= XtCreateManagedWidget("namerow",xmRowColumnWidgetClass,headrow,NULL,0);
  XtVaSetValues(namerow,XmNorientation,XmVERTICAL,
                        XmNpacking,XmPACK_TIGHT,
                        XmNentryBorder,0,NULL);
  //XtVaGetValues (namerow,XmNforeground,&fg,XmNbackground,&bg,NULL);
  
  stepbutrow= XtCreateManagedWidget("stepbutrow",xmRowColumnWidgetClass,headrow,NULL,0);
  XtVaSetValues(stepbutrow,XmNorientation,XmVERTICAL,
                       XmNpacking,XmPACK_TIGHT,
                       XmNentryBorder,0,NULL);
  
  togcntbutrow= XtCreateManagedWidget("togcntbutrow",xmRowColumnWidgetClass,headrow,NULL,0);
  XtVaSetValues(togcntbutrow,XmNorientation,XmVERTICAL,
                       XmNpacking,XmPACK_TIGHT,
                       XmNentryBorder,1,NULL);
  
  filebutrow= XtCreateManagedWidget("filebutrow",xmRowColumnWidgetClass,headrow,NULL,0);
  XtVaSetValues(filebutrow,XmNorientation,XmVERTICAL,
                       XmNpacking,XmPACK_TIGHT,
                       XmNentryBorder,0,NULL);
  
  togxorbutrow= XtCreateManagedWidget("togxorbutrow",xmRowColumnWidgetClass,headrow,NULL,0);
  XtVaSetValues(togxorbutrow,XmNorientation,XmVERTICAL,
                       XmNpacking,XmPACK_TIGHT,
                       XmNentryBorder,1,NULL);
  
  quitbutrow= XtCreateManagedWidget("quitbutrow",xmRowColumnWidgetClass,headrow,NULL,0);
  XtVaSetValues(quitbutrow,XmNorientation,XmVERTICAL,
                       XmNpacking,XmPACK_TIGHT,
                       XmNentryBorder,0,NULL);
  
  /*** create the message_field anyway (but not the buttons and so on): ***/
  message_field= XtCreateManagedWidget("message",xmTextFieldWidgetClass,namerow,NULL,0);
  // set background color, blink behavior and editability:
  XtVaSetValues(message_field,XtVaTypedArg,XmNbackground,XmRString,"white",strlen("white") + 1,
                              XmNblinkRate,0,NULL);
  XmTextFieldSetEditable(message_field,FALSE);
  
  numbEnt_field= NULL; // used in init_environ (like message_field) if data.ok
  if (argc > 1) {
    data.f= fopen(argv[1],"rb");
    if (!data.f) {
      strcpy(data.message," unrecognized filename: ");
      strcat(data.message,argv[1]);
      msglen= strlen(data.message);
      XtSetArg(args[0],XmNcolumns,msglen);
      XtSetValues(message_field,args,1);
      XmTextFieldSetString(message_field,data.message);
    }
    else {
      data.ok= TRUE;
      
      // make colors available (in toplevel (usable everywhere)):
      XAllocColor(XtDisplay(toplevel),
                  DefaultColormapOfScreen(XtScreen(toplevel)),
                  &tglColor);
      XAllocColor(XtDisplay(toplevel),
                  DefaultColormapOfScreen(XtScreen(toplevel)),
                  &xorColor);

      // considering drawing time, counting time (3GHz):
      dtm= 7500; ctm= 11300;
      // slow motion:
      rectsPerSec= 100;                          // rectangles per second
      timePerRect= 1000000000 / rectsPerSec;     // 1000000000ns = 1s
      timePerSleep= timePerRect - dtm - ctm;
      
      sleepTime.tv_sec= 0;
      sleepTime.tv_nsec= timePerSleep;
      
      rectCount= 0;
      
      /* create number of entries field: */
      numbEnt_field= XtCreateManagedWidget("mumbEnt",xmTextFieldWidgetClass,namerow,NULL,0);
      XtVaSetValues(numbEnt_field,XtVaTypedArg,XmNbackground,XmRString,"white",strlen("white") + 1,
                                  XmNblinkRate,0,NULL);
      XmTextFieldSetEditable(numbEnt_field,FALSE);
      
      // create number field:
      number_field= XtCreateManagedWidget("number",xmTextFieldWidgetClass,namerow,NULL,0);
      // set background color, blink behavior, length and editability:
      countStrLen= DecStrLen(sizeof(Rpint));
      sprintf(CSF,"  %%%dP",countStrLen);
      countStrLen+= 2; // for the "  "
      countStr= malloc(countStrLen + 1);
      XtVaSetValues(number_field,XtVaTypedArg,XmNbackground,XmRString,"white",strlen("white") + 1,
                                 XmNblinkRate,0,
                                 XmNcolumns,countStrLen,NULL);
      XmTextFieldSetEditable(number_field,FALSE);
      
      // set initial string:
      sprintf(countStr,strans(CSF,s),rectCount);
      XmTextFieldSetString(number_field,countStr);
      
      /* create buttons: */
      step_button= XtCreateManagedWidget("step",xmPushButtonWidgetClass,stepbutrow,NULL,0);
      XtAddCallback(step_button,XmNactivateCallback,step,&data);
      
      cont_button= XtCreateManagedWidget("cont",xmPushButtonWidgetClass,stepbutrow,NULL,0);
      XtAddCallback(cont_button,XmNactivateCallback,cont,&data);
      
      stop_button= XtCreateManagedWidget("stop",xmPushButtonWidgetClass,stepbutrow,NULL,0);
      XtAddCallback(stop_button,XmNactivateCallback,stop,&data);
      
      count_button= XtCreateManagedWidget("count",xmToggleButtonWidgetClass,togcntbutrow,NULL,0);
      XtAddCallback(count_button,XmNvalueChangedCallback,count_func,&data);
      
      // set a color (actually 16 (not 8) bit values): (yellow: hint)
      tglColor.red= 255 << 8;
      tglColor.green= 235 << 8;
      tglColor.blue= 25 << 8;
      // make color map for count_button:
      cmap_cntSlw_b = DefaultColormapOfScreen(XtScreen(count_button));
      // free all colors of count_b (could be limited without true color):
      XFreeColors(XtDisplay(count_button),cmap_cntSlw_b,&tglColor.pixel, 1, 0);
      // allocate the new color:
      XAllocColor(XtDisplay(count_button),cmap_cntSlw_b,&tglColor);
      // set toggling colors:
      XtSetArg(args[0],XmNselectColor,tglColor.pixel);
      XtSetArg(args[1],XmNenableToggleColor,TRUE);
      XtSetValues(count_button,args,2);
      
      slow_button= XtCreateManagedWidget("slow",xmToggleButtonWidgetClass,togcntbutrow,NULL,0);
      XtAddCallback(slow_button,XmNvalueChangedCallback,slow_func,&data);
      XtSetValues(slow_button,args,2);
      
      reset_button= XtCreateManagedWidget("reset",xmPushButtonWidgetClass,filebutrow,NULL,0);
      XtAddCallback(reset_button,XmNactivateCallback,reset,&data);
      
      clear_button= XtCreateManagedWidget("clr",xmPushButtonWidgetClass,filebutrow,NULL,0);
      XtAddCallback(clear_button,XmNactivateCallback,clear,&data);
      
      xor_button= XtCreateManagedWidget("xor",xmToggleButtonWidgetClass,togxorbutrow,NULL,0);
      XtAddCallback(xor_button,XmNvalueChangedCallback,xor_func,&data);
      
      // set a color (actually 16 (not 8) bit values): (red: warning)
      xorColor.red= 255 << 8;
      xorColor.green= 0 << 8;
      xorColor.blue= 0 << 8;
      // make color map for xor_button:
      cmap_xor_b = DefaultColormapOfScreen(XtScreen(xor_button));
      // free all colors of xor_b (could be limited without true color):
      XFreeColors(XtDisplay(xor_button), cmap_xor_b, &xorColor.pixel, 1, 0);
      // allocate the new color:
      XAllocColor(XtDisplay(xor_button), cmap_xor_b, &xorColor);
      // set toggling colors:
      XtSetArg(args[0],XmNselectColor,xorColor.pixel);
      XtSetArg(args[1],XmNenableToggleColor,TRUE);
      XtSetValues(xor_button,args,2);
    }
  }
  else {
    fprintf(stderr,"%s\n",
"vidiCE displays rectangle data files"
    );
    fprintf(stderr,"%s",
"data format:  centerX, centerY, extensionX, extensionY: "
    );
    if (sizeof(typcoord) == sizeof(double)) {
      fprintf(stderr,"DOUBLE\n");
    }
    else if (sizeof(typcoord) == sizeof(float)) {
      fprintf(stderr,"FLOAT\n");
    }
    else {
      fprintf(stderr,"UNKNOWN (error?)\n");
    }
    fprintf(stderr,"%s\n",
"              where extension is the distance between center and edge."
    );
    fprintf(stderr,"%s\n",
"Usage:"
    );
    fprintf(stderr,"%s\n",
"  vidiCE filename [ xl xh yl yh ] [ -wwidth ] [ -m ] [ -b ]"
    );
    fprintf(stderr,"%s\n",
"    xl, xh: Minimum, maximum world coordinates, X-axis, default: 0, 1"
    );
    fprintf(stderr,"%s\n",
"    yl, yh: Minimum, maximum world coordinates, Y-axis, default: 0, 1"
    );
    fprintf(stderr,"%s\n",
"    -wwidth: width of the quadratic drawing area in pixels, default: 512"
    );
    fprintf(stderr,"%s\n",
"    -m: mathematic orientation: 0,0 at left lower, default: 0,0 at left upper"
    );
    fprintf(stderr,"%s\n",
"    -b: bordered drawing area, default: unbordered"
    );
    fprintf(stderr,"%s\n",
"-----  Clicks:  -----"
    );
    fprintf(stderr,"%s\n",
"step:             draws rectangles step by step"
    );
    fprintf(stderr,"%s\n",
"cont:             draws rectangles continuously"
    );
    fprintf(stderr,"%s\n",
"stop:             stops continuous drawing"
    );
    fprintf(stderr,"%s\n",
"count:            toggles counting (drawing speed reduced by factor 2.5)"
    );
    fprintf(stderr,"%s\n",
"slow:             toggles \"slow motion\" (drawing speed: 100/s)"
    );
    fprintf(stderr,"%s\n",
"reset:            resets to the beginning of the file, read from"
    );
    fprintf(stderr,"%s\n",
"clr:              clears the canvas"
    );
    fprintf(stderr,"%s\n",
"xor:              toggles XOR drawing mode"
    );
    fprintf(stderr,"%s\n",
"quit:             quits the application"
    );
    fprintf(stderr,"%s\n",
"CLICK ON CANVAS:  reverses video mode"
    );
    fprintf(stderr,"%s\n",
"------------------------------------------------------------------------------"
    );
    fprintf(stderr,"%s\n",
"Example:"
    );
    fprintf(stderr,"%s\n",
"vidiCE rea02 -120.4 -118.1 36.1 38.4 -m -w960 -b"
    );
    fprintf(stderr,"%s\n",
"Projects (a cutout of) file \"rea02\" with the limits"
    );
    fprintf(stderr,"%s\n",
"X-axis: [-120.4, -118.1],  Y-axis: [36.1, 38.4]"
    );
    fprintf(stderr,"%s\n",
"onto a canvas of size 960 x 960 pixels."
    );
    fprintf(stderr,"%s\n",
"Origin is at left lower corner,  canvas is bordered."
    );
    fprintf(stderr,"%s\n",
"Note: The file is scanned sequentially, hence setting up a small cutout may"
    );
    fprintf(stderr,"%s\n",
"      lead to long phases without something happening on the canvas. Turning"
    );
    fprintf(stderr,"%s\n",
"      on counting allows monitoring the advance."
    );
    fprintf(stderr,"%s\n",
"------------------------------------------------------------------------------"
    );

    strcpy(data.message," You'd have to specify a filename on the command line");
    msglen= strlen(data.message);
    XtVaSetValues(message_field,XmNcolumns,msglen,NULL);
    XmTextFieldSetString(message_field,data.message);
  }
  XtAddCallback(message_field,XmNactivateCallback,beep,NULL);

  quit_button= XtCreateManagedWidget("quit",xmPushButtonWidgetClass,quitbutrow,NULL,0);
  XtAddCallback(quit_button,XmNactivateCallback,quit,&data);
  
  canrow= XtCreateManagedWidget("canrow",xmRowColumnWidgetClass,mainrow,NULL,0);
  XtVaSetValues(canrow,XmNorientation,XmVERTICAL,
                       XmNpacking,XmPACK_TIGHT,NULL);
  
  canvas= XtCreateManagedWidget("canvas",xmDrawingAreaWidgetClass,canrow,NULL,0);
  XtVaSetValues(canvas,XtVaTypedArg,XmNbackground,XmRString,"black",strlen("black") + 1,
                       XtVaTypedArg,XmNforeground,XmRString,"white",strlen("white") + 1,NULL);
  
  init_environ(canvas,message_field,numbEnt_field,&data,argc,argv);
  XtAddCallback(canvas,XmNinputCallback,invert,&data);
  XtAddCallback(canvas,XmNresizeCallback,redisplay,&data);
  XtAddCallback(canvas,XmNexposeCallback,redisplay,&data);
  
  XtRealizeWidget(toplevel);
  XtMainLoop();
  return 0;
}

/**************************************************************/

void initcan(Widget w, image_data *data)

{
  int i;

  XSetFunction(XtDisplay(w),data->gc,GXclear);
  for (i= 0; i < data->height; i++) {
    XDrawLine(XtDisplay(w),data->pix,data->gc,0,i,data->width-1,i);
  }
}

/****************************************************************/

void clear(Widget w, XtPointer voiddata, XtPointer voidcalldata)

{
  ref_image_data data;
  
  data= (ref_image_data)voiddata;
  
  clearcan(data->canvas,data);
  redisplay(data->canvas,data,voidcalldata);
}

/**************************************************************/

void clearcan(Widget w, image_data *data)

{
  int i;

  if (data->isInvert) {
    XSetFunction(XtDisplay(w),data->gc,GXor);
    for (i= 0; i < data->height; i++) {
      XDrawLine(XtDisplay(w),data->pix,data->gc,0,i,data->width-1,i);
    }
    if (data->func == GXxor) {
      XSetFunction(XtDisplay(w),data->gc,GXxor);
    }
    else {
      XSetFunction(XtDisplay(w),data->gc,GXandInverted);
    }
  }
  else {
    XSetFunction(XtDisplay(w),data->gc,GXandInverted);
    for (i= 0; i < data->height; i++) {
      XDrawLine(XtDisplay(w),data->pix,data->gc,0,i,data->width-1,i);
    }
    if (data->func == GXxor) {
      XSetFunction(XtDisplay(w),data->gc,GXxor);
    }
    else {
      XSetFunction(XtDisplay(w),data->gc,GXor);
    }
  }
}

/****************************************************************/

void create_gc(Widget w, image_data *data) /* page 229 */

{
  XGCValues values;

  // get the colors used by the widget:
  XtSetArg(args[0],XtNforeground,&values.foreground);
  XtSetArg(args[1],XtNbackground,&values.background);
  XtGetValues(w,args,2);

  values.foreground= values.foreground ^ values.background;
  values.function= data->func;
  values.line_width= data->linewidth;
  data->gc= XtGetGC(w, GCForeground | GCBackground | GCFunction | GCLineWidth,&values);
}

/**************************************************************/

void init_environ(Widget w, Widget m_f, Widget nE_f, image_data *data, int argc, char *argv[])

{
  int a;
  char *str;
  char s[80];
  char nESF[16];
  int borderwidth, msglen;
  Rpint numbRects;
  
  borderwidth= 0;
  // get command line arguments:
  for (a= 2; a < argc; a++) {
    str= argv[a];
    if (*argv[a] == '-') {
      str++;
      if (*str) {
        if (*str == 'w') {
          str++;
          data->width= atoi(str);
          data->height= data->width;
        }
        else if (*str == 'b') {
          borderwidth= 1;
        }
        else if (*str == 'm') {
          data->MathOri= TRUE;
        }
        else if (a == 2) {
          data->WorldXL= -atof(str);
        }
        else if (a == 3) {
          data->WorldXH= -atof(str);
        }
        else if (a == 4) {
          data->WorldYL= -atof(str);
        }
        else if (a == 5) {
          data->WorldYH= -atof(str);
        }
      }
    }
    else if (*str) {
      if (a == 2) {
        data->WorldXL= atof(str);
      }
      else if (a == 3) {
        data->WorldXH= atof(str);
      }
      else if (a == 4) {
        data->WorldYL= atof(str);
      }
      else if (a == 5) {
        data->WorldYH= atof(str);
      }
    }
  }
  if (data->ok) {
    strcpy(data->message," ");
    strcat(data->message,argv[1]);
    if (data->MathOri) {
      strcat(data->message," -m");
    }
    else {
      strcat(data->message,"   ");
    }
    msglen= strlen(data->message);
    XtVaSetValues(m_f,XmNcolumns,msglen,NULL);
    
    // display number of entries in numbEnt_field (nE_f):
    numbEntStrLen= DecStrLen(sizeof(Rpint));
    sprintf(nESF," #%%%dP",numbEntStrLen);
    numbEntStrLen+= 2; // for the " #";
    numbEntStr= malloc(numbEntStrLen + 1);
    numbRects= GetLOF(argv[1]) / (4 * sizeof(typcoord));
    sprintf(numbEntStr,strans(nESF,s),numbRects);
    XtVaSetValues(nE_f,XmNcolumns,numbEntStrLen,NULL);
    XmTextFieldSetString(nE_f,numbEntStr);      
  }
  XmTextFieldSetString(m_f,data->message);
  
  data->WorldXlength= data->WorldXH - data->WorldXL;
  data->WorldYlength= data->WorldYH - data->WorldYL;
  data->WorldXS= 1.0 / data->WorldXlength;
  data->WorldYS= 1.0 / data->WorldYlength;
  XtResizeWidget(w,data->width,data->height,borderwidth);
  data->canvas= w;
  data->depth= DefaultDepthOfScreen(XtScreen(w));
  data->pix= XCreatePixmap(XtDisplay(w),DefaultRootWindow(XtDisplay(w)),data->width, data->height, data->depth);
  data->HorScale= (data->width - 1) * data->WorldXS;
  data->VerScale= (data->height - 1) * data->WorldYS;
  create_gc(w,data);
  initcan(w,data);
}

/************************************************************/

void xor_func(Widget w, XtPointer voiddata, XtPointer voidcalldata)
 
{
  ref_image_data data;
  
  data= (ref_image_data)voiddata;
  
  if (data->func == GXor) {
    data->func= GXxor;
  }
  else {
    data->func= GXor;
  }
  redisplay(data->canvas,data,voidcalldata);
}

/**********************************************************/
/* slowMo --> counting */
void slow_func(Widget w, XtPointer voiddata, XtPointer voidcalldata)

{
  ref_image_data data;
  
  data= (ref_image_data)voiddata;
  
  data->slowMo^= TRUE;
  if (data->slowMo) {
    if (!data->counting) {
      // set also count button to "set" and counting to TRUE
      XtSetArg(args[0],XmNset,TRUE);
      XtSetValues(count_button,args,1);
      data->counting= TRUE;
    }
  }
}

/**********************************************************/
/* !counting --> !slowMo */
void count_func(Widget w, XtPointer voiddata, XtPointer voidcalldata)

{
  ref_image_data data;
  
  data= (ref_image_data)voiddata;
  
  data->counting^= TRUE;
  if (!data->counting) {
    if (data->slowMo) {
      // set also slow button to "unset" and slowMo to FALSE
      XtSetArg(args[0],XmNset,FALSE);
      XtSetValues(slow_button,args,1);
      data->slowMo= FALSE;
    }
  }
}

/**********************************************************/

void redisplay(Widget w, XtPointer voiddata, XtPointer voidcalldata)
 
{
  ref_image_data data;
  
  data= (ref_image_data)voiddata;
  
  XSetFunction(XtDisplay(w),data->gc,GXcopy);
  XCopyArea(XtDisplay(w), data->pix, XtWindow(w),data->gc,0,0,data->width,data->height,0,0);
  if (data->func == GXxor) {
    XSetFunction(XtDisplay(w),data->gc,GXxor);
  }
  else {
    if (data->isInvert) {
      XSetFunction(XtDisplay(w),data->gc,GXandInverted);
    }
    else {
      XSetFunction(XtDisplay(w),data->gc,GXor);
    }
  }
}

/*********************************************************/

void invert(Widget w, XtPointer voiddata, XtPointer voidcalldata)

{
  int i;
  ref_image_data data;
  XmDrawingAreaCallbackStruct *calldata;
  
  data= (ref_image_data)voiddata;
  calldata= (XmDrawingAreaCallbackStruct*)voidcalldata;
  
  /* fprintf(stderr,"%d %d %d\n",calldata->reason,
                         calldata->event->type,
                         calldata->window);
  */
  /* event->type = button-press has been 4
     event->type = button-release has been 5
     !! if released outside the event comes anyway !!
     thus action on PRESS:
  */
  if (calldata->event->type == 4) {
    XSetFunction(XtDisplay(w),data->gc,GXxor);
    for (i= 0; i < data->height; i++) {
      XDrawLine(XtDisplay(w),data->pix,data->gc,0,i,data->width-1,i);
    }
    data->isInvert= data->isInvert ^ 1;
    redisplay(w,voiddata,voidcalldata);
  }
}

/***********************************************************/

void quit(Widget w, XtPointer voiddata, XtPointer voidcalldata)

{
  ref_image_data data;
  
  data= (ref_image_data)voiddata;
  
  if (data != NULL) {
    fprintf(stderr,"vidiCE: quit:%s\n",data->message);
  }
  else {
    fprintf(stderr,"vidiCE: quit\n");
  }
  exit(0);
}

/***********************************************************/

void beep(Widget w, XtPointer voiddata, XtPointer voidcalldata)

{
  putc(7,stdout); fflush(stdout);
}

/***********************************************************/

void stop(Widget w, XtPointer voiddata, XtPointer voidcalldata)

{
  ref_image_data data;
  
  data= (ref_image_data)voiddata;
  
  if (feof(data->f)) /* f save to exist, see main */ {
    beep(w,voiddata,voidcalldata);
  }
  else {
    data->go= FALSE;
  }
}

/************************************************************/

void reset(Widget w, XtPointer voiddata, XtPointer voidcalldata)

{
  ref_image_data data;
  
  data= (ref_image_data)voiddata;
  
  rewind(data->f);
  rectCount= 0;
  sprintf(countStr,strans(CSF,s),rectCount);
  XmTextFieldSetString(number_field,countStr);
}

/**************************************************************/

void cont(Widget w, XtPointer voiddata, XtPointer voidcalldata)

{
  XEvent event;
  ref_image_data data;
  int nanoRet;
  
  data= (ref_image_data)voiddata;
  
  data->go= TRUE;
  while (data->go) {
    if (data->slowMo && sleepTime.tv_nsec > 499) {
      nanoRet= nanosleep(&sleepTime,NULL);
      if (nanoRet != 0) {
        fprintf(stderr,"WARNING: nanosleep FAILED!\n");
      }
    }
    step(w,voiddata,voidcalldata);
    if (XtPending()) {
      XtNextEvent(&event);
      XtDispatchEvent(&event);
    }
  }
  beep(w,voiddata,voidcalldata);
}

/****************************************************************/

void step(Widget w, XtPointer voiddata, XtPointer voidcalldata)

{
  data_rect datarect;
  int lux, luy, width, height, xhigh, yhigh;
  ref_image_data data;
  
  data= (ref_image_data)voiddata;
  
  // f save to exist, see main
  if (fread(&datarect,sizeof(datarect),1,data->f)) {
    rectCount++;
    
    // Begin data to X conversion
    lux= (int)((datarect.cx - datarect.ex - data->WorldXL) * data->HorScale + 0.5);
    if (data->MathOri) {
      luy= (int)((data->WorldYlength - (datarect.cy + datarect.ey - data->WorldYL)) * data->VerScale + 0.5);
    }
    else {
      luy= (int)((datarect.cy - datarect.ey - data->WorldYL) * data->VerScale + 0.5);
    }
    width= (int)(datarect.ex * 2.0 * data->HorScale + 0.5);
    height= (int)(datarect.ey * 2.0 * data->VerScale + 0.5);
    //   End data to X conversion
    
    if (!width && !height) {
      if (lux > -32767 && lux < 32767 && luy > -32767 && luy < 32767) {
        XDrawPoint(XtDisplay(w),data->pix, data->gc,lux,luy);
        XDrawPoint(XtDisplay(w),XtWindow(data->canvas), data->gc,lux,luy);
      }
    }
    else {
      if (!width) {
        yhigh= luy + height;
        if (lux > -32767 && lux < 32767) {
          if (luy < -32767) {
            luy= -32767;
          }
          if (yhigh > 32767) {
            yhigh= 32767;
          }
          XDrawLine(XtDisplay(w),data->pix,data->gc,lux,luy,lux,yhigh);
          XDrawLine(XtDisplay(w),XtWindow(data->canvas),data->gc,lux,luy,lux,yhigh);
        }
      }
      else {
        if (!height) {
          xhigh= lux + width;
          if (luy > -32767 && luy < 32767) {
            if (lux < -32767) {
              lux= -32767;
            }
            if (xhigh > 32767) {
              xhigh= 32767;
            }
            XDrawLine(XtDisplay(w),data->pix,data->gc,lux,luy,xhigh,luy);
            XDrawLine(XtDisplay(w),XtWindow(data->canvas),data->gc,lux,luy,xhigh,luy);
          }
        }
        else {
          if (lux < 32767  && luy < 32767) {
            if (lux < -32767) {
              lux= -32767;
            }
            if (width > 65535) {
              width= 65535;
            }
            if (luy < -32767) {
              luy= -32767;
            }
            if (height > 65535) {
              height= 65535;
            }
            XDrawRectangle(XtDisplay(w),data->pix,data->gc,lux,luy,width,height);
            XDrawRectangle(XtDisplay(w),XtWindow(data->canvas),data->gc,lux,luy,width,height);
          }
        }
      }
    }
    if (!data->go || data->counting) {
      sprintf(countStr,strans(CSF,s),rectCount);
      XmTextFieldSetString(number_field,countStr);
    }
  }
  else {
    data->go= FALSE;
    beep(w,voiddata,voidcalldata);
    // but counting should be updated (could have been off during cont)
    sprintf(countStr,strans(CSF,s),rectCount);
    XmTextFieldSetString(number_field,countStr);
  }
}

/*********************************************************************/

Rpint GetLOF(char *path) // GetLengthOfFile

{
  struct stat status;
  int ferr;
  
  ferr= stat(path,&status);
  if (ferr == -1) {
    return 0;
  }
  else {
    return status.st_size;
  }
}

/***********************************************************************/

int DecStrLen(int byteSize) {
  
  int bits= byteSize * 8;
  double v= pow(2.0,bits);
  int log10v;

  log10v= (int)(log10(v));
  return log10v + 1; // number of digits
}

/***********************************************************************/

