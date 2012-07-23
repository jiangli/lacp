
#ifndef _LACP_CHOOSE_H__
#define _LACP_CHOOSE_H__

#define CHOOSE(a) a
typedef enum STATES LACP_STATE_T;
#undef CHOOSE

char * GET_STATE_NAME (uint32_t state)
{
#define CHOOSE(a) #a
    static char    *state_names[] = STATES;
#undef CHOOSE

    if (LACP_BEGIN == state) return "Begin";
    return state_names[state];
}

#endif /* _LACP_CHOOSE_H__ */

