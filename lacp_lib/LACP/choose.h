
#ifndef _LAC_CHOOSE_H__
#define _LAC_CHOOSE_H__

/* State machines states & debug tools. Sorry, if these are no readable enogth :( */

#define CHOOSE(a) a
typedef enum STATES THE_STATE_T;
#undef CHOOSE

char * GET_STATE_NAME (int state)
{
#define CHOOSE(a) #a
    static char    *state_names[] = STATES;
#undef CHOOSE

    if (BEGIN == state) return "Begin";
    return state_names[state];
}

#endif /* _LAC_CHOOSE_H__ */

