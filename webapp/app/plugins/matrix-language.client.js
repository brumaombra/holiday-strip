import { watch } from 'vue';
import { applyDeviceSettings } from '~/composables/useDeviceState.js';
import { useGlobalStore } from '~/composables/stores/useGlobalStore.js';

// Send the language picked in the top bar to the strip, so the texts on the matrix follow it (the Christmas countdown, and the greetings still at their default)
export default defineNuxtPlugin(nuxtApp => {
    const globalStore = useGlobalStore();

    // Send it when it changes, once the state of the strip is known (and only when the strip has another one)
    watch(() => nuxtApp.$i18n.locale.value, language => {
        if (!globalStore.value.deviceLoaded || globalStore.value.device.language === language) return;
        applyDeviceSettings({ language });
    });
});