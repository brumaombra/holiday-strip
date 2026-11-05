<script setup>
import { computed, onBeforeUnmount, ref } from 'vue';
import FlashIcon from '@hugeicons/core-free-icons/FlashIcon';
import GlobeIcon from '@hugeicons/core-free-icons/GlobeIcon';
import { ListGroup, ListRow } from '@brumaombra/ui-vintage/list';
import { Slider } from '@brumaombra/ui-vintage/slider';
import { createSettingsSaver } from '~/composables/useDeviceState.js';
import { useGlobalStore } from '~/composables/stores/useGlobalStore.js';
import DeviceStateGate from '~/components/device/DeviceStateGate.vue';
import { LanguageFlag } from '@brumaombra/ui-vintage/language-flag';
import { OptionPickerDialog } from '@brumaombra/ui-vintage/option-picker-dialog';

const POWER_LIMIT_STEP = 100; // Milliamps
const { locale, locales, setLocale, t } = useI18n();
const globalStore = useGlobalStore();
const saveMaxMilliamps = createSettingsSaver();
const isLanguagePickerOpen = ref(false);

// Reactive device state
const device = computed(() => globalStore.value.device);

// Languages of the app, to pick from
const languageOptions = computed(() => locales.value.map(language => ({ value: language.code, label: language.name })));

// Name of the language in use, shown in its row
const selectedLanguageName = computed(() => languageOptions.value.find(option => option.value === locale.value)?.label || '');

// Change the power limit (the device gets one request when the slider rests)
const handleMaxMilliampsChange = ([maxMilliamps]) => {
    saveMaxMilliamps({ maxMilliamps });
};

// Apply a selected language
const handleSelectLanguage = async language => {
    await setLocale(language);
};

// Send a power limit still waiting when leaving the page
onBeforeUnmount(() => saveMaxMilliamps.flush());

// Page metadata
definePageMeta({
    layout: 'dashboard'
});
</script>

<template>
    <GizmoPage :title="t('pages.settings.title')">
        <DeviceStateGate>
            <div class="space-y-6">
                <!-- Power supply -->
                <ListGroup :title="t('pages.settings.powerTitle')" :hint="t('pages.settings.maxMilliamps.note')">
                    <ListRow :label="t('pages.settings.maxMilliamps.label')" :icon="FlashIcon" :value="`${device.maxMilliamps}mA`">
                        <Slider :model-value="[device.maxMilliamps]" :min="device.minMaxMilliamps" :max="device.maxMaxMilliamps" :step="POWER_LIMIT_STEP" :thumb-label="t('pages.settings.maxMilliamps.label')" @update:model-value="handleMaxMilliampsChange" />
                    </ListRow>
                </ListGroup>

                <!-- Language -->
                <ListGroup :title="t('pages.settings.languageTitle')">
                    <ListRow :label="t('pages.settings.language.label')" :icon="GlobeIcon" :value="selectedLanguageName" action @click="isLanguagePickerOpen = true" />
                </ListGroup>
            </div>
        </DeviceStateGate>

        <!-- Device info, restart and forget network (from the gizmo-common layer, it reads the device by itself) -->
        <GizmoDeviceCard />

        <!-- Language picker -->
        <OptionPickerDialog v-model:open="isLanguagePickerOpen" :title="t('pages.settings.language.label')" :description="t('pages.settings.language.note')" :options="languageOptions" :model-value="locale" @update:model-value="handleSelectLanguage">
            <template #icon="{ option }">
                <LanguageFlag :code="option.value" size="lg" />
            </template>
        </OptionPickerDialog>
    </GizmoPage>
</template>
