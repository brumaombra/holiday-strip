// Wrap a function so it runs once the calls stop for a while (cancel drops the pending call, flush runs it right away)
export const createDebounced = (callback, waitMs) => {
    let timerId = null;
    let pendingArguments = null;

    // Run the pending call
    const flush = () => {
        clearTimeout(timerId);
        timerId = null;
        if (!pendingArguments) return;
        const callArguments = pendingArguments;
        pendingArguments = null;
        return callback(...callArguments);
    };

    // Drop the pending call
    const cancel = () => {
        clearTimeout(timerId);
        timerId = null;
        pendingArguments = null;
    };

    // Remember the latest call and restart the wait
    const debounced = (...callArguments) => {
        pendingArguments = callArguments;
        clearTimeout(timerId);
        timerId = setTimeout(flush, waitMs);
    };

    debounced.flush = flush;
    debounced.cancel = cancel;
    return debounced;
};