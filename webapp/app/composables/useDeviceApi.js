import { decodeFrame } from '~/composables/useFrame.js';
import { delay, translate } from '~/composables/useUtils.js';

// WiFi connection status codes returned by the device
const connectionStatus = Object.freeze({
    WIFI_TRY: 2,
    WIFI_OK: 1,
    WIFI_KO: 0
});

// Fetch either the real ESP endpoint or the local Nitro mock endpoint
export const fetchEsp = (path, options = {}) => {
    const runtimeConfig = useRuntimeConfig();
    const baseUrl = runtimeConfig.public.espBaseUrl.trim();
    const normalizedBaseUrl = baseUrl ? baseUrl.replace(/\/$/, '') : '';
    const normalizedPath = path.startsWith('/') ? path : `/${path}`;
    const resolvedUrl = normalizedBaseUrl ? `${normalizedBaseUrl}${normalizedPath}` : normalizedPath;
    return $fetch(resolvedUrl, options);
};

// Fetch the list of visible WiFi networks
export const getNetworks = async () => {
    const response = await fetchEsp('/api/networks');
    return response.data || {};
};

// Fetch the device state: the settings, plus what the device knows about itself
export const getState = async () => {
    const response = await fetchEsp('/api/state');
    return response.data || {};
};

// Send some of the settings ({ on, brightness, scene, maxMilliamps, countdown, message }) and get the state after the change
export const saveSettings = async settings => {
    // Call the device API to save the settings
    const response = await fetchEsp('/api/settings', {
        method: 'POST',
        body: settings
    });

    // Return the state after the change
    return response.data || {};
};

// Fetch the frame on the matrix as raw bytes (72x12 pixels, 3 bytes each)
export const getFrame = async () => {
    const buffer = await fetchEsp('/api/frame', { responseType: 'arrayBuffer', retry: 0 });
    return decodeFrame(buffer);
};

// Poll the device until the WiFi connection attempt completes
const checkWiFiConnectionPolling = async () => {
    const maxAttempts = 45;

    // Poll the device every second to check the WiFi connection status
    for (let attempt = 1; attempt <= maxAttempts; attempt += 1) {
        try {
            const response = await fetchEsp('/api/check-connection', { retry: 0 });
            const status = Number(response.data?.connectionStatus);

            // If the device is still trying to connect, continue polling
            switch (status) {
                // Trying to connect
                case connectionStatus.WIFI_TRY:
                    break;

                // Connection successful
                case connectionStatus.WIFI_OK:
                    return {
                        ssid: response.data?.ssid || '',
                        ip: response.data?.ip || '',
                        hostname: response.data?.hostname || ''
                    };

                // Connection failed
                case connectionStatus.WIFI_KO:
                    throw new Error(translate('api.wifiWrongCredentials'));

                // Unknown status code
                default:
                    throw new Error(translate('api.wifiUnknownStatus'));
            }
        } catch (error) {
            if (error instanceof Error && error.message !== 'fetch failed') {
                throw error;
            }
        }

        // Wait 1 second before the next polling attempt
        await delay(1000);
    }

    // If we exhaust all attempts without a successful connection, throw a timeout error
    throw new Error(translate('api.wifiTimeout'));
};

// Send WiFi credentials and wait for the final connection result
export const connectToWiFi = async (ssid, password) => {
    try {
        // Call the device API to start the WiFi connection process
        const response = await fetchEsp('/api/connect', {
            method: 'POST',
            body: {
                ssid,
                password
            }
        });

        // If the device is trying to connect, start polling for the result
        if (Number(response.data?.connectionStatus) === connectionStatus.WIFI_TRY) {
            return await checkWiFiConnectionPolling();
        }

        // If the device returned an immediate failure, throw an error
        throw new Error(translate('api.wifiFailed'));
    } catch (error) {
        if (error instanceof Error) throw error;
        throw new Error(translate('api.wifiConnectError'));
    }
};