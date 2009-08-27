  /* \brief XMI counter */
  typedef struct { 
      size_t opaque[2];
  } xmi_counter_t;

  /* \brief Set counter value */
  XMI_Counter_set(xmi_context_t context, xmi_counter_t *counter, size_t value);

  /* \brief Get counter value */
  XMI_Counter_get(xmi_context_t context, xmi_counter_t *counter, size_t *value); 
  /* \brief Wait for the counter to reach/exceed the specified the value
     then the specified value is substracted from the counter and the new
     value is returned. CPU is released after poll_count expires. */
  XMI_Counter_wait(xmi_context_t context, xmi_counter_t *counter, 
          size_t target_value, size_t *current_value, size_t poll_count);

  /* \brief Simple send command using counters for notifications */
  typedef struct
  {
    xmi_send_t             send;     /**< Common send parameters */
    struct
    {
      size_t               bytes;    /**< Number of bytes of data */
      void               * addr;     /**< Address of the buffer */
      xmi_counter_t      * local_done;/**< Local message completion event */
      xmi_counter_t      * remote_done;/**< Remote message completion event */
    } simple;                        /**< Simple send parameters */
  } xmi_send_simple_counter_t;

  /* \brief Typed send command using counters for notifications */
  typedef struct
  {
    xmi_send_t             send;     /**< Common send parameters */
    struct
    {
      size_t               bytes;    /**< Number of bytes of data */
      void               * addr;     /**< Starting address of the buffer */
      size_t               offset;   /**< Starting offset */
      xmi_type_t           type;     /**< Datatype */
      xmi_counter_t      * local_done;/**< Local message completion event */
      xmi_counter_t      * remote_done;/**< Remote message completion event */
    } typed;                         /**< Typed send parameters */
  } xmi_send_typed_counter_t;

  /* \brief Simple send function using counters for notifications */
  xmi_result_t XMI_Send_simple_counter (xmi_context_t       context,
                         xmi_send_simple_counter_t * parameters);

  /* \brief Typed send function using counters for notifications */
  xmi_result_t XMI_Send_typed_counter (xmi_context_t       context,
                         xmi_send_typed_counter_t * parameters);
