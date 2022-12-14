/* config.c - MemTest-86  Version 3.4
 *
 * Released under version 2 of the Gnu Public License.
 * By Chris Brady
 * ----------------------------------------------------
 * MemTest86+ V5.00 Specific code (GPL V2.0)
 * By Samuel DEMEULEMEESTER, sdemeule@memtest.org
 * http://www.x86-secret.com - http://www.memtest.org
 */
#include "test.h"
#include "screen_buffer.h"
#include "dmi.h"

extern int bail, beepmode;
extern struct tseq tseq[];
extern short e820_nr;
void performance();
extern volatile short cpu_mode;
extern volatile int test;
extern void find_chunks();
extern volatile short start_seq;
extern short restart_flag;
extern short onepass;
extern short btflag;

extern void get_list(int x, int y, int len, char *buf);

/* declared in test.h */
char pop_save_buffer_1[2][POP_H][POP_W];
char pop_save_buffer_2[2][POP2_H][POP2_W];

void get_config(void)
{
	int flag = 0, sflag = 0, i, j, k, n, m, prt = 0;
	int reprint_screen = 0;
	char cp[64];
	ulong page;

	popup(POP_SAVE_BUFFER_1);
	wait_keyup();
	while (!flag) {
		cprint(POP_Y+1,  POP_X+2, "Settings:");
		cprint(POP_Y+3,  POP_X+6, "(1) Test Selection");
		cprint(POP_Y+4,  POP_X+6, "(2) Address Range");
		cprint(POP_Y+5,  POP_X+6, "(3) Error Report Mode");
		cprint(POP_Y+6,  POP_X+6, "(4) Core Selection");
		cprint(POP_Y+7,  POP_X+6, "(5) Refresh Screen");
		cprint(POP_Y+8,  POP_X+6, "(6) Display DMI Data");
#if !(CB_NOSPD)
		cprint(POP_Y+9,  POP_X+6, "(7) Display SPD Data");
#endif
		cprint(POP_Y+11, POP_X+6, "(0) Continue");

		/* Wait for key release */
		/* Fooey! This nuts'es up the serial input. */
		sflag = 0;
		switch (get_key()) {
		case 2:
			/* 1 - Test Selection */
			popclear(POP_SAVE_BUFFER_1);
			cprint(POP_Y+1, POP_X+2, "Test Selection:");
			cprint(POP_Y+3, POP_X+6, "(1) Default Tests");
			cprint(POP_Y+4, POP_X+6, "(2) Skip Current Test");
			cprint(POP_Y+5, POP_X+6, "(3) Select Test");
			cprint(POP_Y+6, POP_X+6, "(4) Enter Test List");
			cprint(POP_Y+7, POP_X+6, "(0) Cancel");
			if (vv->testsel < 0) {
				cprint(POP_Y+3, POP_X+5, ">");
			} else {
				cprint(POP_Y+5, POP_X+5, ">");
			}
			wait_keyup();
			while (!sflag) {
				switch (get_key()) {
				case 2:
					/* Default - All tests */
					i = 0;
					while (tseq[i].cpu_sel) {
						tseq[i].sel = 1;
						i++;
					}
					find_ticks_for_pass();
					sflag++;
					break;
				case 3:
					/* Skip test */
					bail++;
					sflag++;
					break;
				case 4:
					/* Select test */
					popclear(POP_SAVE_BUFFER_1);
					cprint(POP_Y+1, POP_X+3,
					       "Test Selection:");
					cprint(POP_Y+4, POP_X+5,
					       "Test Number [1-11]: ");
					n = getval(POP_Y+4, POP_X+24, 0) - 1;
					if (n <= 11) {
						/* Deselect all tests */
						i = 0;
						while (tseq[i].cpu_sel) {
							tseq[i].sel = 0;
							i++;
						}
						/* Now set the selection */
						tseq[n].sel = 1;
						vv->pass = -1;
						test = n;
						find_ticks_for_pass();
						sflag++;
						bail++;
					}
					break;
				case 5:
					/* Enter a test list */
					popclear(POP_SAVE_BUFFER_1);
					cprint(POP_Y+1, POP_X+3,
					       "Enter a comma separated list");
					cprint(POP_Y+2, POP_X+3,
					       "of tests to execute:");
					cprint(POP_Y+5, POP_X+5, "List: ");
					/* Deselect all tests */
					k = 0;
					while (tseq[k].cpu_sel) {
						tseq[k].sel = 0;
						k++;
					}

					/* Get the list */
					for (i=0; i<64; i++) cp[i] = 0;
					get_list(POP_Y+5, POP_X+10, 64, cp);

					/* Now enable all of the tests in the
					 * list */
					i = j = m = 0;
					while (1) {
						if (mt86_isdigit(cp[i])) {
							n = cp[i]-'0';
							j = j*10 + n;
							i++;
							if (cp[i] == ',' || cp[i] == 0) {
								if (j < k) {
									tseq[j].sel = 1;
									m++;
								}
								if (cp[i] == 0) break;
								j = 0;
								i++;
							}
						}
					}

					/* If we didn't select at least one
					 * test turn them all back on */
					if (m == 0) {
						k = 0;
						while (tseq[k].cpu_sel) {
							tseq[k].sel = 1;
							k++;
						}
					}
					vv->pass = -1;
					test = n;
					find_ticks_for_pass();
					sflag++;
					bail++;
					break;
				case 11:
				case 57:
					sflag++;
					break;
				}
			}
			popclear(POP_SAVE_BUFFER_1);
			break;
		case 3:
			/* 2 - Address Range */
			popclear(POP_SAVE_BUFFER_1);
			cprint(POP_Y+1, POP_X+2, "Test Address Range:");
			cprint(POP_Y+3, POP_X+6, "(1) Set Lower Limit");
			cprint(POP_Y+4, POP_X+6, "(2) Set Upper Limit");
			cprint(POP_Y+5, POP_X+6, "(3) Test All Memory");
			cprint(POP_Y+6, POP_X+6, "(0) Cancel");
			wait_keyup();
			while (!sflag) {
				switch (get_key()) {
				case 2:
					/* Lower Limit */
					popclear(POP_SAVE_BUFFER_1);
					cprint(POP_Y+2, POP_X+4,
					       "Lower Limit: ");
					cprint(POP_Y+4, POP_X+4,
					       "Current: ");
					aprint(POP_Y+4, POP_X+13, vv->plim_lower);
					cprint(POP_Y+6, POP_X+4,
					       "New: ");
					page = getval(POP_Y+6, POP_X+9, 12);
					if (page + 1 <= vv->plim_upper) {
						vv->plim_lower = page;
						test--;
						bail++;
					}
					adj_mem();
					find_chunks();
					find_ticks_for_pass();
					sflag++;
					break;
				case 3:
					/* Upper Limit */
					popclear(POP_SAVE_BUFFER_1);
					cprint(POP_Y+2, POP_X+4,
					       "Upper Limit: ");
					cprint(POP_Y+4, POP_X+4,
					       "Current: ");
					aprint(POP_Y+4, POP_X+13, vv->plim_upper);
					cprint(POP_Y+6, POP_X+4,
					       "New: ");
					page = getval(POP_Y+6, POP_X+9, 12);
					if (page - 1 >= vv->plim_lower) {
						vv->plim_upper = page;
						bail++;
						test--;
					}
					adj_mem();
					find_chunks();
					find_ticks_for_pass();
					sflag++;
					break;
				case 4:
					/* All of memory */
					vv->plim_lower = 0;
					vv->plim_upper =
						vv->pmap[vv->msegs - 1].end;
					test--;
					bail++;
					adj_mem();
					find_chunks();
					find_ticks_for_pass();
					sflag++;
					break;
				case 11:
				case 57:
					/* 0/CR - Continue */
					sflag++;
					break;
				}
			}
			popclear(POP_SAVE_BUFFER_1);
			break;
		case 4:
			/* Error Mode */
			popclear(POP_SAVE_BUFFER_1);
			cprint(POP_Y+1, POP_X+2, "Printing Mode:");
			cprint(POP_Y+3, POP_X+6, "(1) Error Summary");
			cprint(POP_Y+4, POP_X+6, "(2) Individual Errors");
			cprint(POP_Y+5, POP_X+6, "(3) BadRAM Patterns");
			cprint(POP_Y+6, POP_X+6, "(4) Error Counts Only");
			cprint(POP_Y+7, POP_X+6, "(5) Beep on Error");
			cprint(POP_Y+8, POP_X+6, "(0) Cancel");
			cprint(POP_Y+3+vv->printmode, POP_X+5, ">");
			if (beepmode) { cprint(POP_Y+7, POP_X+5, ">"); }
			wait_keyup();
			while (!sflag) {
				switch (get_key()) {
				case 2:
					/* Error Summary */
					vv->printmode=PRINTMODE_SUMMARY;
					vv->erri.eadr = 0;
					vv->erri.hdr_flag = 0;
					sflag++;
					break;
				case 3:
					/* Separate Addresses */
					vv->printmode=PRINTMODE_ADDRESSES;
					vv->erri.eadr = 0;
					vv->erri.hdr_flag = 0;
					vv->msg_line = LINE_SCROLL-1;
					sflag++;
					break;
				case 4:
					/* BadRAM Patterns */
					vv->printmode=PRINTMODE_PATTERNS;
					vv->erri.hdr_flag = 0;
					sflag++;
					prt++;
					break;
				case 5:
					/* Error Counts Only */
					vv->printmode=PRINTMODE_NONE;
					vv->erri.hdr_flag = 0;
					sflag++;
					break;
				case 6:
					/* Set Beep On Error mode */
					beepmode = !beepmode;
					sflag++;
					break;
				case 11:
				case 57:
					/* 0/CR - Continue */
					sflag++;
					break;
				}
			}
			popclear(POP_SAVE_BUFFER_1);
			break;
		case 5:
			/* CPU Mode */
			reprint_screen = 1;
			popclear(POP_SAVE_BUFFER_1);
			cprint(POP_Y+1, POP_X+2, "CPU Selection Mode:");
			cprint(POP_Y+3, POP_X+6, "(1) Parallel (All)");
			cprint(POP_Y+4, POP_X+6, "(2) Round Robin (RRb)");
			cprint(POP_Y+5, POP_X+6, "(3) Sequential (Seq)");
			cprint(POP_Y+6, POP_X+6, "(0) Cancel");
			cprint(POP_Y+2+cpu_mode, POP_X+5, ">");
			wait_keyup();
			while (!sflag) {
				switch (get_key()) {
				case 2:
					if (cpu_mode != CPM_ALL) bail++;
					cpu_mode = CPM_ALL;
					sflag++;
					popdown(POP_SAVE_BUFFER_1);
					cprint(9, 34, "All");
					popup(POP_SAVE_BUFFER_1);
					break;
				case 3:
					if (cpu_mode != CPM_RROBIN) bail++;
					cpu_mode = CPM_RROBIN;
					sflag++;
					popdown(POP_SAVE_BUFFER_1);
					cprint(9, 34, "RRb");
					popup(POP_SAVE_BUFFER_1);
					break;
				case 4:
					if (cpu_mode != CPM_SEQ) bail++;
					cpu_mode = CPM_SEQ;
					sflag++;
					popdown(POP_SAVE_BUFFER_1);
					cprint(9, 34, "Seq");
					popup(POP_SAVE_BUFFER_1);
					break;
				case 11:
				case 57:
					/* 0/CR - Continue */
					sflag++;
					break;
				}
			}
			popclear(POP_SAVE_BUFFER_1);
			break;
		case 6:
			reprint_screen = 1;
			flag++;
			break;
		case 7:
			/* Display DMI Memory Info */
			popup(POP_SAVE_BUFFER_2);
			print_dmi_info();
			popdown(POP_SAVE_BUFFER_2);
			break;
		case 8:
			/* Display SPD Data */
			popup(POP_SAVE_BUFFER_2);
			show_spd();
			popdown(POP_SAVE_BUFFER_2);
			sflag++;
			break;
		case 11:
		case 57:
		case 28:
			/* 0/CR/SP - Continue */
			flag++;
			break;
		}
	}
	popdown(POP_SAVE_BUFFER_1);
	if (prt) {
		printpatn();
	}
	if (reprint_screen) {
		tty_print_screen();
	}
}

