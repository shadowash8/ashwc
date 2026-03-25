/* config.def.h */
#define WORKSPACE_COUNT 9
static const struct xkb_rule_names xkb_rules = {
	/* can specify fields: rules, model, layout, variant, options */
	/* example:
	.options = "ctrl:nocaps",
	*/
	.options = NULL,
};

#define MOD  WLR_MODIFIER_ALT
#define SHIFT WLR_MODIFIER_SHIFT
#define CTRL WLR_MODIFIER_CTRL

static const char* termcmd[] = { "foot", NULL };

static struct bind binds[] = {
    /* Modifier         Keysym              Function           Argument */
    { MOD,              XKB_KEY_Return,     spawn,             {.v = termcmd} },
    { MOD,              XKB_KEY_q,          kill_client,       {.v = NULL}    },
    { MOD,              XKB_KEY_Escape,     quit,              {.v = NULL}    },
    { MOD,              XKB_KEY_Tab,        focus_next_window, {.v = NULL}    },

    /* Super+1..9 — switch to tag */
    { MOD,              XKB_KEY_1,          view_tag,          {.i = 0} },
    { MOD,              XKB_KEY_2,          view_tag,          {.i = 1} },
    { MOD,              XKB_KEY_3,          view_tag,          {.i = 2} },
    { MOD,              XKB_KEY_4,          view_tag,          {.i = 3} },
    { MOD,              XKB_KEY_5,          view_tag,          {.i = 4} },
    { MOD,              XKB_KEY_6,          view_tag,          {.i = 5} },
    { MOD,              XKB_KEY_7,          view_tag,          {.i = 6} },
    { MOD,              XKB_KEY_8,          view_tag,          {.i = 7} },
    { MOD,              XKB_KEY_9,          view_tag,          {.i = 8} },

    /* Alt+Shift+1..9 — send focused window to tag */
    { MOD|SHIFT,        XKB_KEY_1,          tag_toplevel,      {.i = 0} },
    { MOD|SHIFT,        XKB_KEY_2,          tag_toplevel,      {.i = 1} },
    { MOD|SHIFT,        XKB_KEY_3,          tag_toplevel,      {.i = 2} },
    { MOD|SHIFT,        XKB_KEY_4,          tag_toplevel,      {.i = 3} },
    { MOD|SHIFT,        XKB_KEY_5,          tag_toplevel,      {.i = 4} },
    { MOD|SHIFT,        XKB_KEY_6,          tag_toplevel,      {.i = 5} },
    { MOD|SHIFT,        XKB_KEY_7,          tag_toplevel,      {.i = 6} },
    { MOD|SHIFT,        XKB_KEY_8,          tag_toplevel,      {.i = 7} },
    { MOD|SHIFT,        XKB_KEY_9,          tag_toplevel,      {.i = 8} },

    /* Alt+Ctrl+1..9 — move focused window to tag and follow */
    { MOD|CTRL,         XKB_KEY_1,          move_to_tag,       {.i = 0} },
    { MOD|CTRL,         XKB_KEY_2,          move_to_tag,       {.i = 1} },
    { MOD|CTRL,         XKB_KEY_3,          move_to_tag,       {.i = 2} },
    { MOD|CTRL,         XKB_KEY_4,          move_to_tag,       {.i = 3} },
    { MOD|CTRL,         XKB_KEY_5,          move_to_tag,       {.i = 4} },
    { MOD|CTRL,         XKB_KEY_6,          move_to_tag,       {.i = 5} },
    { MOD|CTRL,         XKB_KEY_7,          move_to_tag,       {.i = 6} },
    { MOD|CTRL,         XKB_KEY_8,          move_to_tag,       {.i = 7} },
    { MOD|CTRL,         XKB_KEY_9,          move_to_tag,       {.i = 8} },

    /* Alt+Ctrl+Shift+1..9 — toggle tag in current view */
    { MOD|CTRL|SHIFT,   XKB_KEY_1,          toggle_tag_view,    {.i = 0} },
    { MOD|CTRL|SHIFT,   XKB_KEY_2,          toggle_tag_view,    {.i = 1} },
    { MOD|CTRL|SHIFT,   XKB_KEY_3,          toggle_tag_view,    {.i = 2} },
    { MOD|CTRL|SHIFT,   XKB_KEY_4,          toggle_tag_view,    {.i = 3} },
    { MOD|CTRL|SHIFT,   XKB_KEY_5,          toggle_tag_view,    {.i = 4} },
    { MOD|CTRL|SHIFT,   XKB_KEY_6,          toggle_tag_view,    {.i = 5} },
    { MOD|CTRL|SHIFT,   XKB_KEY_7,          toggle_tag_view,    {.i = 6} },
    { MOD|CTRL|SHIFT,   XKB_KEY_8,          toggle_tag_view,    {.i = 7} },
    { MOD|CTRL|SHIFT,   XKB_KEY_9,          toggle_tag_view,    {.i = 8} },
};
