#include <stdio.h>
#include "termbox/termbox.c"
#include "config.c"
#include "types.c"
#include "globals.c"
#include "core.c"
#include "cmd.c"
#include "file.c"
#include "dialog.c"
#include "shared.c"
#include "selection.c"
#include "draw.c"
#include "cedit.h"

/*
 * initializes termbox
 * loads all files defined in argv
 * if no file is loaded a new one will be created
 * if multiple files are loaded the first file will become the current file
 *
 * waiting for input
 */
int main(int argc, char *argv[])
{
	int ret;
	size_t i;
	struct tb_event ev;

	ret = tb_init();
	if(ret) fprintf(stderr, "tb_init() failed with error code %d\n", ret);

	if(argc > 1) for(i = 1 ; i < argc ; i++) file_open(argv[i]);
	if(CF == 0) file_new();

	file_switch(1);

	while (tb_poll_event(&ev)) {
		switch(ev.type) {
		case TB_EVENT_KEY:
		if(CMD) {
		 ///////////////////////// Command Mode /////////////////////////
		 if(ev.key == TB_KEY_ENTER )      { cmd_exec();           break;}
		 if(ev.key == TB_KEY_ARROW_RIGHT) { cmd_input_right();    break;}
		 if(ev.key == TB_KEY_ARROW_LEFT)  { cmd_input_left();     break;}
		 if(ev.key == TB_KEY_SPACE)       { cmd_input_add(32);    break;}
		 if(ev.key == TB_KEY_ESC)         { cmd_del();            break;}
		 if(ev.key == TB_KEY_BACKSPACE2){
			if(0 < CMD->cur)          { cmd_input_del();}	  break;}
		 if (ev.ch != 0){
			if(CMD->input == 0){
				cmd_input_add(ev.ch);
			} else {
				if(ev.ch==48||ev.ch==49||ev.ch==50||ev.ch==51||
				   ev.ch==52||ev.ch==53||ev.ch==54||ev.ch==55||
				   ev.ch==56||ev.ch==57) cmd_input_add(ev.ch);
			}
		 } break;
		} else {
		 //////////////////////// CLOSE & QUIT //////////////////////////
		 if(ev.key == TB_KEY_ESC) {
			/* delete selection */
			if(CF->sel != 0){
				selection_del();

			/* close file / quit cedit */
			} else {
				file_close();
				if (CF == 0) {
					if(CLIP != 0)selection_free_clipboard();
					tb_shutdown();
					return 0;
				}
			}
			draw_all();
			break;
		 }
		 ////////////////////////////KEYBINDS////////////////////////////
		 if(ev.key == TB_KEY_CTRL_O) { file_open(0);              break;}
		 if(ev.key == TB_KEY_CTRL_S) { file_save(0);              break;}
		 if(ev.key == TB_KEY_CTRL_N) { file_new();                break;}
		 if(ev.key == TB_KEY_CTRL_D) { core_remove_line();        break;}
		 if(ev.key == TB_KEY_CTRL_V) { selection_paste();         break;}
		 ///////////////////////SELECTION KEYBINDS///////////////////////
		 if(CF->sel != 0){
			if(ev.key == TB_KEY_CTRL_C) { selection_copy();  break;}
		 } else {
			if(ev.key == TB_KEY_CTRL_C) { selection_new();   break;}
		 }
		 /////////////////////////// END/POS1 ///////////////////////////
		 if(ev.key == TB_KEY_HOME)   { core_pos1();               break;}
		 if(ev.key == TB_KEY_END)    { core_end();                break;}
		 ///////////////////////// PAGE UP/DOWN /////////////////////////
		 if(ev.key == TB_KEY_PGUP)   { core_pageup();             break;}
		 if(ev.key == TB_KEY_PGDN)   { core_pagedown();           break;}
		 ////////////////////////FILE SWITCHING//////////////////////////
		 if(ev.key == TB_KEY_CTRL_H) { file_switch_left();        break;}
		 if(ev.key == TB_KEY_CTRL_L) { file_switch_right();       break;}
		 if(ev.key == TB_KEY_F1)     { file_switch(1);            break;}
		 if(ev.key == TB_KEY_F2)     { file_switch(2);            break;}
		 if(ev.key == TB_KEY_F3)     { file_switch(3);            break;}
		 if(ev.key == TB_KEY_F4)     { file_switch(4);            break;}
		 if(ev.key == TB_KEY_F5)     { file_switch(5);            break;}
		 if(ev.key == TB_KEY_F6)     { file_switch(6);            break;}
		 if(ev.key == TB_KEY_F7)     { file_switch(7);            break;}
		 if(ev.key == TB_KEY_F8)     { file_switch(8);            break;}
		 if(ev.key == TB_KEY_F9)     { file_switch(9);            break;}
		 if(ev.key == TB_KEY_F10)    { file_switch(10);           break;}
		 if(ev.key == TB_KEY_F11)    { file_switch(11);           break;}
		 if(ev.key == TB_KEY_F12)    { file_switch(12);           break;}
		 /////////////////////////////ARROWS/////////////////////////////
		 if (ev.key == TB_KEY_ARROW_RIGHT) { core_right();        break;}
		 if (ev.key == TB_KEY_ARROW_LEFT)  { core_left();         break;}
		 if (ev.key == TB_KEY_ARROW_UP)    { core_up();           break;}
		 if (ev.key == TB_KEY_ARROW_DOWN)  { core_down();         break;}
		 /////////////////////////////INPUT//////////////////////////////
		 if (ev.key == TB_KEY_BACKSPACE2 ) {
			if(0 < CF->cur->p) {                   core_del_char();
			} else if(CF->cur->l->prev != 0) {     core_del_line();}
			break;
		 }
		 if (ev.key == TB_KEY_ENTER)  { core_new_line();          break;}
		 if (ev.key == TB_KEY_SPACE)  { core_add_char(32);        break;}
		 if (ev.key == TB_KEY_TAB)    { core_add_char(9);         break;}
		 if (ev.ch != 0){core_add_char(ev.ch);}                   break;
		}
		////////////////////////////////////////////////////////////////
		case TB_EVENT_RESIZE: draw_all();                         break;
		}
	}
}
