#include "cedit.h"

/*
 * adds an utf-8 character to the current file by shifting all characters from
 * the right side of the cursor len to the right and writing in the gap aftwards
 */
void core_add_char(uint32_t ch)
{
	Line *line;
	char buf[6];

	size_t i;
	size_t len;
	size_t bpos;

	line = CF->cur->l;
	len = tb_utf8_unicode_to_char(buf, ch);
	bpos = misc_utf8_bytepos(line->c, CF->cur->p);
	core_ensure_cap(line, line->blen+len);

	for(i = line->blen; i > bpos; i--) line->c[i] = line->c[i-len];
	for(i = bpos; i < bpos+len ; i++) line->c[i] = buf[i-bpos];

	CF->cur->l->blen += len;
	CF->cur->l->clen += 1;
	CF->cur->p += 1;

	draw_all();
}

/*
 * removes an utf-8 character from the current file by shifting all characters
 * from the right side of the cursor len to the left overwriting the character
 */
void core_del_char()
{
	Line *line;

	size_t i;
	size_t len;
	size_t bpos;

	line = CF->cur->l;
	bpos = misc_utf8_bytepos(line->c, CF->cur->p-1);
	len = tb_utf8_char_length(line->c[bpos]);

	for(i = bpos; i < line->blen; i++) line->c[i] = line->c[i+len];

	CF->cur->l->blen -= len;
	CF->cur->l->clen -= 1;
	CF->cur->p -= 1;

	draw_all();
}

/*
 * creates a new line copying characters behind cursor into the next line
 */
void core_new_line()
{
	Line *line;

	size_t i;
	size_t bpos;

	bpos = misc_utf8_bytepos(CF->cur->l->c, CF->cur->p);

	line = malloc(sizeof(Line));
	line->c = malloc(LINESIZE);
	line->mlen = LINESIZE;
	core_ensure_cap(line, CF->cur->l->blen-bpos);

	for(i=bpos;i<CF->cur->l->blen;i++) line->c[i-bpos] = CF->cur->l->c[i];
	for(i=bpos;i<CF->cur->l->mlen;i++) CF->cur->l->c[i] = 0;

	line->blen = strlen(line->c);
	line->clen = CF->cur->l->clen - CF->cur->p;
	line->next = CF->cur->l->next;
	line->prev = CF->cur->l;

	line->prev->next = line;
	if(line->next != 0) line->next->prev = line;

	CF->cur->l->blen -= line->blen;
	CF->cur->l->clen -= line->clen;

	CF->cur->l->next = line;
	CF->cur->l = CF->cur->l->next;
	CF->cur->p = 0;

	draw_all();
}

/*
 * deletes a line, copying characters left in line into the previous line
 */
void core_del_line()
{
	Line *line;
	size_t bpos;

	line = CF->cur->l;

	line->prev->next = line->next;
	if (line->next != 0) line->next->prev = line->prev;

	bpos = misc_utf8_bytepos(CF->cur->l->c, CF->cur->p);
	core_ensure_cap(line->prev, line->blen + line->prev->blen);

	CF->cur->p = line->prev->clen;

	strncat(line->prev->c, line->c, line->blen);
	CF->cur->l = CF->cur->l->prev;

	line->prev->blen = line->blen + line->prev->blen;
	line->prev->clen = line->clen + line->prev->clen;

	free(line->c);
	free(line);

	draw_all();
}

/*
 * cursor handling: right, left, up, down
 */
void core_right()
{
	if(CF->cur->l->clen > CF->cur->p){
		CF->cur->p++;
	} else if(CF->cur->l->next != 0){
		CF->cur->l = CF->cur->l->next;
		CF->cur->p = 0;
	}
	draw_all();
}
void core_left()
{
	if(0 < CF->cur->p) {
		CF->cur->p--;
	} else if(CF->cur->l->prev != 0){
		CF->cur->l = CF->cur->l->prev;
		CF->cur->p = CF->cur->l->clen;
	}
	draw_all();
}
void core_up()
{
	if(CF->cur->l->prev != 0) {
		core_change_line(CF->cur->l->prev);
	}
	draw_all();
}
void core_down()
{
	if(CF->cur->l->next != 0) {
		core_change_line(CF->cur->l->next);
	}
	draw_all();
}

/*
 * calculate the correct real cursor position depending on displayed cursor
 * position therefore handling tabs correctly
 * sets the new line as current line
 */
void core_change_line(Line *line)
{
	size_t len;
	size_t pc;
	size_t cc;
	size_t cd;
	size_t bc;
	size_t tc;

	cc = 0;
	bc = 0;
	tc = TABSIZE;
	for(pc = 0; bc < CF->cur->l->blen && pc < CF->cur->p; pc++){
		len = tb_utf8_char_length(CF->cur->l->c[bc]);
		if(len == 1 && CF->cur->l->c[bc] == '\t'){
			cc += tc;
			tc = 0;
		} else {
			tc--;
			cc++;
		}
		bc += len;
		if(tc == 0) tc = TABSIZE;
	}

	cd = 0;
	bc = 0;
	tc = TABSIZE;
	for(pc = 0; bc < line->blen; pc++){
		len = tb_utf8_char_length(line->c[bc]);
		if(len == 1 && line->c[bc] == '\t'){
			if(cd + tc > cc) break;
			cd += tc;
			tc = 0;
		} else {
			if(cd + 1 > cc) break;
			tc--;
			cd++;
		}
		bc += len;
		if(tc == 0) tc = TABSIZE;
	}

	CF->cur->l = line;
	CF->cur->p = pc;
}

/*
 * ensures that there are always enough bytes to write new content to
 */
void core_ensure_cap(Line *line, size_t cap)
{
	size_t i;
	i = 0;
	while (line->mlen+i < cap) i += LINESIZE;
	if (i > 0){
		line->mlen += i;
		line->c = realloc(line->c,line->mlen);
	}
}