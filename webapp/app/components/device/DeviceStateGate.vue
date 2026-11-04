<script setup>
import Alert02Icon from '@hugeicons/core-free-icons/Alert02Icon';
import RefreshIcon from '@hugeicons/core-free-icons/RefreshIcon';
import { HugeiconsIcon } from '@hugeicons/vue';
import { setBusy } from '@brumaombra/ui-vintage/busy-indicator';
import { Button } from '@brumaombra/ui-vintage/button';
import { EmptyStateCard } from '@brumaombra/ui-vintage/empty-state-card';
import { LoadingStateCard } from '@brumaombra/ui-vintage/loading-state-card';
import { loadDeviceState } from '~/composables/useDeviceState.js';
import { useGlobalStore } from '~/composables/stores/useGlobalStore.js';

const { t } = useI18n();
const globalStore = useGlobalStore();

// Load the device state again after a failed load
const handleRetryLoadPress = async () => {
    try {
        setBusy(true, { label: t('common.syncingDevice') });
        await loadDeviceState();
    } catch (error) {
        handleBackendErrors({ error, defaultMessage: t('app.loadError'), showDialog: true });
    } finally {
        setBusy(false);
    }
};
</script>

<template>
    <div>
        <!-- The state couldn't be read: offer to try again (a form here would show the defaults, and save them over the real settings) -->
        <EmptyStateCard v-if="globalStore.deviceLoadFailed && !globalStore.deviceLoaded" :icon="Alert02Icon" :title="t('common.loadErrorTitle')" :description="t('common.loadErrorDescription')">
            <template #action>
                <Button variant="primary" @click="handleRetryLoadPress">
                    <HugeiconsIcon :icon="RefreshIcon" class="size-4" />
                    {{ t('common.loadRetryAction') }}
                </Button>
            </template>
        </EmptyStateCard>

        <!-- Still reading the state from the device -->
        <LoadingStateCard v-else-if="!globalStore.deviceLoaded" :title="t('common.syncingDevice')" />

        <!-- The page -->
        <slot v-else />
    </div>
</template>
