typedef int 		Node_id;
typedef struct node
{
    Node   *next;     /*  reserved for owner                             */
    void   *owner;    /* 'control' port                                  */
    Node   *pnext;    /*  reserved for provider->owner                   */
    Node   *provider; /* 'down'    port, transmits to, and receives from */
    Node   *user;     /* 'up'      port, receives to, and transmits from */

    Node_id node_id;

    /* functions called by service provider : */

    void  (*rx_fn       )(Node *,  /*  this node     */
                          void *   /*  pdu           */
                         );
    void  (*tx_fn       )(Node *,  /*  this node     */
                          void *   /*  pdu           */
                         );

    void  (*rx_status_fn)(Node *,  /*  this node     */
                          Bool, /*  enabled       */
                          void *   /*  optional args */
                         );
    void  (*tx_status_fn)(Node *,  /*  this node     */
                          Bool, /*  enabled       */
                          void *   /*  optional args */
                         );

    Bool rx_on;     /* owner's instructions to provider */
    Bool tx_on;     /*               ''                 */

    Bool rx_enabled; /* service provider's current state */
    Bool tx_enabled; /*               ''                 */
} Node;

