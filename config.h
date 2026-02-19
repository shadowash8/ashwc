/* config.h */
static const char* termcmd[] = { "foot", NULL };

static struct bind binds[] = {
    /* Keysym             Function     Argument */
    { XKB_KEY_Return,     spawn,       {.v = termcmd}   },
    { XKB_KEY_Escape,     quit,        {.v = NULL}      },
};
