#ifndef EVENT_HANDLER_HH
#define EVENT_HANDLER_HH



class EventHandler {
public:
    virtual ~EventHandler() = default;
    virtual auto handler() -> void = 0;
};


#endif // EVENT_HANDLER_HH
