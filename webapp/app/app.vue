<script setup>
import { onMounted } from 'vue';
import { useTheme } from '@brumaombra/ui-vintage/theme-selector';
import { syncDeviceState } from '~/composables/useDeviceState.js';
import { useGlobalStore } from '~/composables/stores/useGlobalStore.js';

const globalStore = useGlobalStore();
const { startTheme } = useTheme();

// Apply the saved theme (shared with the theme selector) and load the device state once for the app shell (the Wi-Fi pages scan the networks themselves, so a failed scan can't block the state)
onMounted(async () => {
    startTheme();
    if (!globalStore.value.deviceLoaded) await syncDeviceState();
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
