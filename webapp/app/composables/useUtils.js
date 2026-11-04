import Alert02Icon from '@hugeicons/core-free-icons/Alert02Icon';
import Cancel01Icon from '@hugeicons/core-free-icons/Cancel01Icon';
import { showMessageDialog } from '@brumaombra/ui-vintage/message-dialog';

// Translate a key through the active i18n instance when available
export const translate = (key, params = {}) => {
    const nuxtApp = useNuxtApp();

    // If the function is available, return the translated string
    if (typeof nuxtApp?.$i18n?.t === 'function') {
        return nuxtApp.$i18n.t(key, params);
    }

    // Otherwise, return the key as a fallback
    return key;
};

// Pause execution for a short delay
export const delay = timeout => {
    return new Promise(resolve => setTimeout(resolve, timeout));
};

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

// Split a number of seconds in days, hours and minutes
export const splitUptime = seconds => {
    const totalMinutes = Math.floor(Math.max(0, Number(seconds) || 0) / 60);
    return {
        days: Math.floor(totalMinutes / 1440),
        hours: Math.floor((totalMinutes % 1440) / 60),
        minutes: totalMinutes % 60
    };
};

// Handle backend errors with a shared message strategy
export const handleBackendErrors = ({ error, defaultMessage = '', showDialog = false }) => {
    const backendErrorPayload = error?.response?._data || error?.data || error?.response?.data || null;
    const backendErrorId = backendErrorPayload?.errorId || '';
    const backendErrorMessage = backendErrorPayload?.errorMessage || '';
    const translationKey = backendErrorId ? `api.backendErrors.${backendErrorId}` : '';
    const backendErrorTranslated = translationKey ? translate(translationKey) : '';
    const runtimeErrorMessage = error instanceof Error ? error.message : '';
    const resolvedMessage = backendErrorTranslated || backendErrorMessage || runtimeErrorMessage || defaultMessage || translate('common.genericError');
    const resolvedLogMessage = backendErrorMessage || runtimeErrorMessage || defaultMessage || translate('common.genericError');

    // Show a user-friendly error dialog if requested
    if (showDialog) {
        showMessageDialog({
            type: 'error',
            icon: Alert02Icon,
            title: translate('dialogs.errorTitle'),
            message: resolvedMessage,
            closeText: translate('common.close'),
            closeButtonIcon: Cancel01Icon
        });
    }

    // Log the error (this is the last stop: the callers only show it, so throwing it again left an unhandled error after the dialog)
    console.error(resolvedLogMessage, error);
};