void popup(int pop_x, int pop_y, int pop_h, int pop_w, char pop_save_buffer[2][pop_h][pop_w])
{
	int i, j;
	char *pp;

	for (i = pop_y; i < pop_y + pop_h; i++) {
		for (j = pop_x; j < pop_x + pop_w; j++) {
			/* Point to the write position in the screen */
			pp = (char *)(SCREEN_ADR + (i * 160) + (j * 2));

			/* pp and get_scrn_buf(i,j) should be equal here
			 * except on board that don't have screen (e.g. only serial port)
			 * that's why we save the screen buffer:
			 * save screen buffer */
			pop_save_buffer[0][i - pop_y][j - pop_x] = get_scrn_buf(i, j);
			*pp = ' ';               /* Clear screen */
			set_scrn_buf(i, j, ' '); /* Clear screen buffer */
			pp++;
			pop_save_buffer[1][i - pop_y][j - pop_x] = *pp; /* Save screen background color */
			*pp = 0x07;             /* Change screen Background to black */
		}
	}
	/* print the screen buffer in the serial console */
	tty_print_region(pop_y, pop_x, pop_y+pop_h, pop_x+pop_w);
}

void popdown(int pop_x, int pop_y, int pop_h, int pop_w, char pop_save_buffer[2][pop_h][pop_w])
{
	int i, j;
	char *pp;

	for (i = pop_y; i < pop_y + pop_h; i++) {
		for (j = pop_x; j < pop_x + pop_w; j++) {
			/* Point to the write position in the screen */
			pp = (char *)(SCREEN_ADR + (i * 160) + (j * 2));
			*pp = pop_save_buffer[0][i-pop_y][j-pop_x]; /* Restore screen */
			set_scrn_buf(i, j, pop_save_buffer[0][i - pop_y][j - pop_x]); /* Restore the screen buffer*/
			pp++;
			*pp = pop_save_buffer[1][i - pop_y][j - pop_x]; /* Restore screen color */
		}
	}
	/* print the screen buffer in the serial console */
	tty_print_region(pop_y, pop_x, pop_y+pop_h, pop_x+pop_w);
}

