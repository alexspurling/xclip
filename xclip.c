/*
 *
 *
 *  xclip.c - command line interface to X server selections
 *  Copyright (C) 2001 Kim Saunders
 *  Copyright (C) 2007-2008 Peter Åstrand
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <stdio.h>
#include <stdlib.h>

#include <X11/Xlib.h>
#include "Atoms.h"
#include "xcdef.h"
#include "xcprint.h"
#include "xclib.h"


/* Options that get set on the command line */
char *sdisp = NULL;        /* X display to connect to */
Atom sseln;                /* X selection to work with */
Atom target;

Display *dpy;            /* connection to X11 display */

static int doOut(Window win) {
    Atom sel_type = None;
    unsigned char *sel_buf;    /* buffer for selection data */
    unsigned long sel_len = 0;    /* length of sel_buf */
    XEvent evt;            /* X Event Structures */
    unsigned int context = XCLIB_XCOUT_NONE;

    while (1) {

        fprintf(stderr, "Loop.\n");

        /* only get an event if xcout() is doing something */
        if (context != XCLIB_XCOUT_NONE)
            XNextEvent(dpy, &evt);

        /* fetch the selection, or part of it */
        xcout(dpy, win, evt, sseln, target, &sel_type, &sel_buf, &sel_len, &context);

        if (context == XCLIB_XCOUT_BAD_TARGET) {
            /* no fallback available, exit with failure */
            // free(sel_buf);
            errconvsel(dpy, target, sseln);
            // errconvsel does not return but exits with EXIT_FAILURE
        }

        /* only continue if xcout() is doing something */
        if (context == XCLIB_XCOUT_NONE)
            break;
    }

    if (sel_len) {
        /* only print the buffer out, and free it, if it's not
         * empty
         */
        fwrite(sel_buf, sizeof(char), sel_len, stdout);

        free(sel_buf);
    }

    return EXIT_SUCCESS;
}

int main(int argc, char *argv[]) {
    /* Declare variables */
    Window win;            /* Window */
    int exit_code;

    /* Connect to the X server. */
    if ((dpy = XOpenDisplay(sdisp))) {
        /* successful */
        if (xcverb >= ODEBUG)
            fprintf(stderr, "Connected to X server.\n");
    } else {
        /* couldn't connect to X server. Print error and exit */
        errxdisplay(sdisp);
    }

    sseln = XA_CLIPBOARD(dpy);

    target = XInternAtom(dpy, "image/png", False);

    /* Create a window to trap events */
    win = XCreateSimpleWindow(dpy, DefaultRootWindow(dpy), 0, 0, 1, 1, 0, 0, 0);

    /* get events about property changes */
    XSelectInput(dpy, win, PropertyChangeMask);

    /* If we get an X error, catch it instead of barfing */
    XSetErrorHandler(xchandler);

    exit_code = doOut(win);

    /* Disconnect from the X server */
    XCloseDisplay(dpy);

    /* exit */
    return exit_code;
}
