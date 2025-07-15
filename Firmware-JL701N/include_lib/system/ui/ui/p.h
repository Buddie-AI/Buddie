#ifndef UI_P_H
#define UI_P_H

#include "ui/ui_core.h"

struct ui_str {
    const char *format;
    char *str;
};

struct element_text {
    struct element elm;               //must be first
    char *str;
    const char *format;
    void *priv;
    int color;
    const struct element_event_handler *handler;
};



void text_element_set_text(struct element_text *text, char *str,
                           const char *format, int color);


void text_element_init(struct element_text *text, int id, u8 page, u8 prj,
                       const struct element_css1 *css,
                       const struct element_event_action *action);


void text_element_set_event_handler(struct element_text *text, void *priv,
                                    const struct element_event_handler *handler);






#endif