void popclear(int pop_x, int pop_y, int pop_h, int pop_w, char pop_save_buffer[2][pop_h][pop_w])
{
	int i, j;
	char *pp;

	for (i = pop_y; i < pop_y + pop_h; i++) {
		for (j  =pop_x; j < pop_x + pop_w; j++) {
			/* Point to the write position in the screen */
			pp = (char *)(SCREEN_ADR + (i * 160) + (j * 2));
			*pp = ' ';               /* Clear screen */
			set_scrn_buf(i, j, ' '); /* Clear screen buffer */
			pp++;
		}
	}
	/* print the screen buffer in the serial console */
	tty_print_region(pop_y, pop_x, pop_y+pop_h, pop_x+pop_w);
}

void adj_mem(void)
{
	int i;

	vv->selected_pages = 0;
	for (i=0; i< vv->msegs; i++) {
		/* Segment inside limits ? */
		if (vv->pmap[i].start >= vv->plim_lower &&
		    vv->pmap[i].end <= vv->plim_upper) {
			vv->selected_pages += (vv->pmap[i].end - vv->pmap[i].start);
			continue;
		}
		/* Segment starts below limit? */
		if (vv->pmap[i].start < vv->plim_lower) {
			/* Also ends below limit? */
			if (vv->pmap[i].end < vv->plim_lower) {
				continue;
			}

			/* Ends past upper limit? */
			if (vv->pmap[i].end > vv->plim_upper) {
				vv->selected_pages +=
					vv->plim_upper - vv->plim_lower;
			} else {
				/* Straddles lower limit */
				vv->selected_pages +=
					(vv->pmap[i].end - vv->plim_lower);
			}
			continue;
		}
		/* Segment ends above limit? */
		if (vv->pmap[i].end > vv->plim_upper) {
			/* Also starts above limit? */
			if (vv->pmap[i].start > vv->plim_upper) {
				continue;
			}
			/* Straddles upper limit */
			vv->selected_pages +=
				(vv->plim_upper - vv->pmap[i].start);
		}
	}
}
