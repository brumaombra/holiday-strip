<script setup>
import { onMounted } from 'vue';
import { setBusy } from '@brumaombra/ui-vintage/busy-indicator';
import { useTheme } from '@brumaombra/ui-vintage/theme-selector';
import { loadDeviceState } from '~/composables/useDeviceState.js';
import { useGlobalStore } from '~/composables/stores/useGlobalStore.js';

const globalStore = useGlobalStore();
const { t } = useI18n();
const { startTheme } = useTheme();

// Load the device state once for the app shell (the Wi-Fi pages scan the networks themselves, so a failed scan can't block the state)
const loadInitialData = async () => {
    // Nothing to load if the state is already there
    if (globalStore.value.deviceLoaded) return;

    try {
        setBusy(true, { label: t('common.syncingDevice') });
        await loadDeviceState();
    } catch (error) {
        handleBackendErrors({ error, defaultMessage: t('app.loadError'), showDialog: true });
    } finally {
        setBusy(false);
    }
};

// Apply the saved theme (shared with the theme selector) and preload device data once the app mounts
onMounted(async () => {
    startTheme();
    await loadInitialData();
});
</script>

<template>
    <div>
        <!-- Route content -->
        <NuxtLayout>
            <NuxtPage />
        </NuxtLayout>
    </div>
</template>
