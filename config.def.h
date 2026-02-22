/* config.h */

static const struct xkb_rule_names xkb_rules = {
	/* can specify fields: rules, model, layout, variant, options */
	/* example:
	.options = "ctrl:nocaps",
	*/
	.options = NULL,
};

static const char* termcmd[] = { "foot", NULL };

static struct bind binds[] = {
    /* Modifier             Keysym              Function     Argument */
    { WLR_MODIFIER_ALT,     XKB_KEY_Return,     spawn,       {.v = termcmd}   },
    { WLR_MODIFIER_ALT,     XKB_KEY_Escape,     quit,        {.v = NULL}      },
};
