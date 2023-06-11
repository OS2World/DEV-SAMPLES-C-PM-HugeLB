/* ********************************************************************** */
/*                                                                        */
/*   HugeLB   Main Module                                                 */
/*                                                                        */
/*       This sample program demostrates a technique for using owner-     */
/*   drawn list boxes to allow more list entries than would be possible   */
/*   with a normal list box, given the inherent 64k control data heap.    */
/*                                                                        */
/*      Although this sample will show how it is possible to have a large */
/*   number of list items in a list box, having a large number of list    */
/*   items is not necessarily recommended. Very large list boxes may      */
/*   be difficult for users to work with and search thru. It should be    */
/*   carefully considered whether or not there is a better way to display */
/*   the data.                                                            */
/*      Also it should be carefully consider whether to use a container   */
/*   control instead of a large owner-drawn list box. The container       */
/*   control does not have the 64K data heap limit and is extremely       */
/*   flexible and powerful. In most cases of a large amount of data to    */
/*   be displayed, a container would be the preferred control.            */
/*                                                                        */
/*       List boxes, in versions prior to and including 2.0, have a limit */
/*   of 64K of memory availble for maintaining items in the list. Text    */
/*   for the list item, in most cases, uses up the majority of this       */
/*   memory. Owner-drawn list boxes give the ability to maintain the list */
/*   item data in memory *external* to the list box control, thereby      */
/*   allowing more total list items to be inserted into the list.         */
/*       With owner-drawn list boxes, the system sends a WM_DRAWITEM      */
/*   to the owner of the list box (e.g. the dialog proc or the client     */
/*   window proc ) when an item needs to be draw(displayed) in the        */
/*   listbox window on the screen. At this stage, neither the system      */
/*   nor the list box control cares what is drawn or how the item is      */
/*   represented; the owner could draw a bitmap, an icon, a marker,       */
/*   colored text, or combination. This feature can be used to allow      */
/*   the text to be maintained in an external array. The list item itself */
/*   has no text. When the owner receives a WM_DRAWITEM message, it       */
/*   gets the ID (i.e. index) of the item that needs to be drawn. The     */
/*   owner can use the ID to look up the appropriate text string in       */
/*   the external array. It then draws the text onto the screen so that   */
/*   it appears in the list box window. Note that the owner is given      */
/*   all the elements necessary to draw the item (e.g. hps, bounding      */
/*   rect, etc).                                                          */
/*                                                                        */
/*       This technique could also be used in cases where the list        */
/*   box items need to be repeatedly shuffled and sorted to increase      */
/*   performance. To change an items order in a list box, normally        */
/*   involves deleting an re-inserting items into the list box. When      */
/*   this needs to be done a lot, it can be very expensive. However,      */
/*   by maintaining the text externally, only the indexing of the         */
/*   external text needs to be sorted or shuffled. No actual list box     */
/*   items need to be deleted, inserted, or moved. Only the index of      */
/*   the text logically associated with the item needs to be changed.     */
/*                                                                        */
/*     DISCLAIMER OF WARRANTIES.  The following [enclosed] code is        */
/*     sample code created by IBM Corporation. This sample code is not    */
/*     part of any standard or IBM product and is provided to you solely  */
/*     for  the purpose of assisting you in the development of your       */
/*     applications.  The code is provided "AS IS", without               */
/*     warranty of any kind.  IBM shall not be liable for any damages     */
/*     arising out of your use of the sample code, even if they have been */
/*     advised of the possibility of   such damages.                      */
/*                                                                        */
/* ********************************************************************** */


#define INCL_WIN
#define INCL_GPI
#define INCL_DOS
#define INCL_DOSMISC
#define INCL_DOSERRORS
#include <os2.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "HugeLB.h"


#define   ENTRY_CNT     2000         // number of list box entries
#define   ENTRY_LENGTH  128          // number of chars in each entry

        /* --------------------  Globals  ---------------------- */

HAB     hab;
HMQ     hmq;

PSZ    szMainTitle  = (PSZ) "Huge ListBox Sample" ;
PSZ    szErrorTitle = (PSZ) "HugeLB Error" ;

        

