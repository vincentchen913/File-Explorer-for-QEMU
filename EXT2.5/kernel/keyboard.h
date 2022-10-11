#ifndef _KEYBOARD_H_
#define _KEYBOARD_H_

void kb_init(Shared<Display> dis, Shared<Node> dir, Shared<Ext2> ext);
void kbHandler();

#endif