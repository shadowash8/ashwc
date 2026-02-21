/* config.h */
static const char* termcmd[] = { "foot", NULL };

static struct bind binds[] = {
    /* Modifier             Keysym              Function     Argument */
    { WLR_MODIFIER_ALT,     XKB_KEY_Return,     spawn,       {.v = termcmd}   },
    { WLR_MODIFIER_ALT,     XKB_KEY_Escape,     quit,        {.v = NULL}      },
};