CHAR   entries[ENTRY_CNT][ENTRY_LENGTH] ;  // External list text array
             //  Note that this array is being allocated globally purely
             //   for simplicity. In production code, it probably should
             //   be dynamically allocated at runtime. 

        /* ----------------  Prototypes  ------------------------ */
MRESULT EXPENTRY DlgWindowProc( HWND, USHORT, MPARAM, MPARAM );
 


/* ********************************************************************** */
/*                                                                        */
/*   Main                                                                 */
/*                                                                        */
/*      The program uses a dialog window as it's main window for          */
/*   simplicity; it allows the list box to be displayed but otherwise     */
/*   is not very flexible. Note that the dialog window will register      */
/*   with the task manager (i.e. task window). Error checking is weak.    */
/*                                                                        */
/* ********************************************************************** */

int main (void)
{

  if ( (hab = WinInitialize( 0L )) == (HAB) NULL ){
     printf( "HugeLB Error:  WinInitialize failed \n" );
     return 0;
  }
  else {

     if ( (hmq = WinCreateMsgQueue( hab, 0 )) == (HMQ) NULL ){
        printf( "Shell Error:  WinCreateMsgQueue failed \n" );
        return 0;
     }
     else {
         WinDlgBox( HWND_DESKTOP,
                    HWND_DESKTOP,
                    (PFNWP)DlgWindowProc,
                    NULLHANDLE,
                    ID_DLG_WIN,
                    (PVOID)NULL );

        WinDestroyMsgQueue(hmq);
     }  /* end of else ( ...WinCreateMsgQueue() */
     WinTerminate(hab);
   }  /* end of else (...WinInitialize(NULL) */
}  /*  end of main() */

/* ********************************************************************** */
/*                                                                        */
/*   DlgWindowProc                                                        */
/*                                                                        */
/* ********************************************************************** */

MRESULT EXPENTRY
DlgWindowProc( HWND hwnd, USHORT msg, MPARAM mp1, MPARAM mp2 )
{

  switch (msg) {

       /* --------------------------------------------------------- */
       /*  WM_INITDLG                                               */
       /*                                                           */
       /*     During this message we will register the dialog       */
       /*  window with the task manager. We will also initialize    */
       /*  both the external data array and the list box itself.    */
       /*  We are doing the initialization here for simpilicity;    */
       /*  it would be best to do this in a seperate thread         */
       /*  rather than presenting the wait pointer for an extended  */
       /*  period.                                                  */
       /*                                                           */
       /*     The text string will consist of a number indicating   */
       /*  the index number of the item and a series of '.'s to     */
       /*  fill out the string length                               */
       /*                                                           */
       /*     We will insert items into the list; note that the     */
       /*  text inserted with each item is blank.                   */
       /*                                                           */
       /* --------------------------------------------------------- */
    case WM_INITDLG:
       {
         USHORT    i;
         HPOINTER  hptrOrig, hptrWait;
         PID       pid ;
         SWCNTRL   swCntrl;

            /* ---- set Wait pointer for initialization period ---- */
         hptrWait = WinQuerySysPointer( HWND_DESKTOP, SPTR_WAIT, FALSE );
         hptrOrig = WinQueryPointer( HWND_DESKTOP );
         WinSetPointer( HWND_DESKTOP, hptrWait );


            /* ----------- add program to tasklist  --------------- */
         WinQueryWindowProcess( hwnd, &pid, NULL );
         swCntrl.hwnd = hwnd ;
         swCntrl.hwndIcon = (HWND) NULL ;
         swCntrl.hprog = (HPROGRAM) NULL ;
         swCntrl.idProcess = pid ;
         swCntrl.idSession = (LONG) NULL ;
         swCntrl.uchVisibility = SWL_VISIBLE ;
         swCntrl.fbJump = SWL_JUMPABLE ;
         strncpy( swCntrl.szSwtitle, (const char *) szMainTitle, MAXNAMEL );
         WinCreateSwitchEntry( hab, (PSWCNTRL)&swCntrl);

            /* -- initialize external text array and insert list items -- */
         memset( entries[0], '.', ENTRY_CNT*ENTRY_LENGTH );
         for( i=0; i < ENTRY_CNT; i++ ){

            _itoa( i, entries[i], 10 );            // initialize external text
            entries[i][strlen( entries[i])] = '.' ;
            entries[i][ENTRY_LENGTH-1] = '\0' ;

            WinSendDlgItemMsg( hwnd,              // insert "no text" item
                               ID_LISTBOX,
                               LM_INSERTITEM,
                               MPFROMSHORT( LIT_END ),
                               MPFROMP( (PVOID)"" ) );  //note: empty string
         }

         WinSetPointer( HWND_DESKTOP, hptrOrig );
       }
       break;

       /* --------------------------------------------------------- */
       /*                                                           */
       /*  WM_MEASUREITEM                                           */
       /*                                                           */
       /*    With this message the system is asking for the height  */
       /*  and width of a list box item. We are not using a         */
       /*  horizontal scrollbar (i.e. LS_HORZSCROLL) so we will     */
       /*  not have to specify a width, only a height.              */
       /*                                                           */
       /*    The height we need to specify is the height of the     */
       /*  character box of the font that will be used, in our      */
       /*  case the default font. We can get this char box height   */
       /*  from the Max Baseline Extent metric.                     */
       /*                                                           */
       /* --------------------------------------------------------- */
     case WM_MEASUREITEM:           
        {
           HPS           hps ;
           FONTMETRICS   fmMetrics ;


            hps = WinGetPS (hwnd);
               /* ----------- get metrics of default font  ------------ */
            GpiQueryFontMetrics (hps,
                                 sizeof (FONTMETRICS), 
                                 &fmMetrics);
            WinReleasePS (hps);     

            return ((MRESULT)fmMetrics.lMaxBaselineExt);
         }

       /* --------------------------------------------------------- */
       /*                                                           */
       /*  WM_DRAWITEM                                              */
       /*                                                           */
       /*     With this message the system requests that we draw    */
       /*  a representation for a given list box item. In our case  */
       /*  we will draw the text from the external text array. We   */
       /*  will use the ID(index) number of the list box item as    */
       /*  an index into the text array.                            */
       /*                                                           */
       /*     Note that we much check the selection state of the    */
       /*  item to determine if it should be drawn with a highlight */
       /*  or not.                                                  */
       /*                                                           */
       /* --------------------------------------------------------- */
        case WM_DRAWITEM:
         {
           POWNERITEM  poiItem;                
           COLOR       clrForeGround;
           COLOR       clrBackGround;

            poiItem = (POWNERITEM) PVOIDFROMMP( mp2 );

               /* ---- check selection state of the item  ---- */
            if (poiItem->fsState ) {
                clrForeGround = SYSCLR_HILITEFOREGROUND;
                clrBackGround = SYSCLR_HILITEBACKGROUND;
                poiItem->fsState = FALSE;   //clear so PM won't hilite also
            }
            else                       
            {
                clrForeGround = CLR_NEUTRAL;          // normal colors
                clrBackGround = SYSCLR_ENTRYFIELD;
            }

                /* ---- draw the text into the given rect ----- */
            WinDrawText (poiItem->hps,    
                         -1,           
                         (PCCH) entries[poiItem->idItem], 
                         &poiItem->rclItem,        
                         clrForeGround,
                         clrBackGround,
                         DT_LEFT | DT_VCENTER | DT_ERASERECT);

            poiItem->fsStateOld = FALSE;    //clear so PM won't hilite
            return ((MRESULT)TRUE);
         }


    case WM_COMMAND :
       switch( SHORT1FROMMP( mp1 ) ){
          case DID_OK :
             WinDismissDlg( hwnd, DID_OK );
           break;

          default:
          return WinDefDlgProc(hwnd,msg,mp1,mp2);
        }
    break;

    default:
      return WinDefDlgProc(hwnd,msg,mp1,mp2);

  } /*  end of switch () */
  return( FALSE );

} /*  end of DlgWindowProc */

